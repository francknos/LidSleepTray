"""Regenerate setup.ico / uninstall.ico (needs Pillow). Run: python assets/make_icons.py"""
import math
from PIL import Image, ImageDraw

S = 1024  # supersampled canvas; 128-unit design space like TrayApp::makeIcon
K = S / 128

def pt(x, y): return (x * K, y * K)

def base(active):
    im = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([6 * K, 6 * K, 122 * K, 122 * K], fill="#16a34a" if active else "#4b5263")
    if active:  # sun
        d.ellipse([42 * K, 42 * K, 86 * K, 86 * K], fill="white")
        for i in range(8):
            a = math.radians(i * 45)
            x0, y0 = 64 + 34 * math.sin(a), 64 - 34 * math.cos(a)
            x1, y1 = 64 + 44 * math.sin(a), 64 - 44 * math.cos(a)
            d.line([pt(x0, y0), pt(x1, y1)], fill="white", width=int(8 * K))
            for x, y in ((x0, y0), (x1, y1)):
                d.ellipse([(x - 4) * K, (y - 4) * K, (x + 4) * K, (y + 4) * K], fill="white")
    else:  # crescent
        moon = Image.new("L", (S, S), 0)
        md = ImageDraw.Draw(moon)
        md.ellipse([28 * K, 30 * K, 96 * K, 98 * K], fill=255)
        md.ellipse([50 * K, 22 * K, 110 * K, 82 * K], fill=0)
        im.paste(Image.new("RGBA", (S, S), "white"), mask=moon)
    return im

def badge(im, color, glyph):
    d = ImageDraw.Draw(im)
    cx, cy, r = 94, 94, 30
    d.ellipse([(cx - r - 4) * K, (cy - r - 4) * K, (cx + r + 4) * K, (cy + r + 4) * K], fill="#0f1419")
    d.ellipse([(cx - r) * K, (cy - r) * K, (cx + r) * K, (cy + r) * K], fill=color)
    w = int(7 * K)
    if glyph == "down":  # arrow into a tray
        d.line([pt(cx, cy - 14), pt(cx, cy + 6)], fill="white", width=w)
        d.line([pt(cx - 11, cy - 4), pt(cx, cy + 8), pt(cx + 11, cy - 4)], fill="white", width=w, joint="curve")
        d.line([pt(cx - 14, cy + 16), pt(cx + 14, cy + 16)], fill="white", width=w)
    else:  # cross
        d.line([pt(cx - 12, cy - 12), pt(cx + 12, cy + 12)], fill="white", width=w)
        d.line([pt(cx - 12, cy + 12), pt(cx + 12, cy - 12)], fill="white", width=w)
    return im

def save(im, name):
    big = im.resize((256, 256), Image.LANCZOS)
    big.save(name, format="ICO", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    big.save(name.replace(".ico", ".png"))

here = __file__.rsplit("/", 1)[0] if "/" in __file__ else "."
save(badge(base(True), "#2563eb", "down"), f"{here}/setup.ico")
save(badge(base(False), "#dc2626", "x"), f"{here}/uninstall.ico")
