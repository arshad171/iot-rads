import os
from pathlib import Path
import time
import numpy as np
from PIL import Image
import serial

NUM_IMAGES = 10
DATA_FOLDER = "images"

PORT = "/dev/cu.usbmodem21401"
BAUD = 9600
WIDTH = 320
HEIGHT = 240
BYTES_PER_PIXEL = 2
BYTES_PER_FRAME = WIDTH * HEIGHT * BYTES_PER_PIXEL

BEGIN_IMAGE_TAG = b"BEGIN IMAGE\r\n"


def read_image(ser):
    print("waiting for image...")

    line = ser.readline()
    while line != BEGIN_IMAGE_TAG:
        print(line)
        line = ser.readline()

    image = []
    for row_ix in range(HEIGHT):
        row = []
        for col_ix in range(WIDTH):
            raw_pix = ser.read(BYTES_PER_PIXEL)
            pix = int.from_bytes(raw_pix, "big")

            row.append(pix)
        image.append(row)

    return image


def decode_image(image):
    decoded_image = Image.new("RGB", (WIDTH, HEIGHT))
    for row_ix in range(HEIGHT):
        for col_ix in range(WIDTH):
            pix = image[row_ix][col_ix]

            # r = (pix & 0xF800) >> 11
            # g = (pix & 0x07E0) >> 5
            # b = pix & 0x001F

            r = ((pix >> 11) & 0x1F) << 3
            g = ((pix >> 5) & 0x3F) << 2
            b = ((pix >> 0) & 0x1F) << 3

            decoded_image.putpixel((col_ix, row_ix), (r, g, b))

    return decoded_image


def main():
    Path(DATA_FOLDER).mkdir(exist_ok=True)

    with serial.Serial(PORT, BAUD) as ser:
        for counter in range(NUM_IMAGES):
            ser.write(b"c")

            image = read_image(ser)

            image = decode_image(image)

            image.save(os.path.join(DATA_FOLDER, f"image-{counter}.jpg"))

            time.sleep(5)


if __name__ == "__main__":
    main()
