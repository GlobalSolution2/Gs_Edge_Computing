# SQL da versão anterior

Estes arquivos foram preservados como histórico. Não são o provisionamento do Wokwi final.

| Arquivo | Uso na versão anterior |
| --- | --- |
| `supabase_room_telemetry.sql` | Criava a linha atual da S01 e permitia atualização por PATCH |
| `supabase_room_telemetry_history.sql` | Arquivava a leitura anterior em cada UPDATE |
| `supabase_remove_light_unit.sql` | Removia a coluna de unidade da instalação antiga |

O firmware final insere a leitura atual por **POST em `room_telemetry_history`**, incluindo `room_id`. Esses scripts, isoladamente, não permitem esse fluxo. O último contém remoção de coluna e depende do estado da instalação antiga.

O contrato atual está em [documentacao/README.md](../../../documentacao/README.md).
