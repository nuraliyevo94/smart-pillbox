# Aqlli Dori Qutisi — 3D Korpus Loyihasi va Chop Etish Qo'llanmasi

Mazkur hujjat qurilmaning tibbiy-gigiyenik talablarga javob beruvchi, namlik va yorug'likdan himoyalangan, 3D printerda oson chop etiladigan korpusini yasash bo'yicha to'liq muhandislik ko'rsatmasidir.

---

## 1. Korpus Arxitekturasi (Ikki Bo'linmali Tuzilma)

Tibbiy dori qutisi hech qachon elektronika va batareyalar bilan dori tabletkalarini bitta joyda saqlamasligi kerak (batareya qizishi yoki mikrosxemalar hidi dori sifatiga ta'sir qilmasligi shart).

```
+-------------------------------------------------------------+
|                     ASOSIY QOPQOQ (LID)                     |
|  [Silikon Qistirma] ----------------- [Richagli Mikrouzgich]|
+-------------------------------------------------------------+
|                                                             |
|                 1. TIBBIY DORI KAMERASI                     |
|          (Faqat toza PETG, oson yuviladi,                   |
|           burchakda olinadigan silikagel savatchasi)        |
|                                                             |
+=============================================================+  <-- Germetik To'siq Devor
|                                                             |
|                 2. ELEKTRONIKA VA BATAREYA                  |
|    [ESP32]   [SIM800L]   [DFPlayer + Dinamik]   [18650 UPS] |
|                                                             |
| [Type-C Port]                                  [Ovoz Teshik]|
+-------------------------------------------------------------+
```

### 🖼️ 3D Vizualizatsiya Fayllari:
* **Tashqi Real 3D Ko'rinish:** [`cad/smart_pill_box_3d.jpg`](file:///c:/Users/User/Documents/antigravity/dazzling-volta/cad/smart_pill_box_3d.jpg)
* **Ichki Qatlamlar (Exploded CAD):** [`cad/smart_pill_box_exploded.jpg`](file:///c:/Users/User/Documents/antigravity/dazzling-volta/cad/smart_pill_box_exploded.jpg)
* **Interaktiv 3D Ko'rish Modeli (Brauzerda):** [`cad/3d_viewer.html`](file:///c:/Users/User/Documents/antigravity/dazzling-volta/cad/3d_viewer.html)

---

## 2. 3D Chop Etish Parametrlari (Slicer Settings)

| Parametr | Qiymat | Sababi / Tibbiy talab |
|---|---|---|
| **Material (Filament)** | **PETG** (Qora yoki quyuq to'q ko'k) | Oziq-ovqat va dori bilan kontaktga ruxsat etilgan (FDA compliant), spirt bilan zararsizlantirish mumkin, yorug'lik o'tkazmaydi |
| **Nozzle harorati** | $235^\circ\text{C} - 245^\circ\text{C}$ | Qatlamlar o'zaro jips va monolit yopishishi uchun |
| **Stol (Bed) harorati**| $75^\circ\text{C} - 80^\circ\text{C}$ | Deformatsiya (warping) bo'lmasligi uchun |
| **Qatlam qalinligi** | $0.20\text{mm}$ | Optimal aniqlik va jipslik |
| **Devorlar soni (Perimeters)**| 4 qavat (kamida $1.6\text{mm}$) | Suv va namlik o'tmasligi (watertight) uchun |
| **To'ldirish (Infill)** | $25\% - 30\%$ (Gyroid yoki Grid) | Mustahkamlik va yengillik balansi |

> [!IMPORTANT]
> **Nega PLA emas?**
> PLA biologik eriydigan material bo'lib, namlik va dorilar bilan uzoq muddat tursa mog'or va mikroblar yig'ilishi mumkin. PETG esa mutlaqo gigiyenik, kimyoviy chidamli va tibbiyotda keng qo'llaniladi.

---

## 3. Asosiy Mexanik Bo'limlar va O'lchamlar

### 3.1. Dori Kamerasi (Pill Compartment)
* **O'lchamlari:** $120\text{mm} \times 80\text{mm} \times 45\text{mm}$ (bir necha haftalik blister va flakonlar erkin sig'adi).
* **Burchaklar:** Ichki burchaklar $5\text{mm}$ radiusda yumaloqlanadi (burchaklarda dori kukunlari yoki chang yig'ilib qolmasligi va tozalash oson bo'lishi uchun).
* **Silikagel savatchasi:** Dori kamerasining bir burchagida teshikchali mini savatcha joylashtiriladi (dorixonadan olingan silikagel qopchasini solib qo'yish uchun).

### 3.2. Germetik Silikon Qistirma Kanali
* Qopqoqning pastki yuzasiga butun perimetr bo'ylab chuqurligi $2.2\text{mm}$, kengligi $2.5\text{mm}$ bo'lgan kanal (groove) loyihalanadi.
* Ushbu kanal ichiga $2\text{mm}$ li yumshoq silikon shnur (yoki qurilish do'konlaridan olinadigan sanitariya silikon germetigi) yotqiziladi.
* Qopqoq yopilganda quti ichiga tashqi namlik va chang kirmaydi.

### 3.3. Mikrouzgich (Limit Switch) O'rni
* Richagli mikrouzgich qutining yuqori ichki chetiga 2 ta kichik vint (M2 yoki M2.5) bilan mahkamlanadi.
* Qopqoq yopilganda qopqoqning ichki chiqig'i mikrouzgich richagini bosib, kontaktni tutashtiradi.
* Qopqoq ochilishi bilan richag bo'shab, ESP32 ning GPIO 25 piniga signal beradi.

### 3.4. DHT22 Harorat va Namlik Sensori O'rni
* DHT22 datchigi dori kamerasining yon devoriga (silikagel savatchasi yaqiniga) o'rnatiladi.
* Simlar germetik to'siqdan pastki elektronika kamerasiga o'tadi va simlar teshigi silikon bilan germetik yopiladi (dorilar bo'linmasi to'liq izolyatsiyalangan bo'lishi uchun).

### 3.5. Tashqi Portlar va Dinamik Panjarasi
* **Type-C uyasi:** Qutining orqa tomonida kabel bemalol ulanishi uchun $10\text{mm} \times 4\text{mm}$ o'lchamdagi to'g'ri to'rtburchak teshik.
* **Dinamik panjarasi:** Qutining yon yoki pastki qismida tovush toza chiqishi uchun 10-15 ta $1.5\text{mm}$ diametrli teshiklar panjarasi.
* **LED oynachasi:** ESP32 holat LEDi uchun $3\text{mm}$ teshik.

---

## 4. Yig'ish (Montaj) Ketma-Ketligi

1. 3D printerdan chiqqan PETG korpusni tozalang va ichki dori kamerasini tibbiy spirt bilan artib oling.
2. Pastki elektronika kamerasiga 18650 batareya va UPS modulini mahkamlang (ikki tomonlama mustahkam lenta yoki vintlar bilan).
3. DFPlayer Mini va dinamikni o'z o'rniga joylashtirib, simlarini ulang.
4. SIM800L moduliga $1000\mu\text{F}$ kondensatorni ulab, antennani korpus ichki chetiga yopishtiring.
5. Mikrouzgichni qopqoq ostiga joylashtirib, qopqoq yopilganda aniq "chertillab" bosilayotganini tekshiring.
6. Qopqoq kanaliga silikon qistirma shnurini o'rnating.
7. Silikagel qopchasini o'z savatchasiga joylashtiring. Quti foydalanishga tayyor!
