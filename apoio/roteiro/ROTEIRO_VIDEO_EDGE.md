> Registro de apoio: roteiro usado na preparação do vídeo. Apresentação publicada: https://youtu.be/MziTbxpFiXo. As orientações abaixo preservam o planejamento anterior à gravação.

# Roteiro do vídeo de Edge — StudyComfort

**Duração estimada:** 4 a 5 minutos, com limite de 7 minutos.

**Formato:** gravação de tela narrada pela equipe. Os blocos “Falar” são o texto da apresentação; os blocos “Mostrar” são instruções para quem grava.

**Wokwi final:** https://wokwi.com/projects/480352166150085633

## Preparação — fora do vídeo

- Abrir Wokwi com circuito e Serial Monitor visíveis, Supabase na tabela `public.room_telemetry_history` e dashboard conectado à S01. Ordenar o histórico por `measurement_id` decrescente para mostrar os registros mais recentes.
- No Wokwi, começar com 23 °C, aproximadamente 500 lux, ocupação zerada e potenciômetro próximo de 25% (ruído simulado abaixo de 45 dB).
- Teste confirmado no Wokwi publicado em 14/11/2026: ajustar o LDR para 100.000 lux provoca `sensor_error`; restaurar aproximadamente 500 lux recupera a leitura e o estado adequado, com os outros indicadores adequados.
- Deixar localizados no código: identificação da equipe, limites, `recomputeStatus()`, média móvel em `readSensors()`, comentários e `loop()` com `millis()`.
- Aguardar o valor desejado aparecer e estabilizar no Serial após cada alteração ambiental. Não contar com uma espera fixa de 12–15 segundos: no teste publicado, as requisições HTTPS síncronas e a velocidade do simulador prolongaram a estabilização por dezenas de segundos. As esperas podem ser encurtadas na edição, preservando a alteração e o resultado.
- Conferir se o projeto público abre e executa sem ajustes e se todos os integrantes aparecem no código e na entrega.
- A página 36 do edital menciona respostas locais visíveis e audíveis. O projeto publicado, conferido em 14/11/2026, contém LEDs, sem buzzer ou outro componente sonoro. A resposta audível permanece pendente; a narração não substitui o som do dispositivo.

**Integração conferida:** o Wokwi publicado faz `POST` em `room_telemetry_history` e recebe HTTP **201**. Na gravação, acompanhar a mesma alteração até aparecer no dashboard antes de narrar o resultado.

## 1. A ideia — 0:00 a 0:30

### Mostrar

Dashboard do StudyComfort, destacando a sala S01 e seus indicadores.

### Falar

> “Somos a equipe do StudyComfort. Ajudamos estudantes a escolher uma sala antes de se deslocar, considerando ocupação, temperatura, iluminação e ruído.
>
> O dispositivo representa uma sala de estudos da FIAP. A proposta se conecta ao ODS 4, pelo apoio ao ambiente de aprendizagem, e ao ODS 9, pela aplicação de infraestrutura tecnológica.
>
> A solução integra ESP32 no Wokwi, Supabase e dashboard.”

## 2. O circuito — 0:30 a 1:10

### Mostrar

1. Visão geral do circuito organizado.
2. ESP32, DHT22, LDR, potenciômetro e botões, acompanhando a fala com o cursor.
3. LEDs e resistores.
4. Ligações reais de alimentação, terra comum e pinos de sinal. Aproximar a imagem para tornar os rótulos legíveis.

### Falar

> “O ESP32 lê as entradas e decide o estado da sala. O DHT22 fornece a temperatura, o LDR representa a iluminação e o potenciômetro simula o ruído.
>
> Os seis botões registram entradas e saídas em três mesas de três lugares. Os LEDs verde, amarelo e vermelho indicam adequado, atenção e crítico. Os resistores limitam a corrente dos LEDs.
>
> Aqui mostramos a alimentação, o terra comum e os pinos de sinal. O diagrama separa as entradas e os alertas para facilitar a leitura.
>
> Todas as leituras são simuladas: o ruído não vem de microfone, e os valores em lux são estimados.”

## 3. A lógica — 1:10 a 2:10

### Mostrar

1. Identificação da equipe e comentários no início do código.
2. Constantes `TEMP_LIMITS`, `LIGHT_LIMITS`, `NOISE_LIMITS` e `OCCUPANCY_LIMITS`.
3. `classifySeverity()` e `recomputeStatus()`.
4. Média móvel e validação em `readSensors()`.
5. Temporização com `millis()` no `loop()`.

### Falar

> “Os limites ficam em constantes. No ruído simulado, até 45 decibéis é adequado; acima de 45 até 65 é atenção; acima de 65 é crítico.
>
> Escolhemos essas faixas para representar níveis crescentes de interferência no estudo. Temperatura e luz alertam tanto por falta quanto por excesso; a ocupação alerta pela redução de lugares disponíveis. São critérios do protótipo, sujeitos a validação em ambiente real.
>
> O estado geral acompanha a pior condição. Uma média móvel de cinco leituras suaviza oscilações, enquanto leituras inválidas geram falha e vermelho piscando.
>
> As funções separam as responsabilidades, os comentários explicam as conversões e millis controla os intervalos das tarefas. O Serial Monitor mostra as leituras e as decisões.”

