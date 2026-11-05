# Statystyki produkcji bez CHINT-a

## ESPHome

Użyj ESPHome 2026.10.2 i skopiuj `foxess-inverter.yaml` razem z katalogiem
`components`. Uzupełnij lokalnie Wi-Fi i hasła OTA/AP. Nie publikuj tych wartości.
Komponent lokalny zastępuje usunięte `sensor.custom`. Stary `foxess_t_series.h`
pozostaje wyłącznie jako archiwalny przykład i nie jest już dołączany do kompilacji.

Zachowano nazwy i identyfikatory dotychczasowych pomiarów oraz **oryginalny układ
bajtów**, który według użytkownika działa z jego falownikiem. Nie zmieniaj firmware
falownika w ramach tej aktualizacji. Inne firmware T-G3 mogą wymagać innego
parsera; nie należy zgadywać przesunięć bajtów na podstawie samego modelu.
Parser sprawdza nagłówek, długość i zakończenie, ale nie sprawdza CRC. Przed użyciem
liczników statystycznych porównaj produkcję dzienną i całkowitą z wyświetlaczem
falownika. Nie jest to implementacja odpytywania rejestrów Modbus.

Dodatkowe encje:
- `T-Series Communication`: poprawna strukturalnie ramka w ostatnich 5 minutach.
- `T-Series Faults`: niezerowe słowa błędów w zapisie szesnastkowym, bez zgadywania
  znaczenia bitów. `None` oznacza wyzerowane pola, a nie brak komunikacji.
- `T-Series PV1/PV2 Energy Estimated`: szacowana energia DC z napięcia i prądu
  odpowiedniego wejścia, całkowanie metodą lewostronną, od momentu uruchomienia.
  To nie są liczniki energii AC ani historyczne dane falownika. Przy utracie
  ramek moc zostaje wyzerowana po 5 minutach: do tego momentu energia może być
  zawyżona; przerwy w pracy ESP32 mogą prowadzić do niedoszacowania. Wynik jest
  przybliżony również z powodu rzadkiego odświeżania telemetrii.

Bez licznika sieciowego `Grid Power` i `loads Power` są wewnętrzne. Nie są
podstawą statystyk importu, eksportu, autokonsumpcji ani oszczędności.

## Home Assistant

1. Dodaj urządzenie ESPHome do Home Assistant.
2. Sprawdź faktyczne identyfikatory encji. Pakiet zakłada:
   - `sensor.t_series_generation_total` (kWh, licznik całkowity),
   - `sensor.t_series_today_yield` (kWh, produkcja dzienna falownika),
   - `sensor.t_series_generation_power` (W, produkcja AC),
   - `binary_sensor.t_series_communication`.
   Nazwy HA mogą mieć prefiks urządzenia lub końcówkę `_2`. Zamień wszystkie
   wystąpienia w pakiecie, jeśli są inne. Dla szablonów też sprawdź końcowe IDs
   `binary_sensor.foxess_producing` oraz `sensor.foxess_peak_power_today`.
3. Włącz pakiety w `configuration.yaml` (połącz z istniejącą sekcją, nie twórz
   drugiej sekcji `homeassistant`):
   ```yaml
   homeassistant:
     packages: !include_dir_named packages
   ```
4. Skopiuj `foxess-production.yaml` do `config/packages/` i wykonaj sprawdzenie
   konfiguracji HA, a następnie restart. Zachowaj te encje w Recorderze.
5. W pomocniku `FoxESS moc paneli` wpisz sumę mocy znamionowej **paneli** w kWp.
   10 kW falownika nie musi oznaczać 10 kWp paneli. Do ustawienia wartości > 0
   uzysk właściwy pozostaje niedostępny. Wartość pomocnika jest przywracana.
6. W panelu Energia jako produkcję fotowoltaiczną wybierz licznik
   `T-Series Generation Total` (energię AC). Nie dodawaj obok niego energii PV1/PV2:
   spowodowałoby to podwójne liczenie tej samej produkcji na różnych etapach.

Pakiet tworzy przyrostowe liczniki godzinowe, dzienne, tygodniowe, miesięczne
oraz roczne. Okresy liczone są według strefy czasowej HA; tydzień zaczyna się
w poniedziałek. Liczniki `utility_meter` rozpoczynają zbieranie od instalacji:
pierwszy okres jest niepełny. Licznik dzienny z falownika pokazuje jego dzień
niezależnie od momentu uruchomienia HA. Nie odtwarzamy archiwum FoxCloud.

Maksimum dnia resetuje się o północy i jest przywracane po restarcie HA; jeśli
HA był wyłączony przez północ, nowa data zeruje poprzednie maksimum. Jest to
maksimum **zaobserwowanych próbek**, nie gwarantowany fizyczny szczyt.
Czas produkcji to czas, gdy moc przekracza 20 W, komunikacja działa i HA zapisuje
historię. Okresy braku komunikacji nie są doliczane po wykryciu timeoutu;
rekonstrukcja przerw w pracy HA nie jest możliwa.

## Sprawdzenie

```bash
esphome config foxess-inverter.yaml
esphome compile --only-generate foxess-inverter.yaml
esphome compile foxess-inverter.yaml
# Test dekodera bez sprzętu:
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=undefined \
  tests/test_protocol.cpp -o /tmp/foxess-protocol-test
/tmp/foxess-protocol-test
```

Testy syntetyczne nie potwierdzają zgodności konkretnego firmware falownika.
Nie wykonuj wgrywania firmware ESP32 bez lokalnego sprawdzenia danych dostępowych.

### Wyniki sprawdzenia w środowisku chmurowym

- ESPHome 2026.10.2: walidacja YAML i generowanie C++ poprawne.
- Test dekodera C++ z UndefinedBehaviorSanitizer: poprawny odczyt produkcji,
  energii i błędów; odrzucanie niepełnych i błędnych ramek; duże wartości PV.
- Trzy testy szablonów: maksimum dnia (restart i zmiana daty), uzysk kWh/kWp
  (także zerowa moc paneli), czas produkcji przy braku komunikacji.
- Pełna kompilacja nie została zakończona: proxy zwróciło HTTP 403 dla
  `dl.espressif.com` podczas pobierania ograniczeń zależności ESP-IDF 5.5.
  Po zastosowaniu odpowiedniej reguły sieciowej trzeba ponowić kompilację.
- Nie uruchamiano rzeczywistego HA ani nie wgrywano firmware do ESP32.
  Pakiet wymaga sprawdzenia konfiguracji w docelowym HA i porównania
  odczytów z falownikiem.
