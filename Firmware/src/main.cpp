#pragma region Libraries
#include <ArduinoJson.h>        // To generate the responses to the API requests
#include <Esp.h>                // To use the GPIOs of the ESP8266 Board
#include <ESP8266WiFi.h>        // To connect to the WiFi
#include <ESP8266WebServer.h>   // To host a WebServer
#include <LiquidCrystal_I2C.h>  // To communicate with the LCD-Display
#include <EEPROM.h>             // To save the WiFi login data
#include <Uri.h>                // To decode URL-encoded strings
using namespace std;
#pragma endregion

#pragma region Pin Definitions
#define STOP_BUTTON 14  // D5
#define LCD_SDA 5       // D2
#define LCD_SCL 4       // D1
#define LED 0           // D3
#pragma endregion

#pragma region HTMLs
#pragma region Main
const char* html_main = R"rawliteral(
<!doctype html>
<html lang="de">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Bushaltestelle</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Bitcount+Grid+Single&display=swap');

    :root {
      --lcd-bg: #000000;
      --lcd-text: #FF8C00;
      --bg: #F0F0F0;
      --card: #FFFFFF;
      --accent: #FFEB3B;
      --muted: #4CAF50;
    }

    html, body { height: 100%; }
    body {
      margin: 0;
      font-family: system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial;
      background: var(--bg);
      color: #333;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
      position: relative;
    }

    .card {
      width: 100%;
      max-width: 540px;
      background: var(--card);
      border-radius: 14px;
      padding: 20px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.2);
      position: relative;
    }

    h1 {
      margin: 0 0 14px 0;
      font-size: clamp(20px,3.4vw,28px);
      text-align: center;
      color: var(--muted);
    }

    form { display: grid; gap: 12px; }

    label {
      font-size: 13px;
      color: var(--muted);
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .input-group {
      position: relative;
      display: flex;
      align-items: center;
      gap: 10px;
    }

    .input-wrapper {
      position: relative;
      flex-grow: 1;
    }

    input[type="text"] {
      width: 100%;
      padding: 12px 44px 12px 14px;
      font-size: 18px;
      border-radius: 8px;
      border: 1px solid #CCCCCC;
      background: #F9F9F9;
      color: #333;
      outline: none;
      box-sizing: border-box;
      font-family: monospace;
      letter-spacing: 1px;
    }

    /* SVG-X im Eingabefeld */
    .x-btn {
      position: absolute;
      right: 10px;
      top: 50%;
      transform: translateY(-50%);
      border: none;
      background: transparent;
      width: 22px;
      height: 22px;
      padding: 0;
      cursor: pointer;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: background 0.15s ease;
    }

    .x-btn:hover, .x-btn:active {
      background: rgba(0,0,0,0.1);
    }

    .x-btn svg {
      width: 14px;
      height: 14px;
      stroke: #888;
      stroke-width: 2;
      stroke-linecap: round;
      stroke-linejoin: round;
    }

    .clear-field-btn {
      background: var(--accent);
      border: none;
      border-radius: 8px;
      padding: 10px 14px;
      cursor: pointer;
      font-size: 14px;
      font-weight: bold;
      color: #000;
      text-decoration: none;
      white-space: nowrap;
      flex-shrink: 0;
    }

    .controls {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      justify-content: center;
      margin-top: 6px;
    }

    .controls button {
      appearance: none;
      border: 0;
      padding: 12px 18px;
      border-radius: 10px;
      font-size: 16px;
      cursor: pointer;
      background: var(--accent);
      color: #000;
      font-weight: 600;
      min-width: 140px;
    }

    /* Vorschau */
    .preview {
      background: var(--lcd-bg);
      border-radius: 6px;
      padding: 6px 16px; /* minimaler Rand */
      font-family: 'Bitcount Grid Single', monospace;
      color: var(--lcd-text);
      box-shadow: inset 0 0 10px #222;
      white-space: nowrap;
      width: calc(16ch + 32px); /* 16 Zeichen + Padding */
      height: calc(2 * 1.5em + 12px); /* 2 Zeilen + Padding */
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: flex-start; /* Text linksbündig */
      margin: 0 auto; /* Box zentriert in Karte */
      overflow: hidden;
      font-size: 36px;
    }

    .row {
      width: 100%;
      line-height: 1.5em; /* kompakter Zeilenabstand */
      letter-spacing: 2px;
      overflow: hidden;
    }

    .settings-btn {
      position: absolute;
      top: 20px;
      right: 20px;
      width: 50px;
      height: 50px;
      background: #FFEB3B;
      border: 3px solid #4CAF50;
      border-radius: 50%;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 24px;
      font-weight: bold;
      color: #4CAF50;
    }

    @media (max-width:420px){
      .card{padding:16px}
      input[type="text"]{font-size:16px;padding:10px 40px 10px 10px}
      .controls button{min-width:120px}
      .settings-btn{width:40px;height:40px;font-size:20px}
      .preview { font-size:28px; height: calc(2 * 1.3em + 12px); }
      .row { line-height:1.3em; }
    }
  </style>
</head>
<body>
  <button class="settings-btn" onclick="window.location.href='/preferences'">E</button>

  <main class="card" role="main">
    <h1>Bushaltestelle</h1>

    <form id="busForm" method="GET" action="/set" autocomplete="off">
      <div>
        <label for="row1">Zeile 1 <span class="hint" id="count1">0/16</span></label>
        <div class="input-group">
          <div class="input-wrapper">
            <input id="row1" name="Row1" type="text" maxlength="16" inputmode="text" aria-label="Zeile 1" />
            <button type="button" class="x-btn" aria-label="Feld leeren" onclick="clearField(1)">
              <svg viewBox="0 0 24 24" fill="none">
                <line x1="5" y1="5" x2="19" y2="19" />
                <line x1="19" y1="5" x2="5" y2="19" />
              </svg>
            </button>
          </div>
          <a href="/clear?row=1" class="clear-field-btn">Löschen</a>
        </div>
      </div>

      <div>
        <label for="row2">Zeile 2 <span class="hint" id="count2">0/16</span></label>
        <div class="input-group">
          <div class="input-wrapper">
            <input id="row2" name="Row2" type="text" maxlength="16" inputmode="text" aria-label="Zeile 2" />
            <button type="button" class="x-btn" aria-label="Feld leeren" onclick="clearField(2)">
              <svg viewBox="0 0 24 24" fill="none">
                <line x1="5" y1="5" x2="19" y2="19" />
                <line x1="19" y1="5" x2="5" y2="19" />
              </svg>
            </button>
          </div>
          <a href="/clear?row=2" class="clear-field-btn">Löschen</a>
        </div>
      </div>

      <div class="controls">
        <button type="submit">Senden</button>
        <button type="button" onclick="window.location.href='/clear'">Löschen</button>
      </div>

      <div class="preview" aria-hidden="true">
        <div id="pv1" class="row">&nbsp;</div>
        <div id="pv2" class="row">&nbsp;</div>
      </div>
    </form>
  </main>

  <script>
    const row1 = document.getElementById('row1');
    const row2 = document.getElementById('row2');
    const count1 = document.getElementById('count1');
    const count2 = document.getElementById('count2');
    const pv1 = document.getElementById('pv1');
    const pv2 = document.getElementById('pv2');
    const form = document.getElementById('busForm');

    function sanitizeText(text){
      const replacements = { 'ä':'a','ö':'o','ü':'u','Ä':'A','Ö':'O','Ü':'U','ß':'ss' };
      let clean = text.replace(/[\n\r]/g, '');
      clean = clean.replace(/[äöüÄÖÜß]/g, m => replacements[m] || '');
      return clean;
    }

    function updatePreview(inputEl, previewEl, countEl){
      inputEl.value = sanitizeText(inputEl.value);
      if(inputEl.value.length > 16) inputEl.value = inputEl.value.slice(0,16);
      countEl.textContent = `${inputEl.value.length}/16`;
      previewEl.textContent = inputEl.value || '\u00A0';
    }

    function clearField(n){
      const field = document.getElementById('row' + n);
      const preview = document.getElementById('pv' + n);
      const count = document.getElementById('count' + n);
      field.value = '';
      updatePreview(field, preview, count);
      field.focus();
    }

    row1.addEventListener('input', ()=> updatePreview(row1,pv1,count1));
    row2.addEventListener('input', ()=> updatePreview(row2,pv2,count2));

    updatePreview(row1,pv1,count1);
    updatePreview(row2,pv2,count2);

    form.addEventListener('submit', ()=>{
      row1.value = encodeURIComponent(sanitizeText(row1.value.slice(0,16)));
      row2.value = encodeURIComponent(sanitizeText(row2.value.slice(0,16)));
    });
  </script>
</body>
</html>
)rawliteral";
#pragma endregion

