# Foxess-T-series-ESPHome-Home-Assistant
Read out Foxess T-Series Inverter to Home Assistant by using ESPHome

Compatibale with:
- Foxess T-series

![Screenshot 2022-10-02 153019](https://user-images.githubusercontent.com/106449034/193456801-99b3201c-d321-4a15-a520-aaa8ef501432.jpg)
![Screenshot 2022-10-02 153051](https://user-images.githubusercontent.com/106449034/193456806-b54805a2-b9ea-435d-a3b7-6fe42b514c9a.jpg)

Instructions:
- Use ESPHome 2026.10.2 and copy the YAML together with the `components` directory to your ESPHome directory in Home Assistant
- Fill out individual data in the YAML file (SSID, password, manual ip, etc.)
- Optionally comment sensors you don't need or uncomment sensors you do need

The hardware setup including a wiring diagramm can be found in the [Wiki](https://github.com/assembly12/Foxess-T-series-ESPHome-Home-Assistant/wiki/Hardware-setup).

Designing a custom pcb and enclosure is next on my to do list. I'll update here with the corresponding gerber and stl files when done.

There is some more info being send (like error messages and so on), however this is really not to usefull so I left it out of this component.

Some basic electronics skills (like soldering) are needed to realize this project. I do not take any responsibility for the use of this custom component or anything that it written down in this repository. Use at your own risk.

## Current ESPHome and production statistics

The YAML now uses a local `external_components` implementation compatible with
ESPHome 2026.10.2. Copy the `components` directory with the YAML. The original
working telemetry byte layout is retained; other inverter firmware layouts are
not automatically detected. The root `foxess_t_series.h` is a legacy reference.

For production statistics without a grid meter, setup steps, entity IDs, and
measurement limitations, see [the Polish guide](home-assistant/README_PL.md).
The Home Assistant package includes hourly/daily/weekly/monthly/yearly production,
daily peak power, production time, and specific yield. Actual panel capacity must
be entered separately; inverter rating is not panel capacity.
