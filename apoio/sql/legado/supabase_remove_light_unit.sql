-- Aplicar uma vez no projeto edge_sql apos a atualizacao do sketch no Wokwi.
-- Remove somente a coluna de unidade redundante; light_value e todas as
-- medicoes atuais/historicas permanecem. A unidade e lux estimado no README.

begin;

create or replace function public.citizentech_archive_telemetry()
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

alter table public.room_telemetry_history drop column light_unit;
alter table public.room_telemetry drop column light_unit;

commit;
