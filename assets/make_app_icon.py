"""Generate the multi-size Windows application icon without external packages."""
import math
import struct
import zlib

SUPERSAMPLE = 4
SIZES = (16, 24, 32, 48, 64, 128, 256)


def color(hex_color):
    value = hex_color.lstrip("#")
    return tuple(bytes.fromhex(value)) + (255,)


def canvas_for(size):
    scale = size * SUPERSAMPLE / 256
    width = size * SUPERSAMPLE
    return bytearray(width * width * 4), width, scale


def paint(pixels, width, x, y, rgba):
    offset = (y * width + x) * 4
    pixels[offset:offset + 4] = bytes(rgba)


def circle(pixels, width, scale, cx, cy, radius, rgba):
    cx, cy, radius = cx * scale, cy * scale, radius * scale
    left, right = max(0, int(cx - radius)), min(width, math.ceil(cx + radius))
    top, bottom = max(0, int(cy - radius)), min(width, math.ceil(cy + radius))
    radius_sq = radius * radius
    for y in range(top, bottom):
        for x in range(left, right):
            if (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= radius_sq:
                paint(pixels, width, x, y, rgba)


def rounded_rect(pixels, width, scale, x, y, w, h, radius, rgba):
    x0, y0, x1, y1, radius = (
        x * scale, y * scale, (x + w) * scale, (y + h) * scale, radius * scale
    )
    left, right = max(0, int(x0)), min(width, math.ceil(x1))
    top, bottom = max(0, int(y0)), min(width, math.ceil(y1))
    for py in range(top, bottom):
        for px in range(left, right):
            nearest_x = min(max(px + 0.5, x0 + radius), x1 - radius)
            nearest_y = min(max(py + 0.5, y0 + radius), y1 - radius)
            if (px + 0.5 - nearest_x) ** 2 + (py + 0.5 - nearest_y) ** 2 <= radius ** 2:
                paint(pixels, width, px, py, rgba)


def polygon(pixels, width, scale, points, rgba):
    points = [(x * scale, y * scale) for x, y in points]
    left = max(0, int(min(x for x, _ in points)))
    right = min(width, math.ceil(max(x for x, _ in points)))
    top = max(0, int(min(y for _, y in points)))
    bottom = min(width, math.ceil(max(y for _, y in points)))
    for py in range(top, bottom):
        for px in range(left, right):
            x, y = px + 0.5, py + 0.5
            inside = False
            for index, (x1, y1) in enumerate(points):
                x2, y2 = points[index - 1]
                if (y1 > y) != (y2 > y) and x < (x2 - x1) * (y - y1) / (y2 - y1) + x1:
                    inside = not inside
            if inside:
                paint(pixels, width, px, py, rgba)


def draw_icon(size):
    pixels, width, scale = canvas_for(size)
    teal = color("#087f78")
    mint = color("#e0fff7")
    panel = color("#075d58")
    moon = color("#ffe79d")

    circle(pixels, width, scale, 128, 128, 116, teal)
    rounded_rect(pixels, width, scale, 57, 91, 142, 83, 13, mint)
    rounded_rect(pixels, width, scale, 65, 99, 126, 67, 7, panel)
    polygon(pixels, width, scale, [(45, 177), (211, 177), (192, 195), (64, 195)], mint)
    rounded_rect(pixels, width, scale, 113, 179, 30, 4, 2, teal)

    circle(pixels, width, scale, 151, 59, 27, moon)
    circle(pixels, width, scale, 164, 49, 25, teal)
    circle(pixels, width, scale, 103, 55, 3.5, mint)
    circle(pixels, width, scale, 113, 43, 2.5, mint)

    small_width, small_pixels = size, bytearray(size * size * 4)
    for y in range(size):
        for x in range(size):
            total = [0, 0, 0, 0]
            for sy in range(SUPERSAMPLE):
                for sx in range(SUPERSAMPLE):
                    source = ((y * SUPERSAMPLE + sy) * width + x * SUPERSAMPLE + sx) * 4
                    for channel in range(4):
                        total[channel] += pixels[source + channel]
            paint(small_pixels, small_width, x, y, [value // (SUPERSAMPLE ** 2) for value in total])
    return encode_png(size, small_pixels)


def png_chunk(kind, data):
    body = kind + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


def encode_png(size, rgba):
    rows = bytearray()
    stride = size * 4
    for y in range(size):
        rows.append(0)
        rows.extend(rgba[y * stride:(y + 1) * stride])
    header = struct.pack(">2I5B", size, size, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", header)
        + png_chunk(b"IDAT", zlib.compress(rows, 9))
        + png_chunk(b"IEND", b"")
    )


def main():
    images = [(size, draw_icon(size)) for size in SIZES]
    offset = 6 + 16 * len(images)
    entries, payload = bytearray(), bytearray()
    for size, data in images:
        entries.extend(struct.pack(
            "<BBBBHHII", size % 256, size % 256, 0, 0, 1, 32, len(data), offset
        ))
        payload.extend(data)
        offset += len(data)
    with open("assets/app.ico", "wb") as icon:
        icon.write(struct.pack("<HHH", 0, 1, len(images)))
        icon.write(entries)
        icon.write(payload)


if __name__ == "__main__":
    main()