#pragma region Preferences
const char* html_prefs = R"rawliteral(
<!doctype html>
<html lang="de">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Preferences — Bushaltestelle</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Press+Start+2P&display=swap');

    :root{
      --lcd-bg:#000000;
      --lcd-text:#FF8C00;
      --bg:#F0F0F0;
      --card:#FFFFFF;
      --accent:#FFEB3B;
      --muted:#4CAF50;
    }
    html,body{height:100%;}
    body{
      margin:0;
      font-family:system-ui,-apple-system,Segoe UI,Roboto,Helvetica,Arial;
      background:var(--bg);
      color:#333;
      display:flex;
      flex-direction:column;
      align-items:center;
      justify-content:flex-start;
      padding:20px;
      gap:20px;
      position:relative;
    }

    h1,h2{
      font-size:clamp(18px,3vw,24px);
      color:var(--muted);
      margin:0 0 6px 0;
      text-align:center;
    }

    /* Zurück-Knopf */
    .back-button {
      position:absolute;
      top:20px;
      left:20px;
      width:50px;
      height:50px;
      border-radius:50%;
      border:3px solid #4CAF50;
      background-color:#FFEB3B;
      display:flex;
      align-items:center;
      justify-content:center;
      cursor:pointer;
      padding:0;
    }

    .card{
      width:100%;
      max-width:540px;
      background:var(--card);
      border-radius:14px;
      padding:20px;
      box-shadow:0 4px 20px rgba(0,0,0,0.2);
      display:flex;
      flex-direction:column;
      gap:12px;
    }

    label{font-size:13px;color:var(--muted);display:block}
    .field{display:flex;flex-direction:column;gap:6px;position:relative;}

    input[type="text"], input[type="password"]{
      padding:12px 40px 12px 14px;
      font-size:16px;
      border-radius:8px;
      border:1px solid #CCCCCC;
      background:#F9F9F9;
      color:#333;
      outline:none;
      box-sizing:border-box;
      font-family:monospace;
    }

    .controls{display:flex;gap:10px;flex-wrap:wrap;justify-content:center;margin-top:6px}
    .controls button{
      appearance:none;border:0;padding:12px 18px;border-radius:10px;font-size:16px;cursor:pointer;background:var(--accent);color:#000;font-weight:600;min-width:120px;
    }

    .note{font-size:13px;color:#666;margin-top:6px}

    /* Toggle Switch */
    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 28px;
      margin-right: 12px;
    }

    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 28px;
      transition: 0.4s;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 20px;
      width: 20px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      border-radius: 50%;
      transition: 0.4s;
    }

    input:checked + .slider {
      background-color: var(--muted);
    }

    input:checked + .slider:before {
      transform: translateX(22px);
    }

    .toggle-row {
      display:flex;
      align-items:center;
      justify-content:flex-start;
      gap:12px;
      margin-top:6px;
    }

    @media (max-width:420px){
      .card{padding:16px}
      input[type="text"]{font-size:14px;padding:10px}
      .controls button{min-width:100px}
    }
  </style>
