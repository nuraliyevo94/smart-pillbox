"""
DFPlayer Mini uchun o'zbek tilidagi audio fayllarni avtomatik yaratish skripti.
Kerakli kutubxona: pip install gTTS
"""
import os

try:
    from gtts import gTTS
except ImportError:
    print("gTTS kutubxonasi o'rnatilmagan. O'rnatish uchun: pip install gTTS")
    exit(1)

OUTPUT_DIR = "MP3"
os.makedirs(OUTPUT_DIR, exist_ok=True)

TRACKS = {
    "0001.mp3": "Hurmatli bemor, dori ichish vaqti bo'ldi. Iltimos, doringizni qabul qiling!",
    "0002.mp3": "Rahmat, dori qabul qilindi deb belgilandi.",
    "0003.mp3": "Diqqat! Dori ichish vaqti o'tib bormoqda, iltimos dorini o'z vaqtida iching!",
    "0004.mp3": "Diqqat, elektr toki o'chdi. Dori qutisi batareyada ishlamoqda.",
    "0005.mp3": "Elektr toki qayta ulandi. Batareya quvvatlanmoqda."
}

print("Ovozli fayllarni yaratish boshlandi...")
for filename, text in TRACKS.items():
    filepath = os.path.join(OUTPUT_DIR, filename)
    print(f"Yaratilmoqda: {filename} -> '{text}'")
    tts = gTTS(text=text, lang="uz", slow=False)
    tts.save(filepath)

print("\nBarcha 5 ta audio fayl muvaffaqiyatli 'MP3' papkasiga saqlandi!")
print("Endi ushbu 'MP3' papkasini to'g'ridan-to'g'ri FAT32 formatlangan MicroSD kartaga ko'chirib o'tkazing.")
