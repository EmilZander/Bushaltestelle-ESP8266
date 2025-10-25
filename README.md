# Bushaltestelle-ESP8266

Dieses Projekt betreibt ein einfaches "Bus-Haltestellen-Türschild" auf einem ESP8266. Das Gerät zeigt zwei Zeilen Text auf einem LCD1602-Display an und lässt sich über ein Webinterface (lokal per WLAN) oder über eine kleine HTTP-API steuern. WLAN-Zugangsdaten und die API-Aktivierungs-Einstellung werden im EEPROM des ESP8266 persistiert.

## Kurzbeschreibung

Das Programm läuft auf einem ESP8266 und stellt einen Webserver bereit. Über die Weboberfläche können zwei Textzeilen eingegeben werden, die auf dem angeschlossenen 16x2 LCD (I2C) angezeigt werden. Zusätzlich gibt es eine optionale HTTP-API, mit der externe Geräte (z. B. ein Smartphone oder ein Server) die Anzeige aktualisieren können.

## Hardware (kurz)

- ESP8266 (z. B. NodeMCU/WeMos/ESP-01/ESP-12)
- LCD 16x2 mit I2C-Adapter (Adresse 0x27 im Code)
- Taster (STOP_BUTTON) an D0 (GPIO16)
- Status-LED an GPIO0

Hinweis: Pins sind in `src/main.cpp` definiert und können bei Bedarf angepasst werden.

## Software-Architektur / Komponenten

- `LiquidCrystal_I2C` zur Ansteuerung des LCD-Displays
- `ESP8266WiFi` / `ESP8266WebServer` für WLAN und HTTP-Server
- `EEPROM` zum Speichern der WLAN-Zugangsdaten und eines Flags für die API-Aktivierung
- `ArduinoJson` zur Erzeugung von JSON-Antworten für die API
- Eingebettete HTML-Seiten (root, preferences, loader) sind als C-String Literale in `main.cpp` enthalten und liefern die Web-UI

## Erklärung zu `Bushaltestelle/src/main.cpp`

Im Folgenden wird der Aufbau und das Verhalten der wichtigsten Programmbereiche beschrieben.

### Includes und globale Objekte

- Es werden Bibliotheken für JSON, WLAN, Webserver, LCD und EEPROM eingebunden.
- Globale Objekte:
	- `LiquidCrystal_I2C lcd(0x27, 16, 2);` — LCD mit I2C-Adresse 0x27
	- `ESP8266WebServer server(80);` — Webserver auf Port 80

### Pin-Definitionen

- `STOP_BUTTON` ist an GPIO16 (D0) angeschlossen.
- `LCD_SDA` und `LCD_SCL` sind kommentiert (D1/D2) — werden aber vom I2C-Library intern verwendet.
- `LED` an GPIO0 als Status-LED.

### Eingebettete HTML-Seiten

`main.cpp` enthält drei HTML-Templates als raw string literals:
- `html_main` — Haupt-Webseite, Eingabefelder für zwei Zeilen und Vorschau.
- `html_prefs` — Seite zum Einstellen von WLAN-SSID/-Passwort und API-Flag.
- `html_loader` — Lade-/Redirect-Seite, die nach Aktionen kurz angezeigt wird.

### Globale Variablen und Status

- `ssid`, `password` — gespeicherte WLAN-Zugangsdaten (String)
- `apiActivated` — boolean, ob die API aktiv ist (wird im EEPROM persistiert)
- `line1Text`, `line2Text` — aktuell angezeigte Texte auf dem LCD
- `line1prevText`, `line2prevText` und `displayTempTextUntil` — für temporäre Anzeigen (z. B. Meldungen)

### EEPROM-Persistenz (fetchPrefs / putPrefs)

- Strukturiertes Speichern der WLAN-Daten und des API-Flags mittels einer `WifiCreds`-Struktur.
- `WIFI_MAGIC` stellt sicher, dass nur valide Daten gelesen werden.
- `fetchPrefs()` liest beim Start die Daten, `putPrefs()` schreibt Änderungen zurück.

### Hilfsfunktionen (Anzeige)

