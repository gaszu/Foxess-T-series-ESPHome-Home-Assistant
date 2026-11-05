"""Exercise production templates without requiring a running HA instance."""
from pathlib import Path
from datetime import datetime
from types import SimpleNamespace
import unittest
import yaml
from jinja2 import Environment

PACKAGE = yaml.safe_load((Path(__file__).resolve().parents[1] / 'home-assistant/foxess-production.yaml').read_text())
ENV = Environment()
# HA's float filter accepts a fallback for unknown/unavailable states.
def as_float(value, default=0):
    try:
        return float(value)
    except (ValueError, TypeError):
        return default
ENV.filters['float'] = as_float

class StatisticsTest(unittest.TestCase):
    def render(self, template, states, **extra):
        return ENV.from_string(template).render(
            states=lambda entity: states.get(entity, 'unknown'),
            is_number=lambda v: v not in ('unknown', 'unavailable') and isinstance(as_float(v, None), float),
            is_state=lambda entity, value: states.get(entity) == value,
            now=lambda: datetime(2026, 11, 6, 12), **extra).strip()

    def test_peak_preserves_record_and_resets_on_date_change(self):
        template = PACKAGE['template'][1]['sensor'][0]['state']
        source = {'sensor.t_series_generation_power': '500'}
        self.assertEqual(float(self.render(template, source, this=SimpleNamespace(state='1200', attributes={'day': '2026-11-06'}))), 1200)
        self.assertEqual(float(self.render(template, source, this=SimpleNamespace(state='1200', attributes={'day': '2026-11-05'}))), 500)
        self.assertEqual(float(self.render(template, {}, this=SimpleNamespace(state='unavailable', attributes={}))), 0)

    def test_specific_yield_and_zero_capacity(self):
        sensor = PACKAGE['template'][0]['sensor'][0]
        sources = {'sensor.t_series_today_yield': '40', 'input_number.foxess_installed_pv_kwp': '8'}
        self.assertEqual(float(self.render(sensor['state'], sources)), 5)
        self.assertEqual(self.render(sensor['availability'], sources), 'True')
        sources['input_number.foxess_installed_pv_kwp'] = '0'
        self.assertEqual(self.render(sensor['availability'], sources), 'False')
        self.assertEqual(self.render(sensor['state'], sources), 'None')

    def test_offline_is_not_counted_as_production(self):
        sensor = PACKAGE['template'][0]['binary_sensor'][0]
        sources = {'sensor.t_series_generation_power': '1000', 'binary_sensor.t_series_communication': 'off'}
        self.assertEqual(self.render(sensor['availability'], sources), 'False')
        sources['binary_sensor.t_series_communication'] = 'on'
        self.assertEqual(self.render(sensor['availability'], sources), 'True')
        self.assertEqual(self.render(sensor['state'], sources), 'True')
        sources['sensor.t_series_generation_power'] = '5'
        self.assertEqual(self.render(sensor['state'], sources), 'False')

if __name__ == '__main__':
    unittest.main()
