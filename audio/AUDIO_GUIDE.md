# DFPlayer Mini Uchun Audio Fayllar Qo'llanmasi

DFPlayer Mini moduli ovozlarni to'g'ri ijro etishi uchun microSD kartani to'g'ri tayyorlash zarur.

---

## 1. MicroSD Kartani Formatlash
* **Hajmi:** 1 GB dan 16 GB gacha bo'lgan har qanday MicroSD karta (Class 10 tavsiya etiladi).
* **Fayl tizimi:** **FAT32** formatida formatlangan bo'lishi shart (NTFS yoki exFAT ishlamaydi).
* Karta ichida `MP3` nomli papka ochiladi:
  ```
  [MicroSD Karta]/
  └── MP3/
      ├── 0001.mp3
      ├── 0002.mp3
      ├── 0003.mp3
      ├── 0004.mp3
      └── 0005.mp3
  ```

---

## 2. Audio Fayllar Ro'yxati va Matnlari

| Fayl nomi | Qachon yangraydi | Matni (O'zbek tilida) |
|---|---|---|
| **`0001.mp3`** | Dori vaqti kelganda ($T_{alarm}$) | *"Hurmatli bemor, dori ichish vaqti bo'ldi. Iltimos, qutini oching va doringizni qabul qiling!"* |
| **`0002.mp3`** | Quti $\pm 15$ daqiqa ichida ochilganda | *"Rahmat, dori qabul qilindi deb belgilandi."* |
| **`0003.mp3`** | 15 daqiqadan oshib ketganda (Ogohlantirish) | *"Diqqat! Dori ichish vaqti o'tib bormoqda. Iltimos, doringizni qoldirmasdan iching!"* |
| **`0004.mp3`** | 220V elektr toki o'chganda | *"Diqqat, elektr toki o'chdi. Dori qutisi zaxira batareyasida ishlamoqda."* |
| **`0005.mp3`** | 220V elektr toki qayta ulanganda | *"Elektr toki qayta ulandi. Batareya quvvatlanmoqda."* |

---

## 3. Fayllarni Ovozlashtirish Usullari

Ushbu audio fayllarni yaratish uchun bir nechta juda oson va bepul yo'llar mavjud:

### 1-usul: Telefonda O'z Ovozingiz bilan Yozib Olish (Eng samimiy)
* Bemorning yaqin qarindoshi (farzandi, nevarasi) o'zining yoqimli ovozi bilan telefondagi diktofonda yuqoridagi 5 ta jumlani yozib oladi.
* Internetdagi bepul konverter (masalan, [online-audio-converter.com](https://online-audio-converter.com/uz/)) orqali MP3 formatiga o'tkazib, nomini `0001.mp3`, `0002.mp3` qilib saqlaydi.
* **Tibbiy foydasi:** Bemor o'z farzandining ovozini eshitganda dorini mamnuniyat bilan qabul qiladi.

### 2-usul: Python skripti orqali (Avtomatik Google TTS)
Loyihamizdagi `generate_audio.py` skriptini ishga tushirish:
```bash
python generate_audio.py
```
Bu avtomatik tarzda Google Translate ovozi orqali barcha 5 ta faylni tayyorlab beradi.