</head>
<body>

  <button class="back-button" onclick="window.location.href='/'" aria-label="Zurück">
    <svg viewBox="0 0 24 24" width="24" height="24">
      <line x1="5" y1="5" x2="19" y2="19" stroke="#4CAF50" stroke-width="3" stroke-linecap="round"/>
      <line x1="19" y1="5" x2="5" y2="19" stroke="#4CAF50" stroke-width="3" stroke-linecap="round"/>
    </svg>
  </button>

  <h2>WLAN-Einstellungen</h2>
  <main class="card">
    <form id="wifiForm" method="POST" action="/preferences/apply" autocomplete="off">
      <div class="field">
        <label for="ssid">WLAN-SSID</label>
        <input id="ssid" name="ssid" type="text" placeholder="Netzwerkname" required maxlength="32" />
      </div>

      <div class="field">
        <label for="pwd">WLAN-Passwort</label>
        <input id="pwd" name="password" type="password" placeholder="Passwort (optional)" maxlength="64" />
        <button type="button" id="togglePwd" style="
          position:absolute; 
          right:10px; 
          top:50%; 
          transform:translateY(-50%);
          background:transparent;
          border:none;
          cursor:pointer;
          padding:0;
        " aria-label="Passwort anzeigen/verbergen">
          <svg id="eyeIcon" viewBox="0 0 24 24" width="20" height="20" fill="none" stroke="#4CAF50" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/>
            <circle cx="12" cy="12" r="3"/>
          </svg>
        </button>
      </div>

      <div class="controls">
        <button type="button" onclick="window.location.href='/'">Abbrechen</button>
        <button type="submit">Anwenden</button>
      </div>

      <div class="note">Hinweis: Änderungen werden beim Klicken auf "Anwenden" an den Server gesendet.</div>
    </form>
  </main>

  <h2>API-Einstellungen</h2>
  <main class="card">
    <form id="apiForm" method="POST" action="/preferences/api" autocomplete="off">
      <div class="toggle-row">
        <label for="apiToggle">API aktivieren</label>
        <label class="toggle-switch">
          <input type="checkbox" id="apiToggle" name="apiEnabled">
          <span class="slider"></span>
        </label>
      </div>
      <div class="controls">
        <button type="submit">Speichern</button>
      </div>
    </form>
  </main>

  <a href="https://github.com/EmilZander/Bushaltestelle-ESP8266">
    <div class="note">GitHub-Repo</div>
  </a>

  <script>
    const ssid = document.getElementById('ssid');
    const pwd = document.getElementById('pwd');
    const togglePwdBtn = document.getElementById('togglePwd');
    const eyeIcon = document.getElementById('eyeIcon');
    const wifiForm = document.getElementById('wifiForm');

    function sanitizeInput(s){
      const repl = { 'ä':'a','ö':'o','ü':'u','Ä':'A','Ö':'O','Ü':'U','ß':'ss' };
      return s.replace(/[\n\r]/g,'').replace(/[äöüÄÖÜß]/g, m=>repl[m]||'');
    }

    ssid.addEventListener('input', ()=>{ ssid.value = sanitizeInput(ssid.value); });
    pwd.addEventListener('input', ()=>{ pwd.value = pwd.value.replace(/[\n\r]/g,''); });

    wifiForm.addEventListener('submit', ()=>{
      ssid.value = encodeURIComponent(sanitizeInput(ssid.value));
      pwd.value = encodeURIComponent(pwd.value);
    });

    togglePwdBtn.addEventListener('click', ()=>{
      if(pwd.type === 'password'){
        pwd.type = 'text';
        eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><line x1="1" y1="1" x2="23" y2="23"/>';
      } else {
        pwd.type = 'password';
        eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/>';
      }
    });
  </script>

