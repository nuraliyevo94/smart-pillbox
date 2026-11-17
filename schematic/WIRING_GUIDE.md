# Aqlli Dori Qutisi — To'liq Sxemotexnika va Ulanish Qo'llanmasi

Mazkur qo'llanma barcha elektron modullarni o'zaro to'g'ri, xavfsiz va barqaror ulash uchun mo'ljallangan.

---

## 1. Asosiy Blok-Sxema va Quvvat Taqsimoti

```mermaid
flowchart TD
    subgraph Quvvat [Tashqi Quvvat & Zaxira]
        PWR_IN["Type-C 5V / 2A Kirish"] --> UPS["18650 UPS Zaryadlovchi Platasi"]
        BAT["18650 Li-ion Batareya (3.7V - 4.2V)"] <--> UPS
        UPS --> V5["5V Quvvat Linisasi"]
        BAT --> VBAT["Batareya To'g'ridan-to'g'ri Linisasi (VBAT)"]
    end

    subgraph Monitoring [Quvvat Nazorati]
        PWR_IN --> DIV_5V["10k / 10k Bo'lgich"] --> ESP_P34["ESP32 GPIO 34 (Tok Detektor)"]
        BAT --> DIV_BAT["100k / 100k Bo'lgich"] --> ESP_P35["ESP32 GPIO 35 (Batareya ADC)"]
    end

    subgraph ESP32_Core [ESP32 DevKit v1]
        V5 --> ESP_VIN["ESP32 5V (VIN) Pini"]
        ESP_GND["ESP32 GND"]
    end

    subgraph GSM_Block [SIM800L Aloqa]
        VBAT --> CAP["1000 uF 16V Kondensator"] --> SIM_VCC["SIM800L VCC (3.7V - 4.2V)"]
        ESP_P17["ESP32 GPIO 17 (TX2)"] --> SIM_RX["SIM800L RXD"]
        ESP_P16["ESP32 GPIO 16 (RX2)"] <-- SIM_TX["SIM800L TXD"]
    end

    subgraph Audio_Block [DFPlayer Mini MP3]
        V5 --> DF_VCC["DFPlayer VCC (5V)"]
        ESP_P19["ESP32 GPIO 19 (TX1)"] --> R1K["1k Rezistor"] --> DF_RX["DFPlayer RX"]
        ESP_P18["ESP32 GPIO 18 (RX1)"] <-- DF_TX["DFPlayer TX"]
        DF_SPK["SPK_1 & SPK_2"] --> SPEAKER["8 Ohm Dinamik"]
    end

    subgraph Sensors [Sensor & Indikator]
        LID_SW["Mikrouzgich (Limit Switch)"] --> ESP_P25["ESP32 GPIO 25 (INPUT_PULLUP)"]
        ESP_P2["ESP32 GPIO 2 (LED)"] --> LED["Holat LEDi"]
    end
```

---

## 2. To'liq Pinlar Ulanish Jadvali

| Qaysi moduldan | Modul pini | Qayerga ulanadi | ESP32 pini / Nuqta | Qo'shimcha element / Izoh |
|---|---|---|---|---|
| **ESP32** | VIN (5V) | UPS 5V Chiqishi | UPS +5V | Asosiy mikrokontroller quvvati |
| **ESP32** | GND | Umumiy massa | Umumiy GND | Barcha modullar massasi bitta bo'lishi shart |
| **SIM800L** | VCC | 18650 Batareya (+) | Batareya (+) klemmasi | Parallel **$1000\mu\text{F}$ kondensator** ulanadi |
| **SIM800L** | GND | Umumiy GND | Umumiy GND | Kondensator (-) qutbi ham shu yerga |
| **SIM800L** | TXD | ESP32 RX2 | **GPIO 16** | To'g'ridan-to'g'ri ulanadi |
| **SIM800L** | RXD | ESP32 TX2 | **GPIO 17** | To'g'ridan-to'g'ri ulanadi |
| **DFPlayer** | VCC | 5V Quvvat shinasiga | 5V | Kuchli ovoz uchun 5V maqbul |
| **DFPlayer** | GND | Umumiy GND | Umumiy GND | |
| **DFPlayer** | RX | ESP32 TX1 | **GPIO 19** | Ketma-ket **$1\text{k}\Omega$ rezistor** orqali |
| **DFPlayer** | TX | ESP32 RX1 | **GPIO 18** | To'g'ridan-to'g'ri ulanadi |
| **DFPlayer** | SPK_1 | Dinamik 1-oyoq | Dinamik (+) | 8 Ohm 1W-2W karnay |
| **DFPlayer** | SPK_2 | Dinamik 2-oyoq | Dinamik (-) | |
| **Mikrouzgich** | COM (Umumiy) | GND | Umumiy GND | |
| **Mikrouzgich** | NO yoki NC | ESP32 | **GPIO 25** | Ichki PULLUP: Bosilganda LOW, bo'shaganda HIGH |
| **DHT22 Sensori** | VCC | 3.3V | ESP32 3V3 | Harorat & namlik datchigi |
| **DHT22 Sensori** | GND | Umumiy GND | Umumiy GND | |
| **DHT22 Sensori** | DATA | ESP32 | **GPIO 27** | VCC va DATA orasiga 4.7k-10k pull-up (modulda o'zi bo'ladi) |
| **5V Tok Detektor** | Type-C 5V (+) | Rezistorli bo'lgich | **GPIO 34** | $10\text{k}\Omega + 10\text{k}\Omega$ bo'lgich |
| **Batareya ADC** | 18650 Bat (+) | Rezistorli bo'lgich | **GPIO 35** | $100\text{k}\Omega + 100\text{k}\Omega$ bo'lgich |

