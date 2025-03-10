import numpy as np
import smartcrop
from image_processing.floyd_steinberg import floyd_steinberg
from PIL import Image, ImageEnhance

epd_palette = np.uint8(
    [
        [0, 0, 0],
        [255, 255, 255],
        [0, 255, 0],
        [0, 0, 255],
        [255, 0, 0],
        [255, 255, 0],
        [255, 128, 0],
    ]
)

def crop_image(image: Image.Image, screen_width: int, screen_height: int) -> Image.Image:
    image_width, image_height = image.size

    image_aspect_ratio = image_width / image_height
    screen_aspect_ratio = screen_width / screen_height

    if image_aspect_ratio > screen_aspect_ratio:
        new_width = int(image_height * screen_aspect_ratio)
        left = (image_width - new_width) / 2
        top = 0
        right = left + new_width
        bottom = image_height
    else:
        new_height = int(image_width / screen_aspect_ratio)
        top = (image_height - new_height) / 2
        left = 0
        bottom = top + new_height
        right = image_width

    cropped_image = image.crop((left, top, right, bottom))

    resized_image = cropped_image.resize((screen_width, screen_height))

#    cropper = smartcrop.SmartCrop()
#    crop = cropper.crop(image, screen_width, screen_height)
#    box = (
#        crop['top_crop']['x'],
#        crop['top_crop']['y'],
#        crop['top_crop']['width'] + crop['top_crop']['x'],
#        crop['top_crop']['height'] + crop['top_crop']['y']
#    )
#    resized_image = image.crop(box)
#    resized_image.thumbnail((screen_width, screen_height), Image.Resampling.LANCZOS)
    return resized_image


def prepare_image_for_eink(
    image: Image.Image,
    use_pillow_dithering=False,
    dithering_strength=1.0,
    use_cielab=False,
    enhance=1.5,
    contrast=1.2,
) -> Image.Image:
    image = ImageEnhance.Color(image).enhance(enhance)
    image = ImageEnhance.Contrast(image).enhance(contrast)

    pal_image = Image.new("P", (1, 1))
    pal_image.putpalette(
        [int(y) for x in epd_palette for y in x] + [0] * (768 - len(epd_palette) * 3)
    )
    if use_pillow_dithering:
        image = image.convert("RGB").quantize(
            palette=pal_image, dither=Image.Dither.FLOYDSTEINBERG
        )
    else:
        image = Image.fromarray(
            floyd_steinberg(
                np.array(image),
                epd_palette,
                dithering_strength,
                1 if use_cielab else 0,
            )
        ).quantize(palette=pal_image, dither=Image.Dither.NONE)
    return image
