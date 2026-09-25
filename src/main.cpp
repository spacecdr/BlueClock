#include "clock_logic.h"
#include "display_fonts.h"
#include "page.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Espalexa.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <sys/time.h>
#include <time.h>

ESP8266WebServer web(80);
DNSServer dns;
WiFiUDP udp;
Espalexa espalexa;
hd44780_I2Cexp lcd;
bool lcdOK = false, fsOK = false, ap = false, connecting = false,
     online = false;
String apName, apPassword;
struct Network {
  String ssid, pass;
};
Network networks[3], candidate;
int networkCount = 0, attempt = 0;
bool pendingCandidate = false, automatic = true, showDate = true;
int font = 0, dateEvery = 20, dateDuration = 5, dots = 0;
bool showYear = true;
int glyphBank = -99;
const char *zones[] = {"CET-1CEST,M3.5.0,M10.5.0/3", "UTC0",
                       "GMT0BST,M3.5.0/1,M10.5.0",   "EST5EDT,M3.2.0,M11.1.0",
                       "PST8PDT,M3.2.0,M11.1.0",     "JST-9"};
const char *zoneNames[] = {"Europe/Rome",         "UTC",
                           "Europe/London",       "America/New_York",
                           "America/Los_Angeles", "Asia/Tokyo"};
int zone = 0;
LightBand lightBands[6] = {{420, 100}, {1320, 20}};
int lightBandCount = 2, brightness = 100, effectiveBrightness = 100;
bool lightAuto = false;
bool alexaEnabled = false, alexaReady = false;
String alexaName = "Orologio";
uint32_t restartAt = 0;
void lightJson(JsonDocument &d) {
  d["brightness"] = brightness;
  d["lightAuto"] = lightAuto;
  JsonArray bands = d.createNestedArray("bands");
  for (int i = 0; i < lightBandCount; i++) {
    JsonObject b = bands.createNestedObject();
    b["minute"] = lightBands[i].minute;
    b["level"] = lightBands[i].level;
  }
}
void lightLoad(JsonDocument &d) {
  brightness = constrain(d["brightness"] | 100, 0, 100);
  lightAuto = d["lightAuto"] | false;
  if (d["bands"].is<JsonArray>()) {
    lightBandCount = 0;
    for (JsonObject b : d["bands"].as<JsonArray>()) {
      int m = b["minute"] | -1, l = b["level"] | -1;
      if (lightBandCount < 6 && m >= 0 && m < 1440 && l >= 0 && l <= 100)
        lightBands[lightBandCount++] = {m, l};
    }
  }
}

