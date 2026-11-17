-- CitizenTech GS 2026.2 — ultima leitura simulada da sala S01.
-- Executar uma vez no SQL Editor do projeto edge_sql.
-- O Wokwi usa chave publicavel: anon pode LER a linha S01 e ATUALIZAR
-- somente as colunas de telemetria dessa linha. Qualquer pessoa com a
-- chave publicavel podera simular atualizacoes; nao usar dados reais.

create table public.room_telemetry (
  room_id text primary key check (room_id = 'S01'),
  occupancy_by_table jsonb not null default '[0,0,0]'::jsonb,
  temperature_c numeric(4,1),
  light_value numeric(7,0),
  noise_db_simulated numeric(4,1),
  device_status text not null default 'offline'
    check (device_status in ('ok', 'sensor_error', 'offline')),
  updated_at timestamptz not null default now(),
  constraint occupancy_s01_valid check (
    jsonb_typeof(occupancy_by_table) = 'array'
    and jsonb_array_length(occupancy_by_table) = 3
    and jsonb_typeof(occupancy_by_table->0) = 'number'
    and jsonb_typeof(occupancy_by_table->1) = 'number'
    and jsonb_typeof(occupancy_by_table->2) = 'number'
    and (occupancy_by_table->>0)::integer between 0 and 3
    and (occupancy_by_table->>1)::integer between 0 and 3
    and (occupancy_by_table->>2)::integer between 0 and 3
  ),
  constraint temperature_s01_valid check (
    temperature_c is null or temperature_c between -40 and 80
  ),
  constraint light_s01_valid check (
    light_value is null or light_value between 0 and 100000
  ),
  constraint noise_s01_valid check (
    noise_db_simulated is null or noise_db_simulated between 30 and 80
  ),
  constraint valid_s01_has_values check (
    device_status <> 'ok' or
    (temperature_c is not null and light_value is not null and noise_db_simulated is not null)
  )
);

create function public.citizentech_set_telemetry_updated_at()
returns trigger
language plpgsql
set search_path = pg_catalog
as $$
begin
  new.updated_at := now();
  return new;
end;
$$;

create trigger room_telemetry_updated_at
before update on public.room_telemetry
for each row execute function public.citizentech_set_telemetry_updated_at();

insert into public.room_telemetry (room_id) values ('S01');

alter table public.room_telemetry enable row level security;

-- Remover os privilegios padrao amplos antes de liberar o minimo da demo.
revoke all on table public.room_telemetry from public, anon, authenticated;
grant usage on schema public to anon;
grant select on table public.room_telemetry to anon;
grant update (
  occupancy_by_table, temperature_c, light_value,
  noise_db_simulated, device_status
) on table public.room_telemetry to anon;

create policy citizentech_s01_read
on public.room_telemetry for select to anon
using (room_id = 'S01');

create policy citizentech_s01_update
on public.room_telemetry for update to anon
using (room_id = 'S01')
with check (room_id = 'S01');
