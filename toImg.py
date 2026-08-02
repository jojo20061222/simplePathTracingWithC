from PIL import Image
import sys

if len(sys.argv) <= 2:
    print('Need file name to process')
    print(f'Format: python3 {sys.argv[0]} [raw data] [output png filename]')
    sys.exit(1)

inputFilename = sys.argv[1]
outputFilename = sys.argv[2]

with open(inputFilename) as f:
    sizeInfo = f.readline().split()
    width = int(sizeInfo[0])
    height = int(sizeInfo[1])
    buffer = []

    for i in range(width * height):
        pixel = int(f.readline())
        buffer.append(pixel)

img = Image.new("RGB", (width, height), (0, 0, 0))
pixels = img.load()
assert pixels is not None

for y in range(height):
    for x in range(width):
        pixel = buffer[y * width + x]
        r = pixel % 256
        g = pixel // 256 % 256
        b = pixel // 256 // 256 % 256

        if r > 250 or g > 250 or b > 250:
            print(r, g, b)

        pixels[x, y] = (r, g, b)

img.save(outputFilename)