uint32_t attemptAt = 0, connectAt = 0, retryAt = 0, ipUntil = 0, clockAt = 0;
uint32_t ntpAt = 0, nextNtp = 0, lastRender = 0;
bool ntpBusy = false, clockValid = false;
time_t lastNtp = 0;
String ntpStatus = "Mai aggiornato", wifiError;
IPAddress ntpIP;
uint8_t ntpToken[8];
void timezoneApply() {
  setenv("TZ", zones[zone], 1);
  tzset();
}
void configJson(JsonDocument &d) {
  lightJson(d);
  d["auto"] = automatic;
  d["date"] = showDate;
  d["year"] = showYear;
  d["dots"] = dots;
  d["font"] = font;
  d["every"] = dateEvery;
  d["duration"] = dateDuration;
  d["zone"] = zone;
  d["alexaEnabled"] = alexaEnabled;
  d["alexaName"] = alexaName;
  JsonArray a = d.createNestedArray("networks");
  for (int i = 0; i < networkCount; i++) {
    JsonObject n = a.createNestedObject();
    n["ssid"] = networks[i].ssid;
    n["pass"] = networks[i].pass;
  }
}
bool save() {
  if (!fsOK)
    return false;
  DynamicJsonDocument d(2048);
  configJson(d);
  File f = LittleFS.open("/config.tmp", "w");
  if (!f)
    return false;
  bool ok = serializeJson(d, f) > 0;
  f.close();
  return ok && LittleFS.rename("/config.tmp", "/config.json");
}
void loadConfig() {
  fsOK = LittleFS.begin();
  if (!fsOK) {
    Serial.println("LittleFS non disponibile");
    return;
  }
  File f = LittleFS.open("/config.json", "r");
  if (!f)
    return;
  DynamicJsonDocument d(2048);
  if (deserializeJson(d, f))
    return;
  lightLoad(d);
  automatic = d["auto"] | true;
  showDate = d["date"] | true;
  font = constrain(d["font"] | 0, 0, 7);
  dots = constrain(d["dots"] | 0, 0, 2);
  showYear = d["year"] | true;
  dateEvery = constrain(d["every"] | 20, 5, 3600);
  dateDuration = constrain(d["duration"] | 5, 2, 60);
  zone = constrain(d["zone"] | 0, 0, 5);
  alexaEnabled = d["alexaEnabled"] | false;
  alexaName = d["alexaName"] | "Orologio";
  alexaName.trim();
  if (!alexaName.length() || alexaName.length() > 32)
    alexaName = "Orologio";
  for (JsonObject n : d["networks"].as<JsonArray>()) {
    String s = n["ssid"] | "", p = n["pass"] | "";
    if (networkCount < 3 && s.length() && s.length() <= 32 && p.length() <= 64)
      networks[networkCount++] = {s, p};
  }
}
void alexaBrightnessChanged(uint8_t value) {
  brightness = constrain((int)lroundf(value * 100.0f / 255.0f), 0, 100);
  lightAuto = false;
  bool ok = save();
  Serial.printf("Alexa: luminosita %d%%, salvataggio %s\n", brightness,
                ok ? "OK" : "fallito");
}
void startAlexa() {
  if (!alexaEnabled || alexaReady || WiFi.status() != WL_CONNECTED)
    return;
  uint8_t initial = constrain((int)lroundf(brightness * 255.0f / 100.0f), 0, 255);
  espalexa.addDevice(alexaName, alexaBrightnessChanged, initial);
  alexaReady = espalexa.begin(&web);
  Serial.printf("Alexa %s: %s\n", alexaReady ? "attiva" : "non avviata",
                alexaName.c_str());
}
void remember() {
  int old = -1;
  for (int i = 0; i < networkCount; i++)
    if (networks[i].ssid == candidate.ssid)
      old = i;
  if (old < 0)
    old = min(networkCount++, 2);
  networkCount = min(networkCount, 3);
  for (int i = old; i > 0; i--)
    networks[i] = networks[i - 1];
  networks[0] = candidate;
  if (!save())
    wifiError = "Rete connessa, ma salvataggio non riuscito";
}
void startAP() {
  if (!ap) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));
    ap = WiFi.softAP(apName.c_str(), apPassword.c_str());
    dns.start(53, "*", IPAddress(192, 168, 4, 1));
    Serial.printf("AP: %s IP 192.168.4.1\n", apName.c_str());
  }
  retryAt = millis() + 60000;
}
void beginAttempt() {
  WiFi.disconnect();
  candidate = networks[attempt];
  WiFi.begin(candidate.ssid.c_str(), candidate.pass.c_str());
  attemptAt = millis();
  connecting = true;
}
void startSaved() {
  if (!networkCount) {
    startAP();
    return;
  }
  attempt = 0;
  connectAt = millis();
  beginAttempt();
}
void stopNtp() {
  udp.stop();
  ntpBusy = false;
}
void ntpStart() {
  if (!automatic || !online || ntpBusy)
    return;
  ntpBusy = true;
  digitalWrite(LED_BUILTIN, LOW);
  ntpAt = millis();
  ntpStatus = "Aggiornamento in corso";
  const char *host = (lastNtp == 0 && ((millis() / 60000) % 2))
                         ? "time.google.com"
                         : "pool.ntp.org";
  if (!WiFi.hostByName(host, ntpIP, 1500)) {
    stopNtp();
    ntpStatus = "DNS non disponibile";
    nextNtp = millis() + 60000;
    return;
  }
  udp.begin(2390);
  uint8_t packet[48] = {0};
  packet[0] = 0x23;
  for (int i = 0; i < 8; i++)
    packet[40 + i] = ntpToken[i] = (uint8_t)random(1, 256);
  udp.beginPacket(ntpIP, 123);
  udp.write(packet, 48);
  udp.endPacket();
}
void ntpLoop(uint32_t now) {
  if (!automatic || !online) {
    if (ntpBusy)
      stopNtp();
    return;
  }
  if (!ntpBusy) {
    if (due(now, nextNtp))
      ntpStart();
    return;
  }
  int size = udp.parsePacket();
  if (size >= 48) {
    uint8_t p[48];
    udp.read(p, 48);
    if (udp.remoteIP() == ntpIP && udp.remotePort() == 123 && (p[0] & 7) == 4 &&
        (p[0] >> 6) != 3 && p[1] > 0 && p[1] < 16 &&
        memcmp(p + 24, ntpToken, 8) == 0) {
      uint32_t s = ((uint32_t)p[40] << 24) | ((uint32_t)p[41] << 16) |
                   ((uint32_t)p[42] << 8) | p[43];
      uint64_t unixTime = unixFromNtp(s);
      if (unixTime >= 1704067200ULL && unixTime < 4102444800ULL) {
        timeval tv = {(time_t)unixTime, 0};
        settimeofday(&tv, nullptr);
        clockValid = true;
        lastNtp = tv.tv_sec;
        ntpStatus = "Aggiornato";
        stopNtp();
        nextNtp = now + 900000;
        Serial.println("NTP aggiornato");
      }
    }
  }
  if (ntpBusy && now - ntpAt > 8000) {
    stopNtp();
    ntpStatus = "Timeout: nuovo tentativo tra 60 s";
    nextNtp = now + 60000;
  }
}
void wifiLoop(uint32_t now) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!online) {
      online = true;
      connecting = false;
      pendingCandidate = false;
      wifiError = "";
      remember();
      if (ap) {
        dns.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        ap = false;
      }
      ipUntil = now + 2000;
      clockAt = ipUntil;
      nextNtp = now;
      Serial.printf("WiFi connesso. IP: %s\n",
                    WiFi.localIP().toString().c_str());
      startAlexa();
    }
    return;
  }
  if (online) {
    online = false;
    stopNtp();
    startSaved();
  }
  if (connecting && now - attemptAt >= 15000) {
    if (pendingCandidate) {
      pendingCandidate = false;
      wifiError = "Connessione fallita: controlla rete e password";
      connecting = false;
      startAP();
    } else if (++attempt < networkCount) {
      beginAttempt();
    } else {
      connecting = false;
      wifiError = "Reti salvate non disponibili";
      startAP();
    }
  }
  if (!connecting && ap && networkCount && due(now, retryAt))
    startSaved();
}
void lightLoop() {
  if (!lcdOK)
    return;
  static uint32_t checked = 0;
  uint32_t now = millis();
  if (now - checked >= 200) {
    checked = now;
    effectiveBrightness = brightness;
    if (lightAuto && clockValid && lightBandCount) {
      time_t n = time(nullptr);
      tm t;
      localtime_r(&n, &t);
      int minute = t.tm_hour * 60 + t.tm_min;
      effectiveBrightness =
          brightnessForMinute(lightBands, lightBandCount, minute, brightness);
    }
    // Keep setup instructions visible even during a scheduled dark period.
    if (ap || connecting || !clockValid)
      effectiveBrightness = 100;
  }
  static bool lit = true;
  bool target = effectiveBrightness == 100 ||
                (effectiveBrightness > 0 &&
                 (micros() % 10000UL) < (unsigned)effectiveBrightness * 100UL);
  if (target != lit) {
    lit = target;
    if (lit)
      lcd.backlight();
    else
      lcd.noBacklight();
  }
}
void line(uint8_t row, String s) {
  static String previous[2];
  while (s.length() < 16)
    s += ' ';
  if (s.length() > 16)
    s.remove(16);
  if (previous[row].length() == 16 &&
      memcmp(s.c_str(), previous[row].c_str(), 16) == 0)
    return;
  previous[row] = s;
  lcd.setCursor(0, row);
  for (unsigned i = 0; i < 16; i++)
    lcd.write((uint8_t)s[i]);
}
void originalGlyphs() { // Six segment tiles plus an accented i, within the
                        // eight CGRAM
                        // slots.
  uint8_t g[7][8] = {
      {31, 31, 31, 0, 0, 0, 0, 0},      {0, 0, 0, 0, 0, 31, 31, 31},
      {31, 31, 31, 0, 0, 31, 31, 31},   {31, 31, 31, 31, 31, 31, 31, 31},
      {28, 28, 28, 28, 28, 28, 28, 28}, {7, 7, 7, 7, 7, 7, 7, 7},
      {8, 4, 0, 12, 4, 4, 14, 0}};
  if (font == 1) {
    for (int i = 0; i < 6; i++)
      for (int j = 0; j < 8; j++) {
        if (i == 0)
          g[i][j] = (j == 1) ? 31 : 0;
        if (i == 1)
          g[i][j] = (j == 6) ? 31 : 0;
        if (i == 2)
          g[i][j] = (j == 1 || j == 6) ? 31 : 0;
        if (i == 3)
          g[i][j] = 17;
        if (i == 4)
          g[i][j] = 16;
        if (i == 5)
          g[i][j] = 1;
      }
  }
  for (int i = 0; i < 7; i++)
    lcd.createChar(i, g[i]);
}
// Fonts may consume all eight slots. Reload the accented i only for date mode.
void glyphs(bool dateMode = false) {
  int bank = dateMode ? -1 : font;
  if (bank == glyphBank)
    return;
  line(0, "");
  line(1, "");
  if (dateMode) {
    uint8_t accent[8] = {8, 4, 0, 12, 4, 4, 14, 0};
    lcd.createChar(6, accent);
  } else if (font < 3) {
    originalGlyphs();
  } else {
    ExtraFont f = extraFont(font - 3);
    for (int i = 0; i < f.patternCount; i++) {
      uint8_t bitmap[8];
      for (int y = 0; y < 8; y++)
        bitmap[y] = pgm_read_byte(&f.patterns[i][y]);
      lcd.createChar(i, bitmap);
    }
  }
  glyphBank = bank;
}
void extraTime(tm &t) {
  uint8_t rows[2][16];
  renderExtraFont(font - 3, t.tm_hour, t.tm_min, colonVisible(dots, t.tm_sec),
                  rows);
  for (int row = 0; row < 2; row++) {
    String s = "                ";
    for (int i = 0; i < 16; i++)
      s.setCharAt(i, rows[row][i]);
    line(row, s);
  }
}
void bigTime(tm &t) {
  const uint8_t segments[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66,
                                0x6d, 0x7d, 0x07, 0x7f, 0x6f};
  int digits[4] = {t.tm_hour / 10, t.tm_hour % 10, t.tm_min / 10,
                   t.tm_min % 10};
  String top = "                ", bottom = top;
  int positions[4] = {0, 4, 9, 13};
  for (int i = 0; i < 4; i++) {
    int x = positions[i];
    uint8_t s = segments[digits[i]];
    top.setCharAt(x, (s & 32) ? (font == 1 ? 4 : 3) : ' ');
    top.setCharAt(x + 2, (s & 2) ? (font == 1 ? 5 : 3) : ' ');
    bottom.setCharAt(x, (s & 16) ? (font == 1 ? 4 : 3) : ' ');
    bottom.setCharAt(x + 2, (s & 4) ? (font == 1 ? 5 : 3) : ' ');
    top.setCharAt(x + 1, (s & 1) ? ((s & 64) ? 2 : 0) : ((s & 64) ? 1 : ' '));
    bottom.setCharAt(x + 1, (s & 8) ? 1 : ' ');
  }
  top.setCharAt(8, colonVisible(dots, t.tm_sec) ? '.' : ' ');
  bottom.setCharAt(8, colonVisible(dots, t.tm_sec) ? '.' : ' ');
  line(0, top);
  line(1, bottom);
}
void displayLoop(uint32_t now) {
  if (!lcdOK || now - lastRender < 150)
    return;
  lastRender = now;
  if (connecting && (!ap || pendingCandidate)) {
    line(0, "Connetto... " + String((now - connectAt) / 1000) + "s");
    line(1, candidate.ssid);
    return;
  }
  if (ap) {
    switch ((now / 3500) % 3) {
    case 0:
      line(0, "Collegati al WiFi");
      line(1, apName);
      break;
    case 1:
      line(0, "Password AP:");
      line(1, apPassword);
      break;
    case 2:
      line(0, "Apri nel browser");
      line(1, "192.168.4.1");
      break;
    }
    return;
  }
  if (ipUntil) {
    if (!due(now, ipUntil)) {
      line(0, "WiFi connesso");
      line(1, WiFi.localIP().toString());
      return;
    }
    ipUntil = 0;
  }
  if (!clockValid) {
    line(0, automatic ? "Attendo ora NTP" : "Imposta l'ora");
    line(1, WiFi.localIP().toString());
    return;
  }
  time_t n = time(nullptr);
  tm t;
  localtime_r(&n, &t);
  if (showDate && ((now - clockAt) / 1000) % (dateEvery + dateDuration) >=
                      (unsigned)dateEvery) {
    const char *days[] = {"Domenica",     "Luned\006",  "Marted\006",
                          "Mercoled\006", "Gioved\006", "Venerd\006",
                          "Sabato"};
    const char *months[] = {"Gennaio",   "Febbraio", "Marzo",    "Aprile",
                            "Maggio",    "Giugno",   "Luglio",   "Agosto",
                            "Settembre", "Ottobre",  "Novembre", "Dicembre"};
    glyphs(true);
    line(0, String(days[t.tm_wday]) +
                (showYear ? " " + String(t.tm_year + 1900) : ""));
    line(1, String(t.tm_mday) + " " + months[t.tm_mon]);
  } else if (font != 2) {
    glyphs();
    if (font < 2)
      bigTime(t);
    else
      extraTime(t);
  } else {
    char b[20];
    glyphs();
    strftime(b, sizeof(b), "    %H:%M:%S", &t);
    if (!colonVisible(dots, t.tm_sec))
      b[6] = b[9] = ' ';
    line(0, b);
    line(1, "   OROLOGIO WiFi");
  }
}
bool writeAllowed() {
  if (web.header("X-Orologio") != "1") {
    web.send(403, "text/plain", "Richiesta non autorizzata");
    return false;
  }
  return true;
}
void respond(int code, const String &message) {
  DynamicJsonDocument d(256);
  d["message"] = message;
  String s;
  serializeJson(d, s);
  web.send(code, "application/json", s);
}
void setupWeb() {
  web.collectHeaders("X-Orologio");
  web.on("/", HTTP_GET,
         [] { web.send_P(200, "text/html; charset=utf-8", PAGE); });
  web.on("/api/status", HTTP_GET, [] {
    DynamicJsonDocument d(3072);
    lightJson(d);
    d["effectiveBrightness"] = effectiveBrightness;
    d["auto"] = automatic;
    d["date"] = showDate;
    d["year"] = showYear;
    d["dots"] = dots;
    d["font"] = font;
    d["every"] = dateEvery;
    d["duration"] = dateDuration;
    d["zone"] = zone;
    d["zoneName"] = zoneNames[zone];
    d["alexaEnabled"] = alexaEnabled;
    d["alexaName"] = alexaName;
    d["alexaReady"] = alexaReady;
    d["connected"] = online;
    d["connecting"] = connecting;
    d["ap"] = ap;
    d["ip"] = online ? WiFi.localIP().toString() : "192.168.4.1";
    d["ssid"] = online ? WiFi.SSID() : "";
    d["error"] = wifiError;
    d["ntp"] = ntpStatus;
    d["lastNtp"] = (long long)lastNtp;
    d["storage"] = fsOK;
    d["lcd"] = lcdOK;
    char b[32] = "Ora da impostare";
    if (clockValid) {
      time_t n = time(nullptr);
      tm t;
      localtime_r(&n, &t);
      strftime(b, sizeof(b), "%d/%m/%Y %H:%M:%S", &t);
    }
    d["time"] = b;
    JsonArray a = d.createNestedArray("networks");
    for (int i = 0; i < networkCount; i++)
      a.add(networks[i].ssid);
    String out;
    serializeJson(d, out);
    web.send(200, "application/json", out);
  });
  web.on("/api/scan", HTTP_GET, [] {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) {
      web.send(202, "application/json", "[]");
      return;
    }
    if (n == WIFI_SCAN_FAILED) {
      WiFi.scanNetworks(true);
      web.send(202, "application/json", "[]");
      return;
    }
    DynamicJsonDocument d(4096);
    JsonArray a = d.to<JsonArray>();
    for (int i = 0; i < n && i < 25; i++) {
      JsonObject o = a.createNestedObject();
      o["ssid"] = WiFi.SSID(i);
      o["rssi"] = WiFi.RSSI(i);
      o["open"] = WiFi.encryptionType(i) == ENC_TYPE_NONE;
    }
    String out;
    serializeJson(d, out);
    WiFi.scanDelete();
    web.send(200, "application/json", out);
  });
  web.on("/api/wifi", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    String s = web.arg("ssid"), p = web.arg("password");
    if (!s.length() || s.length() > 32 || p.length() > 64 ||
        (p.length() > 0 && p.length() < 8)) {
      respond(400, "SSID o password non validi");
      return;
    }
    if (connecting) {
      respond(409, "Connessione già in corso; attendi");
      return;
    }
    respond(200, "Connessione avviata. Se cambia rete, riapri il pannello "
                 "all'IP mostrato sul display.");
    startAP();
    online = false;
    candidate = {s, p};
    pendingCandidate = true;
    connectAt = attemptAt = millis();
    connecting = true;
    WiFi.disconnect();
    WiFi.begin(s.c_str(), p.c_str());
  });
  web.on("/api/forget", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    String s = web.arg("ssid");
    for (int i = 0; i < networkCount; i++)
      if (networks[i].ssid == s) {
        for (int j = i; j < networkCount - 1; j++)
          networks[j] = networks[j + 1];
        networkCount--;
        break;
      }
    respond(save() ? 200 : 500,
            fsOK ? "Rete rimossa dalla cache" : "Memoria non disponibile");
  });
  web.on("/api/settings", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    int f = web.arg("font").toInt(), z = web.arg("zone").toInt(),
        e = web.arg("every").toInt(), d = web.arg("duration").toInt(),
        dotMode = web.hasArg("dots") ? web.arg("dots").toInt() : dots;
    if (dotMode < 0 || dotMode > 2 || f < 0 || f > 7 || z < 0 || z > 5 ||
        e < 5 || e > 3600 || d < 2 || d > 60) {
      respond(400, "Impostazioni fuori intervallo");
      return;
    }
    automatic = web.arg("auto") == "1";
    showDate = web.arg("date") == "1";
    if (web.hasArg("year"))
      showYear = web.arg("year") == "1";
    dots = dotMode;
    font = f;
    zone = z;
    dateEvery = e;
    dateDuration = d;
    timezoneApply();
    if (lcdOK)
      glyphs();
    clockAt = millis();
    if (!automatic)
      stopNtp();
    nextNtp = millis();
    bool ok = save();
    respond(ok ? 200 : 500,
            ok ? "Impostazioni salvate"
               : "Impostazioni applicate, ma salvataggio fallito");
  });
  web.on("/api/light", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    DynamicJsonDocument d(1024);
    if (deserializeJson(d, web.arg("plain")) || !d["brightness"].is<int>() ||
        !d["lightAuto"].is<bool>() || !d["bands"].is<JsonArray>()) {
      respond(400, "Configurazione luminosità non valida");
      return;
    }
    int level = d["brightness"].as<int>();
    JsonArray a = d["bands"].as<JsonArray>();
    if (level < 0 || level > 100 || a.size() > 6 ||
        (d["lightAuto"].as<bool>() && a.size() == 0)) {
      respond(400, "Servono da 1 a 6 fasce e luminosità 0–100");
      return;
    }
    int seen[6];
    int seenCount = 0;
    for (JsonObject b : a) {
      int m = b["minute"] | -1, l = b["level"] | -1;
      bool duplicate = false;
      for (int i = 0; i < seenCount; i++)
        if (seen[i] == m)
          duplicate = true;
      if (m < 0 || m >= 1440 || l < 0 || l > 100 || duplicate) {
        respond(400, "Orari duplicati o fascia non valida");
        return;
      }
      seen[seenCount++] = m;
    }
    lightLoad(d);
    bool ok = save();
    respond(ok ? 200 : 500,
            ok ? "Luminosità salvata"
               : "Luminosità applicata, ma salvataggio fallito");
  });
  web.on("/api/alexa", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    bool enabled = web.arg("enabled") == "1";
    String name = web.arg("name");
    name.trim();
    if (!name.length() || name.length() > 32) {
      respond(400, "Il nome Alexa deve contenere da 1 a 32 caratteri");
      return;
    }
    for (unsigned i = 0; i < name.length(); i++) {
      if ((uint8_t)name[i] < 32) {
        respond(400, "Il nome Alexa contiene caratteri non validi");
        return;
      }
    }
    bool changed = alexaEnabled != enabled || alexaName != name;
    alexaEnabled = enabled;
    alexaName = name;
    if (!save()) {
      respond(500, "Impostazione applicata, ma salvataggio fallito");
      return;
    }
    if (changed) {
      respond(200, enabled
                       ? "Alexa salvata. L'orologio si riavvia: poi avvia la ricerca dispositivi nell'app Alexa."
                       : "Alexa disattivata. L'orologio si riavvia.");
      restartAt = millis() + 1200;
    } else {
      respond(200, "Impostazioni Alexa già attive");
    }
  });
  web.on("/api/time", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    if (automatic) {
      respond(409, "Seleziona e salva prima la modalità manuale");
      return;
    }
    String value = web.arg("local");
    tm t = {};
    int year, month, day, hour, minute, second = 0;
    int fields = sscanf(value.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day,
                        &hour, &minute, &second);
    if (fields < 5 || year < 2024 || year > 2099 || month < 1 || month > 12 ||
        day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 ||
        minute > 59 || second < 0 || second > 59) {
      respond(400, "Data o ora non valida");
      return;
    }
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = -1;
    time_t epoch = mktime(&t);
    if (t.tm_mon != month - 1 || t.tm_mday != day || t.tm_hour != hour) {
      respond(400, "Data inesistente o ora nel cambio legale");
      return;
    }
    timeval tv = {epoch, 0};
    settimeofday(&tv, nullptr);
    clockValid = true;
    respond(200, "Ora impostata nel fuso selezionato");
  });
  web.on("/api/sync", HTTP_POST, [] {
    if (!writeAllowed())
      return;
    if (!automatic || !online) {
      respond(409, "Servono WiFi e modalità automatica");
      return;
    }
    nextNtp = millis();
    respond(200, "Sincronizzazione richiesta");
  });
  web.onNotFound([] {
    if (alexaReady && espalexa.handleAlexaApiCall(web.uri(), web.arg(0)))
      return;
    web.sendHeader(
        "Location",
        "http://" + (ap ? String("192.168.4.1") : WiFi.localIP().toString()) +
            "/",
        true);
    web.send(302, "text/plain", "");
  });
  web.begin();
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_STA);
  apName = "Orologio-" + String(ESP.getChipId() & 0xffff, HEX);
  apPassword = "Ora-" + String(ESP.getChipId(), HEX);
  loadConfig();
  timezoneApply();
  Wire.begin(D2, D1);
  lcdOK = lcd.begin(16, 2) == 0;
  if (lcdOK) {
    lcd.backlight();
    glyphs();
    line(0, "Avvio orologio");
    line(1, "");
  }
  Serial.printf("LCD: %s; SDA D2, SCL D1\n", lcdOK ? "OK" : "non trovato");
  setupWeb();
  startSaved();
}
void loop() {
  if (Serial.available() && Serial.read() == '?') {
    Serial.printf(
        "STATUS LCD=%s FS=%s AP=%d WiFi=%d heap=%u IP=%s\n",
        lcdOK ? "OK" : "FAIL", fsOK ? "OK" : "FAIL", ap, online,
        ESP.getFreeHeap(),
        (online ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str());
  }
  uint32_t now = millis();
  if (alexaReady)
    espalexa.loop();
  else
    web.handleClient();
  if (ap)
    dns.processNextRequest();
  wifiLoop(now);
  ntpLoop(millis());
  digitalWrite(LED_BUILTIN, (ap || connecting)
                                ? ((millis() / 350) % 2 ? HIGH : LOW)
                                : (ntpBusy ? LOW : HIGH));
  displayLoop(millis());
  lightLoop();
  if (restartAt && due(millis(), restartAt)) {
    delay(50);
    ESP.restart();
  }
  delay(0);
}