---

## 3. Rezistorli Bo'lgichlar Sxemasi

### 3.1. 220V (5V Type-C) Tok Detektori (GPIO 34):
ESP32 ning ADC kirishlari maksimal $3.3\text{V}$ gacha kuchlanishni qabul qiladi. 5V to'g'ridan-to'g'ri ulansa mikrokontroller kuyadi. Shuning uchun kuchlanish 2 barobarga pasaytiriladi:
```
Type-C 5V Kirish  ----[ 10k Rezistor ]----+----[ 10k Rezistor ]---- GND
                                          |
                                          +----> ESP32 GPIO 34
```
* **Tok borida:** Kuchlanish $\approx 2.5\text{V}$ bo'ladi (ADC: ~3100).
* **Tok o'chganida:** Kuchlanish $0\text{V}$ bo'ladi (ADC: 0).

### 3.2. Batareya Zaryadini O'lchash (GPIO 35):
18650 Li-ion batareyasining to'liq zaryadlangan kuchlanishi $4.2\text{V}$ ga yetadi:
```
Batareya (+) 4.2V ----[ 100k Rezistor ]----+----[ 100k Rezistor ]---- GND
                                           |
                                           +----> ESP32 GPIO 35
```
* **To'liq zaryadda ($4.2\text{V}$):** Bo'lgichdan keyin $2.1\text{V}$ chiqadi (ESP32 uchun mutlaqo xavfsiz).
* **Bo'sh zaryadda ($3.3\text{V}$):** Bo'lgichdan keyin $1.65\text{V}$ chiqadi.

---

## 4. Muhim Muhandislik Qoidalari (Stabil ishlash siri)

1. **SIM800L va $1000\mu\text{F}$ Kondensator:**
   * SIM800L SMS yuborayotgan soniyada mobil minora bilan bog'lanish uchun birdaniga 2 Amper tok so'raydi.
   * Agar SIM800L ning VCC va GND oyoqlariga iloji boricha yaqin joyda **$1000\mu\text{F}$ (10V yoki 16V)** elektrolitik kondensator qo'yilmasa, modul qayta yuklanib (reset) bo'ladi va SMS ketmaydi.
   * Kondensator qutblarini adashtirmang: uzun oyoqcha (+), bo'ylama chiziqli kalta oyoqcha (-).

2. **DFPlayer Mini $1\text{k}\Omega$ Rezistor:**
   * ESP32 mantiqiy sathi 3.3V, DFPlayer esa 5V da ishlaganda Serial aloqada shovqin yoki chiyillash bo'lmasligi uchun ESP32 TX (GPIO 19) bilan DFPlayer RX orasiga **$1\text{k}\Omega$ rezistor** qo'yiladi.

3. **Umumiy Massa (GND):**
   * Barcha qismlarning (ESP32, SIM800L, DFPlayer, UPS platasi, Mikrouzgich) `GND` liniyasi bir-biriga mustahkam ulanishi shart!
