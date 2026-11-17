#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "web_page.h"

// ==========================================
// TIZIM STRUKTURALARI VA O'ZGARUVCHILARI
// ==========================================

struct DoseSlot {
  int hour;
  int minute;
  bool takenToday;
  bool alert1Sent;
  bool alert2Sent;
  unsigned long lastReminderTime;
};

DoseSlot doseSlots[MAX_SCHEDULE_TIMES];
int totalDoseSlots = 0;

String phone1 = "";
String phone2 = "";
int audioVolume = 20;

// Qurilma holati
bool lastPowerState = true;          // true = 220V ulangan
unsigned long powerStateChangeTime = 0;
bool powerAlertPending = false;
bool targetPowerState = true;

bool isLidOpen = false;
bool lastLidState = false;

int gsmSignalPercent = 0;
bool isGsmReady = false;

// DHT22 Harorat va Namlik
float currentTemperature = 22.0f;
float currentHumidity = 45.0f;
bool isDhtValid = false;
unsigned long lastClimateCheckTime = 0;
unsigned long lastTempAlertSmsTime = 0;
unsigned long lastHumidAlertSmsTime = 0;

// DNS va Web server
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);
Preferences prefs;

// Serial portlar
HardwareSerial SerialGSM(2);       // SIM800L
HardwareSerial SerialAudio(1);     // DFPlayer Mini

// ==========================================
// DFPLAYER MINI BUYRUQLARI (Haqiqiy inson ovozi)
// ==========================================

void dfplayer_send_packet(uint8_t cmd, uint16_t param) {
  uint8_t packet[10];
  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = cmd;
  packet[4] = 0x00; // Feedback: no
  packet[5] = (uint8_t)(param >> 8);
  packet[6] = (uint8_t)(param & 0xFF);
  
  uint16_t sum = 0 - (packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (uint8_t)(sum >> 8);
  packet[8] = (uint8_t)(sum & 0xFF);
  packet[9] = 0xEF;

  SerialAudio.write(packet, 10);
  delay(30);
}

void audio_play_track(int trackNumber) {
  Serial.printf("[AUDIO] Trek ijro etilmoqda: %d\n", trackNumber);
  dfplayer_send_packet(0x03, (uint16_t)trackNumber);
}

void audio_set_volume(int vol) {
  if (vol < 0) vol = 0;
  if (vol > 30) vol = 30;
  Serial.printf("[AUDIO] Ovoz balandligi: %d\n", vol);
  dfplayer_send_packet(0x06, (uint16_t)vol);
}

// ==========================================
// SIM800L GSM MODULI BOSHQARUVI
// ==========================================

String gsm_send_cmd(const String& cmd, unsigned long timeout = 1500) {
  while (SerialGSM.available()) SerialGSM.read(); // buferni tozalash
  SerialGSM.println(cmd);
  
  String response = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (SerialGSM.available()) {
      char c = (char)SerialGSM.read();
      response += c;
    }
  }
  return response;
}

bool sim800_send_single_sms(const String& phone, const String& message) {
  if (phone.length() < 7) return false;

  Serial.printf("[GSM] SMS yuborilmoqda: %s -> %s\n", phone.c_str(), message.c_str());

  gsm_send_cmd("AT+CMGF=1", 1000); // Matn rejimi
  gsm_send_cmd("AT+CSCS=\"GSM\"", 1000);

  SerialGSM.print("AT+CMGS=\"");
  SerialGSM.print(phone);
  SerialGSM.println("\"");
  
  delay(250);
  
  SerialGSM.print(message);
  SerialGSM.write(0x1A); // Ctrl+Z

  unsigned long start = millis();
  String resp = "";
  while (millis() - start < 10000) {
    while (SerialGSM.available()) {
      resp += (char)SerialGSM.read();
    }
    if (resp.indexOf("OK") != -1 || resp.indexOf("+CMGS:") != -1) {
      Serial.println("[GSM] SMS muvaffaqiyatli yetkazildi!");
      return true;
    }
    if (resp.indexOf("ERROR") != -1) {
      Serial.println("[GSM] SMS yuborishda xatolik yuz berdi!");
      return false;
    }
  }
  return false;
}

