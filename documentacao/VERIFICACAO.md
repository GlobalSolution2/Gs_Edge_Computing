# Verificação da pasta Edge

Data: 17/11/2026. Wokwi consultado: https://wokwi.com/projects/480352166150085633

## Arquivos sincronizados

O código e o diagrama foram lidos pelo editor do projeto final no navegador. O código anexado anteriormente pelo grupo foi comparado ao texto do editor antes de ser utilizado como fonte local.

| Arquivo | Conferência |
| --- | --- |
| `wokwi/sketch.ino` | Igual ao editor publicado após normalizar CRLF para LF; comparação FNV-1a: `1750743654` |
| `wokwi/diagram.json` | Mesmo JSON do editor após retirar diferenças de formatação; comparação FNV-1a: `2662684657` |
| `wokwi/libraries.txt` | `DHT sensor library for ESPx` e `ArduinoJson`, iguais ao editor |

O FNV-1a registra a comparação de conteúdo desta revisão; não é assinatura de autenticidade. A diferença principal do firmware antigo é a inclusão de `room_id` e a troca de PATCH da linha atual por POST no histórico. O diagrama também incorpora as posições finais do DHT22, do potenciômetro e do botão de entrada da mesa 3.

## Execução observada no Wokwi

A versão publicada iniciou sem ajustes no código e conectou a `Wokwi-GUEST`. O Serial registrou:

| Cenário | Resultado observado |
| --- | --- |
| Inicial | 23 °C, 500 lux estimados, 42,2 dB simulados, `[0,0,0]`, `estado=adequado` |
| Mesa 1 + | `[BOTAO] Mesa 1 = 1`, sala 1/9, vagas 8 e mesas vazias 2 |
| Aumento do potenciômetro | 47,1 e 52,0 dB durante a transição; `estado=atencao` |
| Potenciômetro próximo do máximo | 69,1 até 78,9 dB; `estado=critico` |
| LDR ajustado para 100.000 lux | Mensagem de leitura inválida e `estado=sensor_error` |
| Recuperação do LDR | Aproximadamente 502 lux, 42,2 dB e retorno a `estado=adequado` |
| Mesa 1 - ao finalizar | `[0,0,0]`, 0/9, 9 vagas e nova publicação HTTP 201 |
| Publicações | HTTP 201 repetido nos cenários observados, incluindo falha de leitura |

Os estados desta tabela foram conferidos pelo Serial. A lógica dos LEDs foi conferida no código; este registro não equivale a uma medição elétrica ou auditoria visual de cada quadro da piscada. Os controles foram alterados durante a execução, sem salvar mudanças no projeto publicado.

A simulação foi parada ao término da conferência, após a publicação da ocupação zerada em estado adequado.

## Conferência da organização

Foram conferidos a presença dos arquivos esperados, a sintaxe JSON, os identificadores das conexões, os links locais dos Markdown e a correspondência do firmware/diagrama/bibliotecas com o projeto publicado. A cópia do vídeo e os SQL preservados foram comparados por SHA-256 aos originais. Não há arquivos `.env` na pasta preparada. A documentação da consulta do frontend foi conferida com o código do repositório compartilhado, que busca o registro mais recente diretamente em `room_telemetry_history`.

## Vídeo e materiais

- Vídeo informado pelo grupo: https://youtu.be/MziTbxpFiXo
- MP4 local: 270,8 segundos, 1024 × 576, vídeo e áudio, 32.231.546 bytes.
- O original em Downloads foi preservado e copiado para `video/video_edge.mp4`.
- SQL originais foram preservados sem alteração em `apoio/sql/legado` e identificados como arquitetura antiga.

## Limites da conferência

Os testes diretos desta revisão cobriram o firmware e a inserção aceita pela API. Não foi feita exportação do esquema final do Supabase ou nova compilação local por Arduino CLI. O firmware, o circuito e as bibliotecas foram preservados conforme a versão final do Wokwi.