- `displayText(line1, line2)` — aktualisiert die globalen Strings und schreibt auf das LCD.
- `displayTextTemp(line1,line2,time)` — zeigt temporär Text an, speichert vorherigen Display-Inhalt und stellt ihn nach `time` ms wieder her.
- `clearDisplay(row)` — löscht eine oder beide Zeilen.

### Webserver-Routen und Handler

- `GET /` → `handleRoot()` gibt die Hauptseite (`html_main`) zurück.
- `GET /preferences` → `handlePreferences()` zeigt die Einstellungsseite.
- `POST /preferences/apply` → `handlePrefsApply()` speichert SSID/Passwort und verbindet neu zum WLAN.
- `GET /set?Row1=...&Row2=...` → `handleSet()` setzt die Anzeige (über Webformular) und zeigt kurz den Loader, dann Redirect auf `/`.
- `GET /clear[?row=1|2]` → `handleClear()` löscht eine Zeile bzw. beide und redirectet auf `/`.

API-Endpunkte (nur funktionsfähig, wenn `apiActivated == true`):
- `GET /api/get` → `handleAPIget()` liefert aktuelles Display-JSON: { row1, row2 }
- `GET /api/set?Row1=...&Row2=...` → `handleAPISet()` setzt Zeilen per HTTP-API (gibt JSON-Status zurück). Wenn API deaktiviert ist, wird eine Fehl-Antwort zurückgegeben.
- `GET /api/clear[?row=...]` → `handleApiClear()` löscht Zeilen und liefert JSON-Antwort.

### Button- und Display-Handling

- `handleStopButton()` überwacht den Taster (`STOP_BUTTON`). Wenn gedrückt, wird `stopButtonVal` aktualisiert und die LED gesetzt bzw. bei Loslassen eine temporäre Nachricht angezeigt.
- `handleDisplay()` prüft, ob eine temporäre Anzeige abgelaufen ist und stellt ggf. den vorherigen Text wieder her.

### Setup und Loop

- `setup()` initialisiert Serielle Konsole, Pins, LCD, lädt die WLAN-Daten aus dem EEPROM und versucht, sich mit den gespeicherten Zugangsdaten zu verbinden. Falls keine Verbindung möglich ist, wird ein Soft-AP (`BusStopESP`) gestartet.
- Anschließend werden die Webserver-Routen registriert und der Webserver gestartet.
- `loop()` ruft fortlaufend `handleStopButton()`, `handleDisplay()` auf und bedient eingehende HTTP-Anfragen (`server.handleClient()`).

## API / Web-UI — Kurzüberblick

- Web-UI (`/`) erlaubt manuelle Eingabe der zwei Anzeigezeilen (je max. 16 Zeichen) und zeigt eine Vorschau.
- Einstellungen (`/preferences`) erlauben das Setzen von SSID/Passwort sowie das Aktivieren/Deaktivieren der HTTP-API.
- API-Endpunkte (siehe oben) unterstützen einfache JSON-Antworten und können in Automationsszenarien genutzt werden.

## Nutzung / Hinweise

- Gerät in das gewünschte WLAN bringen: `/preferences` aufrufen, SSID + Passwort eingeben, auf "Anwenden" drücken.
- API muss explizit aktiviert werden, damit `GET /api/set` Änderungen erlaubt.
- Zeichenkodierung: Die Web-UI führt einfache Sanitisierung für Umlaute durch (ersetzt z. B. ä→a), da die Anzeige und URL-Encoding vereinfacht sind.

## Annahmen & mögliche Anpassungen

- I2C-Adresse des LCD ist 0x27; ggf. mit einem I2C-Scanner prüfen und in `main.cpp` anpassen.
- GPIO-Belegung (Taster, LED) ist projektspezifisch und kann bei Bedarf geändert werden.
- Sicherheitsaspekte: WLAN-Passwort wird unverschlüsselt im EEPROM gespeichert — für produktive Einsätze ggf. sicherere Speicherung/Authentifizierung überlegen.

## Dateien geändert / relevant

- `Bushaltestelle/src/main.cpp` — Hauptprogramm, Webserver, Anzeige, EEPROM-Handling (Hauptquelle dieser Beschreibung).

Wenn du möchtest, kann ich diese README noch um Diagramme, Beispiel-API-Requests oder ein kurzes How-to zum Kompilieren mit PlatformIO ergänzen.