void send_broadcast_sms(const String& message) {
  if (phone1.length() >= 7) {
    sim800_send_single_sms(phone1, message);
    delay(1000);
  }
  if (phone2.length() >= 7) {
    sim800_send_single_sms(phone2, message);
  }
}

void sim800_update_signal() {
  String resp = gsm_send_cmd("AT+CSQ", 1000);
  int idx = resp.indexOf("+CSQ:");
  if (idx != -1) {
    int comma = resp.indexOf(",", idx);
    if (comma != -1) {
      int rssi = resp.substring(idx + 5, comma).toInt();
      if (rssi == 99 || rssi == 0) {
        gsmSignalPercent = 0;
        isGsmReady = false;
      } else {
        gsmSignalPercent = map(rssi, 2, 31, 10, 100);
        if (gsmSignalPercent > 100) gsmSignalPercent = 100;
        isGsmReady = true;
      }
    }
  } else {
    isGsmReady = false;
  }
}

void sim800_sync_time_nitz() {
  // GSM tarmoq minorasidan vaqtni o'qish (agar bor bo'lsa)
  String resp = gsm_send_cmd("AT+CCLK?", 1000);
  // Format: +CCLK: "26/09/06,14:35:00+20"
  int idx = resp.indexOf("\"");
  if (idx != -1) {
    String dt = resp.substring(idx + 1);
    int yr = dt.substring(0, 2).toInt() + 2000;
    int mo = dt.substring(3, 5).toInt();
    int dy = dt.substring(6, 8).toInt();
    int hr = dt.substring(9, 11).toInt();
    int mn = dt.substring(12, 14).toInt();
    int sc = dt.substring(15, 17).toInt();

    if (yr >= 2024 && mo >= 1 && mo <= 12) {
      struct tm t = {0};
      t.tm_year = yr - 1900;
      t.tm_mon  = mo - 1;
      t.tm_mday = dy;
      t.tm_hour = hr;
      t.tm_min  = mn;
      t.tm_sec  = sc;
      time_t epoch = mktime(&t);
      struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
      settimeofday(&tv, NULL);
      Serial.printf("[RTC] GSM minorasidan vaqt olindi: %04d-%02d-%02d %02d:%02d:%02d\n", yr, mo, dy, hr, mn, sc);
    }
  }
}

void sim800_init() {
  SerialGSM.begin(SIM800_BAUD, SERIAL_8N1, SIM800_RX_PIN, SIM800_TX_PIN);
  delay(1000);
  Serial.println("[GSM] SIM800L initsializatsiya qilinmoqda...");
  gsm_send_cmd("AT", 1000);
  gsm_send_cmd("ATE0", 1000);       // Echo o'chirish
  gsm_send_cmd("AT+CMGF=1", 1000);  // Matn rejimi
  gsm_send_cmd("AT+CLIP=1", 1000);  // Qo'ng'iroq ko'rsatish
  sim800_update_signal();
  sim800_sync_time_nitz();
}

// ==========================================
// QUVVAT VA BATAREYA O'LCHASH
// ==========================================

bool check_ac_power() {
  int raw = analogRead(POWER_SENSE_PIN);
  return (raw > POWER_ADC_THRESHOLD);
}

float read_battery_volts() {
  int raw = analogRead(BATTERY_SENSE_PIN);
  float v = (float)raw * BAT_VOLT_FACTOR;
  return v;
}

int calculate_battery_percent(float volts) {
  if (volts >= BAT_MAX_VOLTS) return 100;
  if (volts <= BAT_MIN_VOLTS) return 0;
  int pct = (int)(((volts - BAT_MIN_VOLTS) / (BAT_MAX_VOLTS - BAT_MIN_VOLTS)) * 100.0f);
  return pct;
}

int estimate_remaining_hours(int pct) {
  float remainingMah = BAT_CAPACITY_MAH * ((float)pct / 100.0f);
  int hours = (int)(remainingMah / AVG_CURRENT_MA);
  return (hours > 0) ? hours : 1;
}

// ==========================================
// SOZLAMALARNI SAQLASH VA YUKLASH (NVS)
// ==========================================

