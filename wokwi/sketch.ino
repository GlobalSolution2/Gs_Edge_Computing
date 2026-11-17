/*
  Global Solution 2026.2 - CitizenTech - sala S01 (dados simulados)
  Equipe: Daniel Roberto, Felipe Pedrol, Jecky Cossio e Felipi Godoy.

  ESP32/Wokwi: 3 mesas x 3 lugares, temperatura DHT22, lux estimado,
  ruido em dB SIMULADOS por potenciometro e alerta local por 3 LEDs.
  Este circuito nao mede uma sala real nem reserva assentos.
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <math.h>

// Preencher SOMENTE com URL e chave publicavel (anon/publishable) do projeto demo.
// Nunca colocar service_role, secret key ou dados pessoais em projeto publico.
const char *SUPABASE_URL = "https://cuhnfntibbbdgzqsvnjv.supabase.co";
const char *SUPABASE_PUBLISHABLE_KEY = "sb_publishable__FdJIrGdvzlmac6kU0zPng_ZHptkpPW";

constexpr uint8_t TABLE_COUNT = 3;
constexpr uint8_t SEATS_PER_TABLE = 3;
constexpr uint8_t CAPACITY = TABLE_COUNT * SEATS_PER_TABLE;
constexpr uint8_t BTN_IN[TABLE_COUNT] = {13, 16, 18};
constexpr uint8_t BTN_OUT[TABLE_COUNT] = {14, 17, 19};
constexpr uint8_t PIN_DHT = 27;
constexpr uint8_t PIN_LIGHT = 35; // ADC1: funciona com Wi-Fi ativo
constexpr uint8_t PIN_NOISE = 34; // ADC1: potenciometro, NAO e microfone
constexpr uint8_t LED_GREEN = 21;
constexpr uint8_t LED_YELLOW = 22;
constexpr uint8_t LED_RED = 23;

constexpr uint32_t DEBOUNCE_MS = 45;
constexpr uint32_t SENSOR_INTERVAL_MS = 2500; // DHT22 requer intervalo >= 2 s
constexpr uint32_t PRINT_INTERVAL_MS = 3000;
constexpr uint32_t PUBLISH_INTERVAL_MS = 10000;
constexpr uint32_t WIFI_RETRY_MS = 10000;
constexpr uint32_t ERROR_BLINK_MS = 350;
constexpr uint8_t FILTER_SIZE = 5;

// Proposta de limiares do grupo (08/11/2026): aplicar no Edge; validacao
// final pelo grupo ainda pendente. Faixa e severidade sao independentes.
constexpr float NOISE_DB_MIN = 30.0f;
constexpr float NOISE_DB_MAX = 80.0f;
constexpr float ADC_MAX = 4095.0f;
constexpr float LDR_VCC = 3.3f;
constexpr float LDR_R = 10000.0f;
constexpr float LDR_GAMMA = 0.7f;
constexpr float LDR_RL10 = 50.0f;

enum RangeLevel : uint8_t { RANGE_LOW, RANGE_MEDIUM, RANGE_HIGH, RANGE_UNKNOWN };
enum Severity : uint8_t { SEV_OK, SEV_ATTENTION, SEV_CRITICAL };

struct Thresholds {
  float rangeLow;
  float rangeHigh;
  float criticalBelow;
  float attentionBelow;
  float attentionAbove;
  float criticalAbove;
};

const Thresholds TEMP_LIMITS = {21.0f, 26.0f, 18.0f, 21.0f, 25.0f, 26.0f};
const Thresholds LIGHT_LIMITS = {300.0f, 750.0f, 200.0f, 300.0f, 2000.0f, 3000.0f};
const Thresholds NOISE_LIMITS = {40.0f, 55.0f, -INFINITY, -INFINITY, 45.0f, 65.0f};
const Thresholds OCCUPANCY_LIMITS = {35.0f, 70.0f, -INFINITY, -INFINITY, 50.0f, 90.0f};

struct ButtonState {
  uint8_t pin;
  uint8_t table;
  int8_t delta;
};

ButtonState buttons[TABLE_COUNT * 2];
volatile uint8_t pendingButtonPresses[TABLE_COUNT * 2] = {0};
volatile bool buttonPressed[TABLE_COUNT * 2] = {false};
volatile uint32_t buttonPressAt[TABLE_COUNT * 2] = {0};
uint8_t occupied[TABLE_COUNT] = {0, 0, 0};
DHTesp dht;

float tempHistory[FILTER_SIZE] = {0};
float lightHistory[FILTER_SIZE] = {0};
float noiseHistory[FILTER_SIZE] = {0};
uint8_t historyCount = 0;
uint8_t historyNext = 0;
float temperatureC = NAN;
float lightLuxEstimated = NAN;
float noiseDbSimulated = NAN;
int noiseIndex = 0;
bool sensorValid = false;
RangeLevel tempRange = RANGE_UNKNOWN;
RangeLevel lightRange = RANGE_UNKNOWN;
RangeLevel noiseRange = RANGE_UNKNOWN;
RangeLevel occupancyRange = RANGE_UNKNOWN;
Severity tempSeverity = SEV_OK;
Severity lightSeverity = SEV_OK;
Severity noiseSeverity = SEV_OK;
Severity occupancySeverity = SEV_OK;
Severity overall = SEV_ATTENTION;
bool telemetryDirty = true;
bool cloudConfigured = false;
uint32_t lastSensorAt = 0;
uint32_t lastPrintAt = 0;
uint32_t lastPublishAt = 0;
uint32_t lastWifiTryAt = 0;

uint8_t totalOccupied() {
  uint8_t total = 0;
  for (uint8_t i = 0; i < TABLE_COUNT; ++i) total += occupied[i];
  return total;
}

uint8_t emptyTables() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < TABLE_COUNT; ++i) {
    if (occupied[i] == 0) ++count;
  }
  return count;
}

RangeLevel classifyRange(float value, const Thresholds &limits) {
  if (value < limits.rangeLow) return RANGE_LOW;
  if (value > limits.rangeHigh) return RANGE_HIGH;
  return RANGE_MEDIUM;
}

Severity classifySeverity(float value, const Thresholds &limits) {
  if (value < limits.criticalBelow || value > limits.criticalAbove) return SEV_CRITICAL;
  if (value < limits.attentionBelow || value > limits.attentionAbove) return SEV_ATTENTION;
  return SEV_OK;
}

const char *rangeName(RangeLevel range) {
  switch (range) {
    case RANGE_LOW: return "baixo";
    case RANGE_MEDIUM: return "medio";
    case RANGE_HIGH: return "alto";
    default: return "sem_leitura";
  }
}

const char *severityName(Severity severity) {
  if (severity == SEV_CRITICAL) return "critico";
  if (severity == SEV_ATTENTION) return "atencao";
  return "adequado";
}

const char *overallName() {
  if (!sensorValid) return "sensor_error";
  if (overall == SEV_CRITICAL) return "critico";
  if (overall == SEV_ATTENTION) return "atencao";
  return "adequado";
}

void recomputeStatus() {
  float occupancyPercent = 100.0f * totalOccupied() / CAPACITY;
  occupancyRange = classifyRange(occupancyPercent, OCCUPANCY_LIMITS);
  occupancySeverity = classifySeverity(occupancyPercent, OCCUPANCY_LIMITS);
  if (!sensorValid) return;
  tempRange = classifyRange(temperatureC, TEMP_LIMITS);
  lightRange = classifyRange(lightLuxEstimated, LIGHT_LIMITS);
  noiseRange = classifyRange(noiseDbSimulated, NOISE_LIMITS);
  tempSeverity = classifySeverity(temperatureC, TEMP_LIMITS);
  lightSeverity = classifySeverity(lightLuxEstimated, LIGHT_LIMITS);
  noiseSeverity = classifySeverity(noiseDbSimulated, NOISE_LIMITS);

  overall = SEV_OK;
  const Severity severities[] = {tempSeverity, lightSeverity, noiseSeverity, occupancySeverity};
  for (Severity severity : severities) if (severity > overall) overall = severity;
}

void changeOccupancy(uint8_t table, int8_t delta) {
  int proposed = static_cast<int>(occupied[table]) + delta;
  if (proposed < 0 || proposed > SEATS_PER_TABLE) {
    Serial.printf("[BOTAO] Mesa %u: operacao recusada (limite 0..3)\n", table + 1);
    return;
  }
  occupied[table] = static_cast<uint8_t>(proposed);
  telemetryDirty = true;
  recomputeStatus();
  Serial.printf("[BOTAO] Mesa %u = %u | sala = %u/%u | mesas vazias = %u\n",
                table + 1, occupied[table], totalOccupied(), CAPACITY, emptyTables());
}

// Captura o toque mesmo quando uma requisicao HTTPS ocupa o loop principal.
void IRAM_ATTR captureButtonEdge(uint8_t index) {
  bool pressed = digitalRead(buttons[index].pin) == LOW;
  if (pressed == buttonPressed[index]) return;
  // Soltar reabilita imediatamente o proximo toque. O Wokwi desativa o
  // repique eletrico nestes seis botoes; o filtro abaixo protege toques
  // muito proximos sem descartar a soltura durante um HTTPS bloqueante.
  buttonPressed[index] = pressed;
  if (!pressed) return;
  uint32_t now = millis();
  if (buttonPressAt[index] != 0 && now - buttonPressAt[index] < DEBOUNCE_MS) return;
  buttonPressAt[index] = now;
  if (pressed && pendingButtonPresses[index] < UINT8_MAX) ++pendingButtonPresses[index];
}

void IRAM_ATTR onButton0() { captureButtonEdge(0); }
void IRAM_ATTR onButton1() { captureButtonEdge(1); }
void IRAM_ATTR onButton2() { captureButtonEdge(2); }
void IRAM_ATTR onButton3() { captureButtonEdge(3); }
void IRAM_ATTR onButton4() { captureButtonEdge(4); }
void IRAM_ATTR onButton5() { captureButtonEdge(5); }
void (*const BUTTON_ISR[TABLE_COUNT * 2])() = {
  onButton0, onButton1, onButton2, onButton3, onButton4, onButton5
};

void readButtons() {
  for (uint8_t index = 0; index < TABLE_COUNT * 2; ++index) {
    noInterrupts();
    uint8_t count = pendingButtonPresses[index];
    pendingButtonPresses[index] = 0;
    interrupts();
    while (count--) changeOccupancy(buttons[index].table, buttons[index].delta);
  }
}

float mean(const float values[], uint8_t count) {
  float sum = 0;
  for (uint8_t i = 0; i < count; ++i) sum += values[i];
  return sum / count;
}

void readSensors() {
  TempAndHumidity reading = dht.getTempAndHumidity();
  int lightRaw = analogRead(PIN_LIGHT);
  int noiseRaw = analogRead(PIN_NOISE);
  float temp = reading.temperature;

  // Conversao do LDR Wokwi para lux estimado (gamma=0,7; RL10=50 kohm).
  // ADC zero equivale a luz extrema e gera leitura impossivel; ADC maximo
  // equivale a escuridao e resulta em 0 lux.
  float voltage = lightRaw / ADC_MAX * LDR_VCC;
  float resistance = lightRaw == 4095 ? INFINITY : LDR_R * voltage / (LDR_VCC - voltage);
  float light = powf(LDR_RL10 * 1000.0f * powf(10.0f, LDR_GAMMA) / resistance,
                     1.0f / LDR_GAMMA);
  float noise = NOISE_DB_MIN + noiseRaw * (NOISE_DB_MAX - NOISE_DB_MIN) / ADC_MAX;

  if (!isfinite(temp) || temp < -40.0f || temp > 80.0f ||
      lightRaw < 0 || lightRaw > 4095 || !isfinite(light) || light < 0.0f || light > 100000.0f ||
      noiseRaw < 0 || noiseRaw > 4095 || !isfinite(noise) ||
      noise < NOISE_DB_MIN || noise > NOISE_DB_MAX) {
    sensorValid = false;
    temperatureC = NAN;
    lightLuxEstimated = NAN;
    noiseDbSimulated = NAN;
    historyCount = 0;
    historyNext = 0;
    tempRange = lightRange = noiseRange = RANGE_UNKNOWN;
    telemetryDirty = true;
    Serial.println("[ERRO] Leitura invalida; LED vermelho piscando. Nenhum valor antigo sera tratado como atual.");
    return;
  }

  // Ruido e dB SIMULADOS pelo potenciometro, sem microfone nem calibracao.
  tempHistory[historyNext] = temp;
  lightHistory[historyNext] = light;
  noiseHistory[historyNext] = noise;
  if (historyCount < FILTER_SIZE) ++historyCount;
  historyNext = (historyNext + 1) % FILTER_SIZE;

  // Classificar os mesmos numeros que serao publicados: evita divergencia
  // do LED com a web quando um valor fica muito perto de um limiar.
  temperatureC = roundf(mean(tempHistory, historyCount) * 10.0f) / 10.0f;
  lightLuxEstimated = roundf(mean(lightHistory, historyCount));
  float noiseDbUnrounded = mean(noiseHistory, historyCount);
  noiseDbSimulated = roundf(noiseDbUnrounded * 10.0f) / 10.0f;
  noiseIndex = static_cast<int>(floorf(2.0f * (noiseDbUnrounded - NOISE_DB_MIN) + 0.5f));
  noiseIndex = constrain(noiseIndex, 0, 100);
  sensorValid = true;
  telemetryDirty = true;
  recomputeStatus();
}

void updateLeds(uint32_t now) {
  if (!sensorValid) {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, (now / ERROR_BLINK_MS) % 2 == 0 ? HIGH : LOW);
    return;
  }
  digitalWrite(LED_GREEN, overall == SEV_OK ? HIGH : LOW);
  digitalWrite(LED_YELLOW, overall == SEV_ATTENTION ? HIGH : LOW);
  digitalWrite(LED_RED, overall == SEV_CRITICAL ? HIGH : LOW);
}

void printStatus() {
  Serial.printf("[S01] mesas=[%u,%u,%u] ocupacao=%u/%u vagas=%u vazias=%u\n",
                occupied[0], occupied[1], occupied[2], totalOccupied(), CAPACITY,
                CAPACITY - totalOccupied(), emptyTables());
  if (sensorValid) {
    Serial.printf("  temperatura=%.1f C faixa=%s severidade=%s | luz=%.0f lux estimado faixa=%s severidade=%s\n",
                  temperatureC, rangeName(tempRange), severityName(tempSeverity),
                  lightLuxEstimated, rangeName(lightRange), severityName(lightSeverity));
    Serial.printf("  ruido=%.1f dB simulados indice=%d/100 faixa=%s severidade=%s | ocupacao=%.1f%% faixa=%s severidade=%s\n",
                  noiseDbSimulated, noiseIndex, rangeName(noiseRange), severityName(noiseSeverity),
                  100.0f * totalOccupied() / CAPACITY, rangeName(occupancyRange),
                  severityName(occupancySeverity));
  }
  Serial.printf("  estado=%s | Wi-Fi=%s | nuvem=%s\n", overallName(),
                WiFi.status() == WL_CONNECTED ? "conectado" : "offline",
                cloudConfigured ? "configurada" : "nao configurada");
}

void maintainWifi(uint32_t now) {
  if (!cloudConfigured || WiFi.status() == WL_CONNECTED) return;
  if (lastWifiTryAt != 0 && now - lastWifiTryAt < WIFI_RETRY_MS) return;
  lastWifiTryAt = now;
  WiFi.disconnect();
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.println("[Wi-Fi] Tentando conectar a Wokwi-GUEST; alerta local segue ativo.");
}

String jsonPayload() {
  JsonDocument doc;
  doc["room_id"] = "S01";
  JsonArray tables = doc["occupancy_by_table"].to<JsonArray>();
  for (uint8_t table = 0; table < TABLE_COUNT; ++table) tables.add(occupied[table]);
  if (sensorValid) {
    doc["temperature_c"] = temperatureC;
    doc["light_value"] = lightLuxEstimated;
    doc["noise_db_simulated"] = noiseDbSimulated;
  } else {
    doc["temperature_c"] = nullptr;
    doc["light_value"] = nullptr;
    doc["noise_db_simulated"] = nullptr;
  }
  doc["device_status"] = sensorValid ? "ok" : "sensor_error";
  String body;
  serializeJson(doc, body);
  return body;
}

void publishTelemetry(uint32_t now) {
  if (!cloudConfigured || WiFi.status() != WL_CONNECTED) return;
  if (!telemetryDirty && now - lastPublishAt < PUBLISH_INTERVAL_MS) return;
  if (lastPublishAt != 0 && now - lastPublishAt < PUBLISH_INTERVAL_MS) return;
  lastPublishAt = now;

  // TLS protege o trafego, mas setInsecure nao valida o servidor: uso apenas
  // nesta demonstracao com dados sinteticos e chave publicavel. Em producao,
  // configurar uma CA confiavel com setCACert().
  // Manter o objeto TLS evita o bloqueio observado no destrutor do cliente
  // local no Wokwi. Fechar o socket antes de cada tentativa evita reutilizar
  // uma conexao que o gateway HTTPS encerrou apos a resposta anterior.
  static WiFiClientSecure client;
  client.stop();
  client.setInsecure();
  HTTPClient http;
  http.setReuse(false);
  http.setTimeout(4000);
  String url = String(SUPABASE_URL) + "/rest/v1/room_telemetry_history";
  if (!http.begin(client, url)) {
    Serial.println("[HTTP] Falha ao iniciar requisicao HTTPS.");
    return;
  }
  http.addHeader("apikey", SUPABASE_PUBLISHABLE_KEY);
  // As chaves novas sb_publishable_ vao SOMENTE no header apikey.
  // Apenas a chave anon legada (JWT) pode servir como Bearer.
  if (!String(SUPABASE_PUBLISHABLE_KEY).startsWith("sb_publishable_")) {
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_PUBLISHABLE_KEY);
  }
  http.addHeader("Content-Type", "application/json");
  // Uma resposta 204 com return=minimal tambem pode significar 0 linhas.
  // Pedir a representacao permite confirmar que S01 realmente foi alterada.
  http.addHeader("Prefer", "return=representation");
  int status = http.POST(jsonPayload());
  String response = status >= 200 ? http.getString() : "";
  if (status >= 200 && status < 300 &&
      response.indexOf("\"room_id\":\"S01\"") >= 0) {
    telemetryDirty = false;
    Serial.printf("[HTTP] S01 publicada com sucesso (%d).\n", status);
  } else {
    Serial.printf("[HTTP] Publicacao falhou (%d): %s\n", status,
                  status < 0 ? http.errorToString(status).c_str() :
                  response.length() ? response.c_str() : "nenhuma linha S01 retornada");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  for (uint8_t table = 0; table < TABLE_COUNT; ++table) {
    buttons[table * 2] = {BTN_IN[table], table, +1};
    buttons[table * 2 + 1] = {BTN_OUT[table], table, -1};
  }
  for (uint8_t index = 0; index < TABLE_COUNT * 2; ++index) {
    pinMode(buttons[index].pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttons[index].pin), BUTTON_ISR[index], CHANGE);
  }
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(PIN_LIGHT, INPUT);
  pinMode(PIN_NOISE, INPUT);
  analogReadResolution(12);
  dht.setup(PIN_DHT, DHTesp::DHT22);
  WiFi.mode(WIFI_STA);
  cloudConfigured = strlen(SUPABASE_URL) > 0 && strlen(SUPABASE_PUBLISHABLE_KEY) > 0;
  Serial.println("CitizenTech S01: simulacao iniciada. + e - por mesa, 0..3 lugares.");
  if (!cloudConfigured) Serial.println("[NUVEM] Configure URL/chave publicavel para ativar Supabase.");
  lastSensorAt = millis() - SENSOR_INTERVAL_MS;
}

void loop() {
  uint32_t now = millis();
  readButtons();
  if (now - lastSensorAt >= SENSOR_INTERVAL_MS) {
    lastSensorAt = now;
    readSensors();
  }
  updateLeds(now);
  maintainWifi(now);
  if (now - lastPrintAt >= PRINT_INTERVAL_MS) {
    lastPrintAt = now;
    printStatus();
  }
  publishTelemetry(now);
}
