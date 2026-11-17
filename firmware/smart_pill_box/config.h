#pragma once
#include <Arduino.h>

// ==========================================
// PIN KONFIGURATSIYASI (ESP32 DevKit v1)
// ==========================================

// SIM800L GSM Moduli (Hardware Serial 2)
#define SIM800_RX_PIN        16   // ESP32 RX2 -> SIM800L TX
#define SIM800_TX_PIN        17   // ESP32 TX2 -> SIM800L RX
#define SIM800_BAUD          9600

// DFPlayer Mini MP3 Moduli (Hardware Serial 1)
#define DFPLAYER_RX_PIN      18   // ESP32 RX1 -> DFPlayer TX
#define DFPLAYER_TX_PIN      19   // ESP32 TX1 -> 1k rezistor orqali DFPlayer RX
#define DFPLAYER_BAUD        9600

// Mikrouzgich (Limit switch - Qopqoq sensori)
// Ulanishi: Bir oyog'i GPIO 25 ga, ikkinchi oyog'i GND ga.
// Ichki PULLUP ishlatiladi: Qopqoq yopiq = LOW, Qopqoq ochiq = HIGH.
#define LID_SWITCH_PIN       25   

// DHT22 Harorat va Namlik Sensori
// Ulanishi: VCC -> 3.3V, GND -> GND, DATA -> GPIO 27 (DATA va VCC orasiga 4.7k-10k pull-up, modullarda o'zi bor)
#define DHT_PIN              27

// 220V Tarmoq Quvvati Detektori (5V Sensing)
// Type-C kirishidagi 5V liniyasi 10k + 10k rezistorli bo'lgich orqali ulanadi.
// 5V kelganda GPIO 34 da taxminan 2.5V hosil bo'ladi.
#define POWER_SENSE_PIN      34   // ADC1_CH6 (faqat input)

// 18650 Akkumulyator Kuchlanish Sensori
// Batareyaning (+) klemmasi 100k + 100k rezistorli bo'lgich orqali ulanadi.
// 4.2V kelganda GPIO 35 da 2.1V hosil bo'ladi.
#define BATTERY_SENSE_PIN    35   // ADC1_CH7 (faqat input)

// Holat LED indikatori
#define STATUS_LED_PIN       2    // ESP32 bortidagi ko'k LED

// ==========================================
// TIZIM VA ALGORITM CHEGARALARI
// ==========================================

// Tibbiy Dori Saqlash Me'yorlari (DHT22)
#define TEMP_ALERT_MAX       25.0f // +25°C dan oshsa dori aynishi mumkin
#define HUMID_ALERT_MAX      65.0f // 65% dan oshsa dori nam tortishi mumkin
#define CLIMATE_SMS_COOLDOWN (2 * 3600 * 1000UL) // Har 2 soatda ko'pi bilan 1 marta SMS spam bo'lmasligi uchun

// Dori ichish oynasi (daqiqalarda)
#define MED_WINDOW_MINUTES   15   // Belgilangan vaqtdan +-15 daqiqa ichida

// Quvvat o'chishi/yonishi sezish chegarasi (ADC qiymati: 0-4095)
// 10k/10k bo'lgich bilan: 5V -> ~2.5V (ADC ~3100). Agar < 1200 bo'lsa tok o'chgan deb olinadi.
#define POWER_ADC_THRESHOLD  1500 

// Quvvat o'zgarishini tasdiqlash vaqti (debounce - millisekundlarda)
#define POWER_DEBOUNCE_MS    2500 // Qisqa o'chib-yonishlarni filtrlaydi

// Batareya kuchlanish kalibrlash koeffitsiyenti
// 100k/100k bo'lgich 2 barobar bo'ladi. ESP32 ADC 3.3V / 4095 = 0.000805V
#define BAT_VOLT_FACTOR      (2.0f * 3.3f / 4095.0f * 1.05f) // 1.05f sozlash koeffitsiyenti

// Batareya chegaralari (Volt)
#define BAT_MIN_VOLTS        3.30f // 0%
#define BAT_MAX_VOLTS        4.18f // 100%

// O'rtacha tizim sarfi va batareya sig'imi (soatni taxmin qilish uchun)
#define BAT_CAPACITY_MAH     2600.0f
#define AVG_CURRENT_MA       55.0f  // ESP32 kutish + SIM800L

// Maksimal dori qabul qilish vaqtlari soni
#define MAX_SCHEDULE_TIMES   6

// Wi-Fi Access Point sozlamalari
#define AP_SSID              "AqlliDoriQutisi"
#define AP_PASSWORD          "12345678"
#define AP_IP_OCTET          4   // 192.168.4.1