void load_settings() {
  prefs.begin("pillbox", false);
  phone1 = prefs.getString("phone1", "+998901234567");
  phone2 = prefs.getString("phone2", "");
  audioVolume = prefs.getInt("volume", 20);

  String timesStr = prefs.getString("times", "08:00,13:00,20:00");
  prefs.end();

  totalDoseSlots = 0;
  int startIdx = 0;
  while (startIdx < timesStr.length() && totalDoseSlots < MAX_SCHEDULE_TIMES) {
    int commaIdx = timesStr.indexOf(',', startIdx);
    String token = (commaIdx == -1) ? timesStr.substring(startIdx) : timesStr.substring(startIdx, commaIdx);
    token.trim();
    if (token.length() >= 4) {
      int colonIdx = token.indexOf(':');
      if (colonIdx != -1) {
        doseSlots[totalDoseSlots].hour = token.substring(0, colonIdx).toInt();
        doseSlots[totalDoseSlots].minute = token.substring(colonIdx + 1).toInt();
        doseSlots[totalDoseSlots].takenToday = false;
        doseSlots[totalDoseSlots].alert1Sent = false;
        doseSlots[totalDoseSlots].alert2Sent = false;
        doseSlots[totalDoseSlots].lastReminderTime = 0;
        totalDoseSlots++;
      }
    }
    if (commaIdx == -1) break;
    startIdx = commaIdx + 1;
  }
  Serial.printf("[CONFIG] Sozlamalar yuklandi. Dori qabul vaqtlari soni: %d\n", totalDoseSlots);
}

void save_settings(const String& p1, const String& p2, int vol, const String& timesStr) {
  prefs.begin("pillbox", false);
  prefs.putString("phone1", p1);
  prefs.putString("phone2", p2);
  prefs.putInt("volume", vol);
  prefs.putString("times", timesStr);
  prefs.end();

  phone1 = p1;
  phone2 = p2;
  audioVolume = vol;
  audio_set_volume(audioVolume);

  // Qayta jadvalga kiritish
  totalDoseSlots = 0;
  int startIdx = 0;
  while (startIdx < timesStr.length() && totalDoseSlots < MAX_SCHEDULE_TIMES) {
    int commaIdx = timesStr.indexOf(',', startIdx);
    String token = (commaIdx == -1) ? timesStr.substring(startIdx) : timesStr.substring(startIdx, commaIdx);
    token.trim();
    if (token.length() >= 4) {
      int colonIdx = token.indexOf(':');
      if (colonIdx != -1) {
        doseSlots[totalDoseSlots].hour = token.substring(0, colonIdx).toInt();
        doseSlots[totalDoseSlots].minute = token.substring(colonIdx + 1).toInt();
        doseSlots[totalDoseSlots].takenToday = false;
        doseSlots[totalDoseSlots].alert1Sent = false;
        doseSlots[totalDoseSlots].alert2Sent = false;
        doseSlots[totalDoseSlots].lastReminderTime = 0;
        totalDoseSlots++;
      }
    }
    if (commaIdx == -1) break;
    startIdx = commaIdx + 1;
  }
}

// ==========================================
// DHT22 HARORAT VA NAMLIK SENSORI
// ==========================================

bool read_dht22_sensor(float* temp, float* hum) {
  uint8_t data[5] = {0, 0, 0, 0, 0};

  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(2); // kamida 1-2 ms LOW signali
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(30);
  pinMode(DHT_PIN, INPUT_PULLUP);

  unsigned long start = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - start > 120) return false;
  }
  start = micros();
  while (digitalRead(DHT_PIN) == LOW) {
    if (micros() - start > 120) return false;
  }
  start = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - start > 120) return false;
  }

  for (int i = 0; i < 40; i++) {
    start = micros();
    while (digitalRead(DHT_PIN) == LOW) {
      if (micros() - start > 120) return false;
    }
    unsigned long tHigh = micros();
    start = micros();
    while (digitalRead(DHT_PIN) == HIGH) {
      if (micros() - start > 150) return false;
    }
    if ((micros() - tHigh) > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return false;
  }

  int rawH = (data[0] << 8) | data[1];
  int rawT = ((data[2] & 0x7F) << 8) | data[3];
  if (data[2] & 0x80) rawT = -rawT;

  *hum = (float)rawH * 0.1f;
  *temp = (float)rawT * 0.1f;
  return true;
}

