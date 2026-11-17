-- Executar uma vez no SQL Editor de edge_sql, depois de supabase_room_telemetry.sql.
-- Mantem public.room_telemetry como ultima leitura S01. A cada UPDATE,
-- arquiva a leitura ANTERIOR com seu updated_at original.
-- Leituras anteriores a esta migracao nao podem ser reconstruidas.

begin;

create table public.room_telemetry_history (
  measurement_id bigint generated always as identity primary key,
  room_id text not null check (room_id = 'S01'),
  occupancy_by_table jsonb not null,
  temperature_c numeric(4,1),
  light_value numeric(7,0),
  noise_db_simulated numeric(4,1),
  device_status text not null,
  measured_at timestamptz not null,
  archived_at timestamptz not null default now()
);

create index room_telemetry_history_room_time_idx
  on public.room_telemetry_history (room_id, measured_at desc);

alter table public.room_telemetry_history enable row level security;
revoke all on table public.room_telemetry_history from public, anon, authenticated;
revoke all on sequence public.room_telemetry_history_measurement_id_seq
  from public, anon, authenticated;

create function public.citizentech_archive_telemetry()
returns trigger
language plpgsql
security definer
set search_path = ''
as $$
begin
  if old.device_status in ('ok', 'sensor_error') then
    insert into public.room_telemetry_history (
      room_id, occupancy_by_table, temperature_c, light_value,
      noise_db_simulated, device_status, measured_at
    ) values (
      old.room_id, old.occupancy_by_table, old.temperature_c, old.light_value,
      old.noise_db_simulated, old.device_status, old.updated_at
    );
  end if;
  return new;
end;
$$;

revoke all on function public.citizentech_archive_telemetry()
  from public, anon, authenticated;

create trigger room_telemetry_archive_previous
after update on public.room_telemetry
for each row execute function public.citizentech_archive_telemetry();

commit;