**Nota para a equipe, fora da fala:** essa justificativa explica a intenção dos limites, sem alegar respaldo normativo. Se houver uma referência técnica efetivamente usada para escolher os números, acrescentar sua identificação. Não afirmar que todo o programa é assíncrono: o envio HTTPS do Wokwi publicado é síncrono.

## 4. A simulação comentada — 2:10 a 4:30

### 4.1. Estado adequado

**Mostrar:** circuito e Serial Monitor, com os valores iniciais estabilizados e LED verde. Manter temperatura, luz e ocupação constantes durante os testes de ruído.

**Falar:**

> “Com as condições dentro dos limites adotados, a sala está adequada e o LED verde acende.”

### 4.2. Estado de atenção e suavização

**Mostrar:** aumentar o potenciômetro para 50%; acompanhar a leitura até aproximadamente 55 dB e mostrar o LED amarelo.

**Falar:**

> “Aumentando o ruído, o sistema passa para atenção. A mudança gradual que aparece no Serial é resultado da média móvel.”

### 4.3. Estado crítico

**Mostrar:** aumentar o potenciômetro para 100%; aguardar aproximadamente 80 dB e mostrar o LED vermelho aceso continuamente.

**Falar:**

> “Acima de 65 decibéis simulados, a condição é crítica e o LED vermelho acende. Essa decisão acontece no próprio ESP32.”

### 4.4. Leitura inválida e recuperação

**Mostrar:** voltar o potenciômetro para aproximadamente 25%. Ajustar o LDR para 100.000 lux; destacar `sensor_error` no Serial e o vermelho piscando. Restaurar aproximadamente 500 lux e mostrar a recuperação para adequado. O seletor do LDR usa escala logarítmica; no extremo direito, o painel mostra `100000 lux`.

**Falar:**

> “Neste extremo, a conversão do sensor produz uma leitura inválida. O sistema identifica sensor_error e faz o vermelho piscar, diferenciando falha de leitura de condição crítica.
>
> Ao restaurar uma entrada válida, o dispositivo volta a classificar as condições.”

### 4.5. Wokwi → Supabase → dashboard

**Mostrar, nesta ordem:**

1. Pressionar uma vez o botão de entrada da mesa 1.
2. Mostrar `[1,0,0]` no Serial e a confirmação de publicação HTTP **201**.
3. No Supabase, atualizar a tabela `room_telemetry_history`, ordenada por `measurement_id` decrescente; destacar o novo registro S01, `occupancy_by_table` e `measured_at`.
4. No dashboard, mostrar a S01 com uma pessoa e oito lugares disponíveis, além de temperatura, luz e ruído.

**Falar acompanhando cada tela:**

> “Registramos uma pessoa na mesa 1. O ESP32 atualiza a ocupação e confirma o envio ao Supabase.
>
> Na tabela room_telemetry_history, vemos um novo registro com a mesma ocupação e o horário da leitura.
>
> No dashboard, a S01 mostra uma pessoa e oito vagas. O estudante consulta essas informações para escolher onde estudar.
>
> A S01 recebe os dados do Wokwi; as outras oito salas usam dados demonstrativos. Assim, mostramos o ciclo de medir, decidir e alertar, integrado à interface do cidadão.”

**Orientação de gravação:** acompanhar a mesma alteração nas três telas. Aguardar a confirmação real em cada uma antes de passar à seguinte. A atualização visual no dashboard deve ser conferida na versão usada para gravar.

## Conferência do vídeo antes de publicar

| Pilar da correção | Evidência prevista |
|---|---|
| Problema e CitizenTech — 15 pontos | Etapa 1: cidadão, problema, local, ODS e contribuição da solução. |
| Circuito — 20 pontos | Etapa 2: componentes, funções, ligações visíveis e diagrama organizado. |
| Lógica de borda — 25 pontos | Etapas 3 e 4: justificativa dos limites, três estados, respostas distintas, falha, recuperação e suavização. |
| Qualidade do código — 15 pontos | Etapas 3 e 4: funções, constantes, comentários, `millis()` e Serial Monitor. |
| Vídeo — 25 pontos | Explicação narrada e demonstração de adequado, atenção, crítico e falha de leitura. |

Publicar no YouTube como **público ou não listado**, com duração máxima de **7 minutos**. Um integrante envia os links do Wokwi e do YouTube, com os nomes de toda a equipe, na tarefa indicada pelo professor.

**Referência:** edital “Desafio-FIAP-GS-CitizenTech 1ES Paulista e Lins.pdf”, páginas 36–38 do arquivo. A página de avaliação é a 38 do arquivo, embora seu rodapé esteja numerado como 41.