void check_climate_status() {
  if (millis() - lastClimateCheckTime >= 10000 || lastClimateCheckTime == 0) {
    lastClimateCheckTime = millis();
    float t = 0, h = 0;
    if (read_dht22_sensor(&t, &h)) {
      currentTemperature = t;
      currentHumidity = h;
      isDhtValid = true;
      Serial.printf("[DHT22] Harorat: %.1f °C, Namlik: %.1f %%\n", currentTemperature, currentHumidity);

      // Harorat chegarasi (+25°C)
      if (currentTemperature > TEMP_ALERT_MAX) {
        if (millis() - lastTempAlertSmsTime > CLIMATE_SMS_COOLDOWN || lastTempAlertSmsTime == 0) {
          lastTempAlertSmsTime = millis();
          char buf[160];
          snprintf(buf, sizeof(buf), "OGOHLANTIRISH: Dori qutisi harorati me'yordan oshdi: %.1f C (Me'yor: <=25 C). Dori buzilmasligi uchun salqin joyga oling!", currentTemperature);
          send_broadcast_sms(buf);
        }
      }

      // Namlik chegarasi (65%)
      if (currentHumidity > HUMID_ALERT_MAX) {
        if (millis() - lastHumidAlertSmsTime > CLIMATE_SMS_COOLDOWN || lastHumidAlertSmsTime == 0) {
          lastHumidAlertSmsTime = millis();
          char buf[160];
          snprintf(buf, sizeof(buf), "OGOHLANTIRISH: Dori qutisi namligi me'yordan oshdi: %.1f%% (Me'yor: <=65%%). Silikagel paketini almashtiring!", currentHumidity);
          send_broadcast_sms(buf);
        }
      }
    } else {
      isDhtValid = false;
    }
  }
}

// ==========================================
// WEB SERVER HANDLERLARI (Captive Portal & API)
// ==========================================

