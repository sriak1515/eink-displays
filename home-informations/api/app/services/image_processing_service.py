import numpy as np
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


def prepare_image_for_eink(
    image: Image,
    epd_width: int,
    epd_height: int,
    use_pillow_dithering=False,
    dithering_strength=1.0,
    use_cielab=False,
    enhance=1.5,
    contrast=1.2,
) -> Image:
    aspect_ratio = image.width / image.height
    if aspect_ratio > (epd_width / epd_height):
        new_width = epd_width
        new_height = int(epd_width / aspect_ratio)
    else:
        new_height = epd_height
        new_width = int(epd_height * aspect_ratio)
    image = image.resize((new_width, new_height))
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
                epd_width,
                epd_height,
                1 if use_cielab else 0,
            )
        ).quantize(palette=pal_image, dither=Image.Dither.NONE)
    return image
