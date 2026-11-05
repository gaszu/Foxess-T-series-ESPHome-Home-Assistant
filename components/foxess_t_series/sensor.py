import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart, text_sensor, binary_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor", "binary_sensor"]
ns = cg.esphome_ns.namespace("foxess_t_series")
FoxessTSeries = ns.class_("FoxessTSeries", cg.Component, uart.UARTDevice)
SENSORS = ['grid_power', 'generation_power', 'loads_power', 'grid_voltage_r', 'grid_current_r', 'grid_frequency_r', 'grid_power_r', 'grid_voltage_s', 'grid_current_s', 'grid_frequency_s', 'grid_power_s', 'grid_voltage_T', 'grid_current_T', 'grid_frequency_T', 'grid_power_T', 'pv1_voltage', 'pv1_current', 'pv2_voltage', 'pv2_current', 'pv3_voltage', 'pv3_current', 'pv4_voltage', 'pv4_current', 'boost_temperature', 'inverter_temperature', 'ambient_temperature', 'today_yield', 'generation_total', 'pv1_power', 'pv2_power', 'pv3_power', 'pv4_power', 'inverter_state']

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(FoxessTSeries),
    **{cv.Required(key): sensor.sensor_schema() for key in SENSORS},
    cv.Required("inverter_mode"): text_sensor.text_sensor_schema(),
    cv.Required("faults"): text_sensor.text_sensor_schema(entity_category="diagnostic"),
    cv.Required("connected"): binary_sensor.binary_sensor_schema(device_class="connectivity", entity_category="diagnostic"),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    for index, key in enumerate(SENSORS):
        entity = await sensor.new_sensor(config[key])
        cg.add(var.set_sensor(index, entity))
    for key in ("inverter_mode", "faults"):
        entity = await text_sensor.new_text_sensor(config[key])
        cg.add(getattr(var, "set_" + key)(entity))
    entity = await binary_sensor.new_binary_sensor(config["connected"])
    cg.add(var.set_connected(entity))