void handle_root() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handle_status() {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);
  char clockBuf[16];
  snprintf(clockBuf, sizeof(clockBuf), "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  float bVolts = read_battery_volts();
  int bPct = calculate_battery_percent(bVolts);
  bool ac = check_ac_power();
  bool lid = (digitalRead(LID_SWITCH_PIN) == HIGH); // Ochiq

  String json = "{";
  json += "\"power_ac\":" + String(ac ? "true" : "false") + ",";
  json += "\"lid_open\":" + String(lid ? "true" : "false") + ",";
  json += "\"clock\":\"" + String(clockBuf) + "\",";
  json += "\"gsm_ok\":" + String(isGsmReady ? "true" : "false") + ",";
  json += "\"gsm_signal\":" + String(gsmSignalPercent) + ",";
  json += "\"bat_pct\":" + String(bPct) + ",";
  json += "\"bat_volts\":\"" + String(bVolts, 2) + "\",";
  json += "\"temp\":\"" + String(currentTemperature, 1) + "\",";
  json += "\"humidity\":\"" + String(currentHumidity, 1) + "\",";
  json += "\"dht_ok\":" + String(isDhtValid ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handle_config() {
  String json = "{";
  json += "\"phone1\":\"" + phone1 + "\",";
  json += "\"phone2\":\"" + phone2 + "\",";
  json += "\"volume\":" + String(audioVolume) + ",";
  json += "\"times\":[";
  for (int i = 0; i < totalDoseSlots; i++) {
    char tBuf[10];
    snprintf(tBuf, sizeof(tBuf), "\"%02d:%02d\"", doseSlots[i].hour, doseSlots[i].minute);
    json += tBuf;
    if (i < totalDoseSlots - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handle_save_config() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Ma'lumot topilmadi");
    return;
  }
  String body = server.arg("plain");
  
  // Oddiy JSON parslash
  String p1 = phone1;
  String p2 = phone2;
  int vol = audioVolume;
  String timesStr = "";

  int p1Idx = body.indexOf("\"phone1\":\"");
  if (p1Idx != -1) {
    int end = body.indexOf("\"", p1Idx + 10);
    p1 = body.substring(p1Idx + 10, end);
  }

  int p2Idx = body.indexOf("\"phone2\":\"");
  if (p2Idx != -1) {
    int end = body.indexOf("\"", p2Idx + 10);
    p2 = body.substring(p2Idx + 10, end);
  }

  int vIdx = body.indexOf("\"volume\":");
  if (vIdx != -1) {
    int end = body.indexOf(",", vIdx);
    if (end == -1) end = body.indexOf("}", vIdx);
    vol = body.substring(vIdx + 9, end).toInt();
  }

  int tIdx = body.indexOf("\"times\":[");
  if (tIdx != -1) {
    int tEnd = body.indexOf("]", tIdx);
    String rawTimes = body.substring(tIdx + 9, tEnd);
    rawTimes.replace("\"", "");
    timesStr = rawTimes;
  }

  save_settings(p1, p2, vol, timesStr);
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handle_sync_time() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bo'sh so'rov");
    return;
  }
  String body = server.arg("plain");

  int yr = 2026, mo = 1, dy = 1, hr = 0, mn = 0, sc = 0;
  int idx = body.indexOf("\"year\":");
  if (idx != -1) yr = body.substring(idx + 7).toInt();
  idx = body.indexOf("\"month\":");
  if (idx != -1) mo = body.substring(idx + 8).toInt();
  idx = body.indexOf("\"day\":");
  if (idx != -1) dy = body.substring(idx + 6).toInt();
  idx = body.indexOf("\"hour\":");
  if (idx != -1) hr = body.substring(idx + 7).toInt();
  idx = body.indexOf("\"minute\":");
  if (idx != -1) mn = body.substring(idx + 9).toInt();
  idx = body.indexOf("\"second\":");
  if (idx != -1) sc = body.substring(idx + 9).toInt();

  struct tm t = {0};
  t.tm_year = yr - 1900;
  t.tm_mon  = mo - 1;
  t.tm_mday = dy;
  t.tm_hour = hr;
  t.tm_min  = mn;
  t.tm_sec  = sc;
  time_t epoch = mktime(&t);
  struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
  settimeofday(&tv, NULL);

  Serial.printf("[RTC] Veb-portaldan vaqt yangilandi: %04d-%02d-%02d %02d:%02d:%02d\n", yr, mo, dy, hr, mn, sc);
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handle_test_audio() {
  audio_play_track(1); // 1-eslatma treki
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handle_test_sms() {
  float volts = read_battery_volts();
  int pct = calculate_battery_percent(volts);
  String testMsg = "Aqlli Dori Qutisi: Tizim sozlandi va ishlamoqda. Batareya: " + String(pct) + "% (" + String(volts, 2) + "V).";
  bool ok = sim800_send_single_sms(phone1, testMsg);
  if (ok) {
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(200, "application/json", "{\"status\":\"error\",\"msg\":\"SMS jo'natilmadi\"}");
  }
}

// ==========================================
// DORI JADVALI VA MONITORING LOGIKASI
// ==========================================

void check_medication_schedule() {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);

  int curHour = ptm->tm_hour;
  int curMin  = ptm->tm_min;
  int curTotalMin = curHour * 60 + curMin;

  // Har kuni yarim kechada (00:00 da) kunlik bayroqchalarni yangilash
  static int lastResetDay = -1;
  if (ptm->tm_mday != lastResetDay && curHour == 0 && curMin == 0) {
    for (int i = 0; i < totalDoseSlots; i++) {
      doseSlots[i].takenToday = false;
      doseSlots[i].alert1Sent = false;
      doseSlots[i].alert2Sent = false;
      doseSlots[i].lastReminderTime = 0;
    }
    lastResetDay = ptm->tm_mday;
    Serial.println("[MED] Yangi kun boshlandi. Dori bayroqlari yangilandi.");
  }

  // Qopqoq holatini o'qish (PULLUP: LOW = Yopiq, HIGH = Ochiq)
  isLidOpen = (digitalRead(LID_SWITCH_PIN) == HIGH);

  // Qopqoq endigina ochildimi? (rising edge)
  bool justOpened = (isLidOpen && !lastLidState);

  for (int i = 0; i < totalDoseSlots; i++) {
    int slotTotalMin = doseSlots[i].hour * 60 + doseSlots[i].minute;
    int diff = curTotalMin - slotTotalMin; // daqiqa farqi

    // 1. DORI ICHISH OYNASI: +-15 DAQIQA ORALIG'I
    if (diff >= -MED_WINDOW_MINUTES && diff <= MED_WINDOW_MINUTES) {
      // Agar quti ochilsa va hali bugun ichildi deb belgilanmagan bo'lsa
      if (justOpened && !doseSlots[i].takenToday) {
        doseSlots[i].takenToday = true;
        Serial.printf("[MED] Dori qabul qilindi! Vaqti: %02d:%02d\n", curHour, curMin);
        
        // 2-trek: "Rahmat, dori qabul qilindi"
        audio_play_track(2);

        // SMS xabar jo'natish
        char msgBuf[128];
        snprintf(msgBuf, sizeof(msgBuf), "XABAR: Bemor dori qabul qildi. Vaqti: %02d:%02d. Holat: Me'yorda.", curHour, curMin);
        send_broadcast_sms(msgBuf);
      }

      // Agar vaqti yetib kelgan bo'lsa (0 dan 15 daqiqagacha) va hali ichilmagan bo'lsa: eslatma ovozi
      if (diff >= 0 && !doseSlots[i].takenToday) {
        // Har 2 daqiqada ovozli eslatma berib turish
        if (millis() - doseSlots[i].lastReminderTime > 120000 || doseSlots[i].lastReminderTime == 0) {
          audio_play_track(1); // 1-trek: "Hurmatli bemor, dori ichish vaqti bo'ldi"
          doseSlots[i].lastReminderTime = millis();
        }
      }
    }

    // 2. BELGILANGAN VAQTDAN 15 DAQIQA O'TIB KETDI VA DORI ICHILMADI (1-OGOHLANTIRISH)
    if (diff > MED_WINDOW_MINUTES && diff <= 30) {
      if (!doseSlots[i].takenToday && !doseSlots[i].alert1Sent) {
        doseSlots[i].alert1Sent = true;
        Serial.printf("[MED] 1-ogohlantirish: 15 daqiqa o'tdi, dori ichilmadi! Jadval: %02d:%02d\n", doseSlots[i].hour, doseSlots[i].minute);
        
        audio_play_track(3); // 3-trek: "Diqqat, dori vaqti o'tib bormoqda!"
        
        char msgBuf[140];
        snprintf(msgBuf, sizeof(msgBuf), "OGOHLANTIRISH: Bemor dori ichish vaqtidan 15 daqiqa o'tsa ham dorini ichmadi! Belgilangan vaqt: %02d:%02d.", 
                 doseSlots[i].hour, doseSlots[i].minute);
        send_broadcast_sms(msgBuf);
      }
    }

    // 3. BELGILANGAN VAQTDAN 30 DAQIQA O'TDI VA HALI HAM ICHILMADI (2-YAKUNIY OGOHLANTIRISH)
    if (diff > 30 && diff <= 50) {
      if (!doseSlots[i].takenToday && !doseSlots[i].alert2Sent) {
        doseSlots[i].alert2Sent = true;
        Serial.printf("[MED] 2-ogohlantirish: Hali ham ichilmadi! Jadval: %02d:%02d\n", doseSlots[i].hour, doseSlots[i].minute);
        
        audio_play_track(3);
        
        char msgBuf[140];
        snprintf(msgBuf, sizeof(msgBuf), "SHOSHILINCH: Bemor hali ham dori qabul qilmadi (2-eslatma)! Belgilangan vaqt: %02d:%02d. Iltimos, xabar oling!", 
                 doseSlots[i].hour, doseSlots[i].minute);
        send_broadcast_sms(msgBuf);
      }
    }
  }

  lastLidState = isLidOpen;
}

// ==========================================
// 220V ELEKTR TOKI MONITORINGI
// ==========================================

void check_power_status() {
  bool currentReading = check_ac_power();

  if (currentReading != lastPowerState) {
    if (!powerAlertPending) {
      powerAlertPending = true;
      powerStateChangeTime = millis();
      targetPowerState = currentReading;
    } else {
      // 2.5 soniya davomida o'zgarish barqaror saqlanib turishi kerak (debounce)
      if (millis() - powerStateChangeTime >= POWER_DEBOUNCE_MS) {
        lastPowerState = targetPowerState;
        powerAlertPending = false;

        float volts = read_battery_volts();
        int pct = calculate_battery_percent(volts);
        int estHours = estimate_remaining_hours(pct);

        if (!lastPowerState) {
          // TOK O'CHDI
          Serial.println("[POWER] 220V elektr toki o'chdi!");
          audio_play_track(4); // 4-trek: "Diqqat, elektr toki o'chdi"

          char msgBuf[160];
          snprintf(msgBuf, sizeof(msgBuf), "DIQQAT: 220V elektr toki o'chdi! Dori qutisi batareyada ishlamoqda. Zaryad: %d%% (taxminan %d soatga yetadi).", pct, estHours);
          send_broadcast_sms(msgBuf);
        } else {
          // TOK QAYTA ULINDI
          Serial.println("[POWER] 220V elektr toki qayta ulandi!");
          audio_play_track(5); // 5-trek: "Elektr toki ulandi"

          char msgBuf[140];
          snprintf(msgBuf, sizeof(msgBuf), "XABAR: 220V elektr toki qayta ulandi. Batareya quvvatlanmoqda (%d%%).", pct);
          send_broadcast_sms(msgBuf);
        }
      }
    }
  } else {
    powerAlertPending = false;
  }
}

// ==========================================
// SETUP & LOOP
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==========================================");
  Serial.println("   Aqlli Dori Qutisi (Smart Pill Box)     ");
  Serial.println("==========================================");

  // Pinlar sozlamasi
  pinMode(LID_SWITCH_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // Sozlamalarni xotiradan yuklash
  load_settings();

  // DFPlayer Mini
  SerialAudio.begin(DFPLAYER_BAUD, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(500);
  audio_set_volume(audioVolume);

  // SIM800L
  sim800_init();

  // Wi-Fi Access Point va Captive Portal
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(200);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("[WIFI] Access Point ochildi: ");
  Serial.println(AP_SSID);
  Serial.print("[WIFI] IP manzil: ");
  Serial.println(IP);

  // DNS Server (Captive Portal uchun barcha so'rovlarni 192.168.4.1 ga yo'naltiradi)
  dnsServer.start(DNS_PORT, "*", IP);

  // Web Server marshrutlari
  server.on("/", handle_root);
  server.on("/api/status", HTTP_GET, handle_status);
  server.on("/api/config", HTTP_GET, handle_config);
  server.on("/api/save_config", HTTP_POST, handle_save_config);
  server.on("/api/sync_time", HTTP_POST, handle_sync_time);
  server.on("/api/test_audio", HTTP_GET, handle_test_audio);
  server.on("/api/test_sms", HTTP_GET, handle_test_sms);

  // Android va Apple Captive Portal qo'shimcha so'rovlari
  server.on("/generate_204", handle_root);
  server.on("/fwlink", handle_root);
  server.on("/hotspot-detect.html", handle_root);
  server.onNotFound(handle_root);

  server.begin();
  Serial.println("[WEB] Veb-portal muvaffaqiyatli ishga tushdi!");

  // Boshlang'ich quvvat holatini o'qish
  lastPowerState = check_ac_power();
  lastLidState = (digitalRead(LID_SWITCH_PIN) == HIGH);

  digitalWrite(STATUS_LED_PIN, HIGH);
  Serial.println("[SYSTEM] Aqlli dori qutisi to'liq ishga tayyor!");
}

void loop() {
  // DNS va Web server xizmati
  dnsServer.processNextRequest();
  server.handleClient();

  // Har 1 soniyada dori va quvvat monitoringi
  static unsigned long lastCheckTime = 0;
  if (millis() - lastCheckTime >= 1000) {
    lastCheckTime = millis();

    check_medication_schedule();
    check_power_status();
    check_climate_status();

    // LED holatini yangilash (sekin miltillash)
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
  }

  // Har 30 soniyada GSM signali va tarmoqni tekshirish
  static unsigned long lastGsmCheck = 0;
  if (millis() - lastGsmCheck >= 30000) {
    lastGsmCheck = millis();
    sim800_update_signal();
  }
}