</body>
</html>
)rawliteral";
#pragma endregion

#pragma region Loader
const char* html_loader = R"rawliteral(
<!doctype html>
<html lang="de">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Ladebildschirm — Bushaltestelle</title>
  <style>
    :root{
      --sky:#87CEEB;
      --road:#555;
      --bus:#FFD600;
      --highlight:#4CAF50;
    }

    html,body{height:100%;margin:0;background:var(--sky);font-family:system-ui,Segoe UI,Roboto,Arial;overflow:hidden;}

    .loader-screen{position:fixed;inset:0;background:var(--sky);display:flex;flex-direction:column;align-items:center;justify-content:flex-end;overflow:hidden;}

    .clouds{position:absolute;top:10%;left:0;width:400%;display:flex;gap:60px;animation:moveSky 80s linear infinite;z-index:2;}
    .cloud{width:120px;height:70px;background:#fff;border-radius:50%;position:relative;filter:drop-shadow(2px 2px 3px rgba(0,0,0,0.2));}
    .cloud::before,.cloud::after{content:'';position:absolute;background:#fff;border-radius:50%;}
    .cloud::before{width:70px;height:70px;top:-25px;left:20px;}
    .cloud::after{width:90px;height:90px;top:-35px;left:50px;}

    .trees{position:absolute;bottom:120px;left:0;width:400%;display:flex;gap:50px;animation:moveSky 30s linear infinite;z-index:2;}
    .tree svg{height:150px;}

    .road{position:absolute;bottom:0;left:0;width:200%;height:120px;background:var(--road);z-index:1;}
    .lane{position:absolute;bottom:60px;left:0;width:200%;height:6px;background:repeating-linear-gradient(to right, #fff 0 50px, transparent 50px 100px);animation:drive 8s linear infinite;border-radius:3px;}

    .bus{position:absolute;bottom:110px;left:50%;transform:translateX(-50%);width:180px;height:80px;background:var(--bus);border-radius:15px 15px 8px 8px;box-shadow:0 6px 12px rgba(0,0,0,0.3);display:flex;align-items:center;justify-content:space-between;padding:0 12px;animation:busFloat 2s ease-in-out infinite alternate;z-index:3;}
    .window{width:38px;height:30px;background:#4CAF50;border-radius:5px;margin-top:10px;box-shadow:inset 0 2px 3px rgba(0,0,0,0.2);}
    .door{width:28px;height:50px;background:#4CAF50;border-radius:4px;margin-right:6px;box-shadow:inset 0 2px 3px rgba(0,0,0,0.2);}
    .wheel{width:24px;height:24px;background:#111;border-radius:50%;position:absolute;bottom:-12px;box-shadow:0 2px 3px rgba(0,0,0,0.5);}
    .wheel.left{left:30px;}
    .wheel.right{right:30px;}

    .dot-container{position:absolute;top:25%;width:100%;display:flex;justify-content:center;gap:6px;}
    .dot{width:12px;height:12px;background:var(--highlight);border-radius:50%;animation:dotBlink 1.2s infinite ease-in-out;}
    .dot:nth-child(2){animation-delay:0.2s;}
    .dot:nth-child(3){animation-delay:0.4s;}

    .message-box{
      position:absolute;
      bottom:240px;
      background:rgba(255,255,255,0.8);
      color:#111;
      padding:12px 18px;
      border-radius:10px;
      box-shadow:0 2px 6px rgba(0,0,0,0.3);
      font-size:1.1rem;
      text-align:center;
      max-width:80%;
    }

    @keyframes drive{0%{transform:translateX(0);}100%{transform:translateX(-50%);}}
    @keyframes moveSky{0%{transform:translateX(0);}100%{transform:translateX(-50%);}}
    @keyframes dotBlink{0%,80%,100%{opacity:0.3;}40%{opacity:1;}}
    @keyframes busFloat{0%{transform:translateX(-50%) translateY(0);}50%{transform:translateX(-50%) translateY(-5px);}100%{transform:translateX(-50%) translateY(0);}}

    @media (prefers-reduced-motion: reduce){.lane,.clouds,.trees,.dot,.bus{animation:none}}
  </style>
</head>
<body>
  <div class="loader-screen">
    <div class="clouds">
      <div class="cloud"></div><div class="cloud"></div><div class="cloud"></div>
      <div class="cloud"></div><div class="cloud"></div><div class="cloud"></div>
    </div>

    <div class="trees">
      <div class="tree">
        <svg viewBox="0 0 64 128" xmlns="http://www.w3.org/2000/svg">
          <rect x="28" y="80" width="8" height="48" fill="#8B4513"/>
          <circle cx="32" cy="64" r="32" fill="green"/>
        </svg>
      </div>
      <div class="tree">
        <svg viewBox="0 0 64 128" xmlns="http://www.w3.org/2000/svg">
          <rect x="28" y="80" width="8" height="48" fill="#8B4513"/>
          <circle cx="32" cy="64" r="32" fill="green"/>
        </svg>
      </div>
      <div class="tree">
        <svg viewBox="0 0 64 128" xmlns="http://www.w3.org/2000/svg">
          <rect x="28" y="80" width="8" height="48" fill="#8B4513"/>
          <circle cx="32" cy="64" r="32" fill="green"/>
        </svg>
      </div>
      <div class="tree">
        <svg viewBox="0 0 64 128" xmlns="http://www.w3.org/2000/svg">
          <rect x="28" y="80" width="8" height="48" fill="#8B4513"/>
          <circle cx="32" cy="64" r="32" fill="green"/>
        </svg>
      </div>
    </div>

    <div class="road"><div class="lane"></div></div>

    <div class="bus">
      <div class="window"></div>
      <div class="window"></div>
      <div class="door"></div>
      <div class="wheel left"></div>
      <div class="wheel right"></div>
    </div>

    <div class="dot-container">
      <div class="dot"></div><div class="dot"></div><div class="dot"></div>
    </div>

    <div class="message-box">%MESSAGE%</div>
  </div>

  <script>
    window.showBusLoader = ()=> document.querySelector('.loader-screen').style.display='flex';
    window.hideBusLoader = ()=> document.querySelector('.loader-screen').style.display='none';
  </script>
</body>
</html>
)rawliteral";
#pragma endregion
#pragma endregion

#pragma region Variables

#pragma region Globals
LiquidCrystal_I2C lcd(0x27, 16, 2); // The LCD-display
ESP8266WebServer server(80); // The webserver to be hosted
#pragma endregion

// WiFi login data
String ssid;
String password;

bool stopButtonVal = false; // The value of the stop button

bool apiActivated = false; // If the api requests go through (persisted in EEPROM)

// The text shown by the LCD
String line1Text = "";
String line2Text = "";

// The text shown by the LCD before the temp text was displayed
String line1prevText = "";
String line2prevText = "";

// Until this timestamp the temporary text is displayed
unsigned long displayTempTextUntil = 0;
#pragma endregion

#pragma region Method Declaration
void displayText(String line1, String line2);
void displayTextTemp(const String& line1, const String& line2, const int time);
void clearDisplay(int row);
void fetchPrefs();
void putPrefs();
void handleStopButton();
void handleDisplay();
void showLoadingScreen(int statusCode, const String& message);
void handleRoot();
void handlePreferences();
void handleClear();
void handleApiClear();
void handleApiSet();
void handleApiGet();
void handlePrefsApply();
void handlePrefsApi();
void handleSet();
void webServerRoutes();
#pragma endregion

#pragma region Hilfsfunktionen
//=======AUXILIARY FUNCTIONS========//

// Display text on the LCD
void displayText(String line1 = "", String line2 = "") {
  if (line1 != "")
    line1Text = line1;

  if (line2 != "")
    line2Text = line2;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1Text.c_str());
  lcd.setCursor(0, 1);
  lcd.print(line2Text.c_str());
}

// Display text for a set amount of time on the LCD. After the time has expired the current text is displayed again
void displayTextTemp(const String& line1, const String& line2, const int time) {
  line1prevText = line1Text;
  line2prevText = line2Text;
  displayTempTextUntil = millis() + time;
  displayText(line1, line2);
}

void clearDisplay(int row = -1)
{
  switch (row)
  {
  case -1:
    line1Text = "";
    line2Text = "";
    break;
  case 1:
    line1Text = "";
    break;
  case 2:
    line2Text = "";
    break;
  default:
    line1Text = "";
    line2Text = "";
    break;
  }
  displayText();
}

// EEPROM-backed storage for WiFi credentials
struct WifiCreds {
  uint16_t magic;
  uint8_t apiEnabled; // 0 = disabled, 1 = enabled
  char ssid[33];      // 32 chars + null
  char password[65];  // 64 chars + null
};

static const uint16_t WIFI_MAGIC = 0xA5A5;
static const int EEPROM_SIZE_BYTES = 512;
static const int EEPROM_ADDR = 0;

void fetchPrefs()
{
  EEPROM.begin(EEPROM_SIZE_BYTES);
  WifiCreds creds;
  // Read raw struct from EEPROM
  EEPROM.get(EEPROM_ADDR, creds);

  if (creds.magic == WIFI_MAGIC) {
    // Valid data present
    creds.ssid[sizeof(creds.ssid)-1] = '\0';
    creds.password[sizeof(creds.password)-1] = '\0';
    ssid = String(creds.ssid);
    password = String(creds.password);
    // Load persisted API flag
    apiActivated = (creds.apiEnabled != 0);
  } else {
    // No valid data -> use empty strings
    ssid = "";
    password = "";
    // Initialize EEPROM with an empty, valid record to avoid repeated reads
    memset(&creds, 0, sizeof(creds));
    creds.magic = WIFI_MAGIC;
    creds.apiEnabled = 0;
    EEPROM.put(EEPROM_ADDR, creds);
    EEPROM.commit();
  }

  EEPROM.end();
}

void putPrefs()
{
  EEPROM.begin(EEPROM_SIZE_BYTES);
  WifiCreds creds;
  memset(&creds, 0, sizeof(creds));
  creds.magic = WIFI_MAGIC;
  creds.apiEnabled = apiActivated ? 1 : 0;
  // Copy strings with boundary checks
  strncpy(creds.ssid, ssid.c_str(), sizeof(creds.ssid)-1);
  strncpy(creds.password, password.c_str(), sizeof(creds.password)-1);

  EEPROM.put(EEPROM_ADDR, creds);
  EEPROM.commit();
  EEPROM.end();
}
#pragma endregion

#pragma region Handlers
//========HANDLERS========//

// Handle the stop button and how he behaves
void handleStopButton() {
    if (digitalRead(STOP_BUTTON) != stopButtonVal) {
        stopButtonVal = digitalRead(STOP_BUTTON);
        if (stopButtonVal == HIGH) {
          digitalWrite(LED, LOW);
          displayTextTemp("WAGEN HAELT", "WAGEN HAELT", 5000);
        }
        else {
          digitalWrite(LED, HIGH);
        }
    }
}

// Handle the display and make sure that the temporary text is refreshed
void handleDisplay() {
  if (displayTempTextUntil != (unsigned long)0 && millis() >= displayTempTextUntil) {
    displayTempTextUntil = 0;
    displayText(line1prevText, line2prevText);
  }
}
#pragma endregion

#pragma region WebServer
//=======WEBSERVER=======//

// Show the loading screen with a custom message
void showLoadingScreen(int statusCode, const String& message) {
  String html = String(html_loader);
  html.replace("%MESSAGE%", message);
  server.send(statusCode, "text/html", html);
}

// The logic for the root path ("/")
void handleRoot() {
  server.send(200, "text/html", html_main);
}

// The logic for the preferences path ("/preferences")
void handlePreferences() {
  server.send(200, "text/html", html_prefs);
}

// The logic for the clear path ("/clear[?row=-1|1|2]")
void handleClear()
{
  int row = -1;
  if (server.hasArg("row")) row = server.arg("row").toInt();
  clearDisplay(row);

  server.sendHeader("Location", "/"); // Redirect to root
  showLoadingScreen(303, "Lösche Anzeige"); 
}

// The logic for the clear path ("/api/clear[?row=-1|1|2]")
void handleApiClear()
{
  int row = -1;
  if (server.hasArg("row")) row = server.arg("row").toInt();
  clearDisplay(row);

  // The json response
  StaticJsonDocument<200> response;
  response["status"] = "succes";
  response["message"] = "";
  String jsonString;
  serializeJson(response, jsonString);

  server.send(200, "application/json", jsonString);
}

// The logic for the api-set-request ("/api/set?row1=[...]&row2=[...]")
void handleAPISet()
{
  if (!apiActivated)
  {
    // The json response
    StaticJsonDocument<200> response;
    response["status"] = "failed";
    response["message"] = "api not activated";
    String jsonString;
    serializeJson(response, jsonString);

    server.send(200, "application/json", jsonString);
    return;
  }

  String row1 = "";
  String row2 = "";

  // Fetch the parameters from the request
  if (server.hasArg("row1")) row1 = server.arg("row1");
  if (server.hasArg("row2")) row2 = server.arg("row2");

  // Refresh the LCD
  displayText(row1.c_str(), row2.c_str());

  // The json response
  StaticJsonDocument<200> response;
  response["status"] = "succes";
  response["message"] = "";
  String jsonString;
  serializeJson(response, jsonString);

  server.send(200, "application/json", jsonString);
}

// The logic for the api-get path ("api/get")
void handleAPIget() {
  StaticJsonDocument<200> response;
  response["row1"] = line1Text;
  response["row2"] = line2Text;
  String jsonString;
  serializeJson(response, jsonString);

  server.send(200, "application/json", jsonString);
}

void handlePrefsAPI() {
  if (server.hasArg("apiEnabled")) apiActivated = server.arg("apiEnabled") == "on";
  else apiActivated = false;

  // Persist the API setting
  putPrefs();

  server.sendHeader("Location", "/"); // Redirect to root
  showLoadingScreen(303, "Einstellungen werden Eingebaut"); // 303 See Other
}

// The logic for the set path ("/set?row1=[...]&row2=[...]")
void handleSet() {
  String row1 = "";
  String row2 = "";

  // Fetch the parameters from the request
  if (server.hasArg("Row1")) row1 = server.arg("Row1");
  if (server.hasArg("Row2")) row2 = server.arg("Row2");

  // Refresh the LCD
  displayText(row1.c_str(), row2.c_str());

  server.sendHeader("Location", "/"); // Redirect to root
  showLoadingScreen(303, "Anzeige wird aktualisiert");
}

// The logic for the preferences-apply-path ("preferences/apply?ssid=[...]&pwd=[...]")
void handlePrefsApply() {
  // Fetch data from the request
  if(server.hasArg("ssid")) ssid = server.arg("ssid");
  if(server.hasArg("password")) password = server.arg("password");

  putPrefs();

  showLoadingScreen(200, "Bitte wechsel das WLAN und verbinde dich neu mit dem Gerät");
  delay(2000);
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());
}

void setupWebServerRoutes() {
  // Webserver Routes
  server.on("/preferences/api", HTTP_POST, handlePrefsAPI);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/preferences", HTTP_GET, handlePreferences);
  server.on("/preferences/apply", HTTP_POST, handlePrefsApply);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/api/set", HTTP_GET, handleAPISet);
  server.on("/api/get", HTTP_GET, handleAPIget);
  server.on("/clear", HTTP_GET, handleClear);
  server.on("/api/clear", HTTP_GET, handleApiClear);
  
  server.begin();
}
#pragma endregion

#pragma region Logic
void setup() {
  Serial.begin(9600);
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Starte");

  // Fetch the WiFi login-data
  fetchPrefs();

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  uint8_t attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    displayTextTemp("Verbunden", WiFi.localIP().toString().c_str(), 3000);
  } else {
    // Hotspot fallback
    WiFi.mode(WIFI_AP);
    WiFi.softAP("BusStopESP");
    displayTextTemp("Hotspot aktiv:", WiFi.softAPIP().toString().c_str(), 3000);
  }

  setupWebServerRoutes();
}

void loop() {
  handleStopButton();
  handleDisplay();
  server.handleClient();
}
#pragma endregion