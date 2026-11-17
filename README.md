# Aqlli Dori Qutisi (Smart Medical Pill Box)

Tibbiy talablarga to'liq javob beruvchi, dori ichish vaqtini inson ovozida eslatuvchi, belgilangan vaqtdan $\pm 15$ daqiqa ichida ochilsa SMS orqali xabar beruvchi, ochilmasa 2 marta qat'iy ogohlantiruvchi, 220V tarmoqdan uzilganda batareyada avtonom ishlovchi va tok holatini SMS orqali bildiruvchi **Aqlli Dori Qutisi** to'liq loyihasi.

---

## 📁 Loyiha Tarkibi

```
dazzling-volta/
├── firmware/
│   ├── platformio.ini               # PlatformIO / VS Code sozlamalari
│   └── smart_pill_box/
│       ├── smart_pill_box.ino       # Asosiy ESP32 dastur kodi (Arduino C++)
│       ├── config.h                 # Pinlar, chegaralar va parametrlar
│       └── web_page.h               # ESP32 xotirasiga o'rnatilgan Web Portal
├── web_ui/
│   └── index.html                   # Brauzerda ochiladigan boshqaruv paneli
├── audio/
│   ├── AUDIO_GUIDE.md               # Ovozli eslatmalar ro'yxati va MicroSD tayyorlash
│   └── generate_audio.py            # Avtomatik o'zbekcha MP3 yaratuvchi skript
├── schematic/
│   └── WIRING_GUIDE.md              # To'liq ulanish sxemasi va rezistorlar
└── cad/
    └── ENCLOSURE_3D_PRINT.md        # 3D printer (PETG) korpusi va tibbiy talablar
```

---

## ⚡ Asosiy Imkoniyatlar

1. **Inson Ovozidagi Eslatma (DFPlayer Mini):**
   - Belgilangan dori vaqti yetib kelganda yoqimli inson ovozida bemorni ogohlantiradi.
2. **$\pm 15$ Daqiqa Oynasi va SMS Xabarnoma:**
   - Vaqtdan $\pm 15$ daqiqa ichida quti ochilsa: *"Bemor dori qabul qildi. Vaqti: HH:MM"* deb belgilangan raqamlarga SMS yuboradi.
   - 15 daqiqa o'tib ketsa va ochilmasa: 1-ogohlantirish SMS yuboriladi va audio yangraydi.
   - 30 daqiqa o'tib ketsa va ochilmasa: 2-ogohlantirish SMS (Shoshilinch) yuboriladi.
3. **Telefon Orqali Qulay Sozlash (Wi-Fi Web Portal):**
   - Telefonda `AqlliDoriQutisi` Wi-Fi tarmog'iga ulanasiz.
   - Brauzerda chiroyli portal ochiladi: telefon raqamlarini, dori vaqtlarini kiritasiz va bir tugma bilan telefon soatini qutiga sinxronlaysiz.
4. **220V Tok va Batareya Monitoringi:**
   - 220V elektr toki o'chganda zudlik bilan SMS boradi: *"DIQQAT: Tok o'chdi! Dori qutisi batareyada ishlamoqda. Zaryad: XX% (~YY soat yetadi)"*.
   - Tok qayta ulanganda: *"XABAR: Tok qayta ulandi. Batareya quvvatlanmoqda"*.
   - 18650 Li-ion batareya qutini bir necha sutka uzluksiz quvvat bilan ta'minlaydi.
5. **Tibbiy-Gigiyenik Standart (PETG):**
   - Dorilar uchun xavfsiz va spirtga chidamli PETG filamenti.
   - Silikon germetik qistirma (namlik va changdan himoya).
   - Silikagel kapsulasi (tabletkalarni quruq saqlash).

---

## 🚀 Tezkor Boshlash (Qadam-baqadam)

### 1-qadam: Audio Fayllarni MicroSD Kartaga Yozish
1. `audio/AUDIO_GUIDE.md` fayliga qarang.
2. MicroSD kartani FAT32 formatida formatlang.
3. Kartada `MP3` papkasi ochib, ichiga 5 ta o'zbekcha audio faylni (`0001.mp3` dan `0005.mp3` gacha) joylang.
4. Kartani DFPlayer Mini moduliga soling.

### 2-qadam: Modullarni Ulash
1. `schematic/WIRING_GUIDE.md` qo'llanmasiga asosan simlarni ulang.
2. SIM800L modulining VCC va GND oyoqlariga parallel $1000\mu\text{F}$ kondensator ulanishini unutmang!
3. Mikrouzgichni GPIO 25 va GND ga ulang.

### 3-qadam: Dasturni ESP32 ga Yuklash
1. **Arduino IDE** dasturini oching.
2. `firmware/smart_pill_box/smart_pill_box.ino` faylini oching.
3. Plata sifatida: **DOIT ESP32 DEVKIT V1** ni tanlang.
4. COM portni tanlab, **Upload** tugmasini bosing.

### 4-qadam: Qutini Telefonda Sozlash
1. Qutini yoqing.
2. Telefoningizning Wi-Fi sozlamalariga kiring va **`AqlliDoriQutisi`** tarmog'iga ulaning (parol: `12345678`).
3. Brauzerda avtomatik ravishda boshqaruv paneli ochiladi (ochilmasa, `192.168.4.1` manziliga kiring).
4. Telefon raqamingizni kiriting, dori vaqtlarini belgilang va **"Telefon Vaqtiga Sinxronlash"** hamda **"Sozlamalarni Saqlash"** tugmalarini bosing.

Hammasi tayyor! Qurilma mustaqil va ishonchli ravishda bemor salomatligini nazorat qiladi.
