# Documentação técnica - Edge S01

**Vídeo:** [YouTube](https://youtu.be/MziTbxpFiXo)  
**Wokwi final:** [Simulação](https://wokwi.com/projects/480352166150085633)

Para executar e replicar a simulação, siga o [README principal](../README.md). Esta página registra o contrato da integração e os limites da reprodução.

## Versão local

Os arquivos em `../wokwi/` correspondem ao projeto final consultado em 17/11/2026. O firmware é idêntico após normalizar as quebras de linha; o diagrama preserva os mesmos componentes, posições e ligações, com formatação JSON local. As bibliotecas também foram conferidas.

A versão final usa **POST em `room_telemetry_history`**, inclui `room_id` no corpo e recebe HTTP 201. A anterior usava PATCH em `room_telemetry?room_id=eq.S01` e recebia HTTP 200.

## Contrato do envio

Endpoint relativo à URL do projeto Supabase:

```text
POST /rest/v1/room_telemetry_history
```

Cabeçalhos: `apikey` com a chave publicável, `Content-Type: application/json` e `Prefer: return=representation`. Chaves novas `sb_publishable_` vão somente em `apikey`; a condição para Bearer legado está no firmware.

Exemplo de corpo, apenas ilustrativo:

```json
{
  "room_id": "S01",
  "occupancy_by_table": [1, 0, 0],
  "temperature_c": 23.0,
  "light_value": 500,
  "noise_db_simulated": 42.2,
  "device_status": "ok"
}
```

`light_value` é sempre lux estimado. O ruído vem da conversão `30 + ADC * 50 / 4095`, sem calibração acústica. O firmware não envia `measurement_id` nem `measured_at`: esses campos precisam ser definidos no backend.

Com leitura inválida, os três valores ambientais são `null`; `device_status` é `sensor_error`. `device_status = ok` significa leitura válida, inclusive quando a condição ambiental é crítica. O estado de conforto é calculado separadamente.

## Replicar a camada de nuvem

Para copiar apenas o circuito, basta usar a demonstração configurada ou deixar URL e chave vazias. Para criar um ambiente independente:

1. Crie um projeto Supabase de demonstração, sem dados pessoais.
2. Prepare `public.room_telemetry_history` para receber o corpo acima, com identificação e horário gerados no servidor.
3. Configure as permissões de inserção e retorno da linha para o papel usado pela chave publicável, restritas aos dados simulados da S01.
4. Permita a consulta do registro mais recente da S01 no histórico. O frontend atual consulta `GET /rest/v1/room_telemetry_history?room_id=eq.S01&select=*&order=measured_at.desc,measurement_id.desc&limit=1` e utiliza `measured_at` como horário da leitura. Essa consulta está implementada em [telemetria.js](https://github.com/GlobalSolution2/GS_Web_Front_Open_Source_Consulting/blob/main/src/services/telemetria.js).
5. Altere a URL e a chave publicável no firmware e na configuração do seu frontend. Confirme HTTP 201 e acompanhe a mesma alteração até a interface.

**Escopo da reprodução:** a simulação pode ser copiada e executada com o ambiente demonstrativo já configurado. Esta pasta não contém um export confirmado do esquema final, dos triggers e das políticas do Supabase publicado; os passos acima descrevem o contrato para uma futura instalação em conta própria, sem oferecer provisionamento automático.

## SQL legado

Os três SQL originais foram preservados em [apoio/sql/legado](../apoio/sql/legado/). Documentam a arquitetura anterior: atualizar a linha atual por PATCH e arquivar a leitura anterior por trigger. Não representam a inserção direta no histórico. Não execute essa sequência para recriar a versão publicada.

## Conferência

- [Registro de verificação](VERIFICACAO.md)
- [Texto para Teams ou Forms](../entrega/LINKS_E_EQUIPE.txt)
