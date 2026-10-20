"""Génère les icônes de la version web installable (écran d'accueil du téléphone).
Usage : python3 scripts/web/generate_icons.py   (nécessite Pillow)
"""

import os

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
font_path = os.path.join(HERE, "..", "..", "assets", "fonts", "Orbitron.ttf")


def icon(size, maskable=False):
    s = 4 * size
    glow = Image.new("RGB", (s, s), (0, 0, 0))
    d = ImageDraw.Draw(glow)
    m = s * (0.2 if maskable else 0.12)
    k = 0.8 if maskable else 1.0  # tout est resserré vers le centre pour l'icône "maskable"
    def Y(v): return s / 2 + (v - 0.5) * s * k
    for i in range(5):
        y = Y(0.76 + (i ** 1.5) * 0.025)
        d.line([(m, y), (s - m, y)], fill=(0, 90, 130), width=max(2, s // 160))
    d.line([(m, Y(0.44)), (s * 0.55, Y(0.44)), (s * 0.55, Y(0.24)), (s - m, Y(0.24))], fill=(40, 200, 255), width=s // 28, joint="curve")
    font = ImageFont.truetype(font_path, int(s * 0.2 * k))
    tw = d.textlength("TRON", font=font)
    d.text(((s - tw) / 2, Y(0.5)), "TRON", font=font, fill=(150, 235, 255))
    img = ImageChops.add(glow, glow.filter(ImageFilter.GaussianBlur(s / 60)))
    return img.resize((size, size), Image.LANCZOS)


if __name__ == "__main__":
    icon(192).save(os.path.join(HERE, "icon-192.png"))
    icon(512).save(os.path.join(HERE, "icon-512.png"))
    icon(512, maskable=True).save(os.path.join(HERE, "icon-maskable-512.png"))
    icon(180).save(os.path.join(HERE, "apple-touch-icon.png"))
