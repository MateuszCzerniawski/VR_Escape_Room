import base64
import os

import numpy as np
import cv2


def quick_show(image, name='test'):
    cv2.imshow(name, image)
    cv2.waitKey(0)


def optional_rotate(image):
    if image.shape[1] <= image.shape[0]:  # width<=height
        image = cv2.rotate(image, cv2.ROTATE_90_CLOCKWISE)
    return image


def invert(image):
    image = image.copy()
    if (len(image.shape)) != 2:
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    height, width = image.shape
    for x in range(width):
        for y in range(height):
            image[y][x] = 255 - image[y][x]
    return image


def clean_noise(mask, radius=2):
    kernel = np.ones((radius, radius), np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    return mask


def auto_mask(image):
    if (len(image.shape)) != 2:
        image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    thresh, mask = cv2.threshold(image, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    hist = cv2.calcHist([image], [0], None, [256], [0, 256])
    dark, light = 0, 0
    thresh = thresh if thresh > 0 else 1
    for i in range(int(thresh)):
        dark += hist[i][0]
    for i in range(int(thresh), len(hist)):
        light += hist[i][0]
    mask = invert(mask) if light > dark else mask
    return mask


def shift_image(image, shift):
    matrix = np.float32([[1, 0, shift[0]], [0, 1, shift[1]]])
    dsize = (image.shape[1], image.shape[0])
    return cv2.warpAffine(image, matrix, dsize, borderMode=cv2.BORDER_CONSTANT, borderValue=(0, 0, 0))


def shift_to_center(image):
    mask = auto_mask(image)
    M = cv2.moments(mask)
    x, y = int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"])
    height, width = mask.shape
    return shift_image(image, (width // 2 - x, height // 2 - y))


def decode_image(grb_string, width, height):
    rgb = list(map(int, grb_string.strip("[]").split(",")))
    rgb = np.array(rgb, dtype=np.uint8).reshape(-1, 3)
    bgr = rgb[:, ::-1]
    image = bgr.reshape((height, width, 3))
    # cv2.imwrite(f'tests/received/{len(os.listdir('tests/received'))}.png', image)
    return image


class Manipulator:
    def __init__(self, size, border):
        self.background = None
        self.size = size
        self.border = border

    def set_background(self, background):
        background = optional_rotate(background)
        self.background = background

    def parse_img(self, image):
        image = optional_rotate(image)
        diff = cv2.absdiff(image, self.background)
        gray = cv2.cvtColor(diff, cv2.COLOR_BGR2GRAY)
        mask = auto_mask(gray)
        mask = clean_noise(mask)
        #cv2.imwrite(f'tests/tmp/{len(os.listdir('tests/tmp'))}.png', image)
        countours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        parsed = []
        for c in countours:
            x, y, w, h = cv2.boundingRect(c)
            img = mask[y:y + h, x:x + w]
            scaling = (self.border, int(h * (self.border / w))) if w > h else (int(w * (self.border / h)), self.border)
            img = cv2.resize(img, scaling, interpolation=cv2.INTER_LINEAR)

            img = cv2.copyMakeBorder(img, 0, self.size - scaling[1], 0, self.size - scaling[0], cv2.BORDER_CONSTANT,
                                     value=(0, 0, 0))
            img = shift_to_center(img)
            if img[0][0] != 0: #skipping objects that couldn't be shifted properly
                continue
            parsed.append((img, x, y, w, h))
        parsed = sorted(parsed, key=lambda i: i[1])
        images = [invert(img) for img, x, y, w, h in parsed]
        horizontals = [int(x) for i, x, y, w, h in parsed]
        verticals = [int(y) for i, x, y, w, h in parsed]
        widths = [int(w) for i, x, y, w, h in parsed]
        heights = [int(h) for i, x, y, w, h in parsed]
        # for img in images:
        #     cv2.imwrite(f'tests/tmp/{len(os.listdir('tests/tmp'))}.png', img)
        # cv2.imwrite(f'tests/parsed/{len(os.listdir('tests/parsed'))}.png', cv2.hconcat(images))
        return images, horizontals, verticals, widths, heights
