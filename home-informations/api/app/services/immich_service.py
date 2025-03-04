from datetime import datetime
from random import choice

from app.config import settings
from app.models.immich import ImmichClient
from PIL.Image import Image


def get_random_image(
    portrait=True,
    after=datetime.fromisoformat("2023-12-15T00:00:00.000Z"),
) -> Image:
    client = ImmichClient(settings.immich_base_url, settings.immich_api_key)
    results = []
    page = 1
    next_page = "1"
    while next_page is not None:
        result = client.search(page, after)
        results += [
            x["id"]
            for x in result["assets"]["items"]
            if is_portrait(x["exifInfo"]) == portrait
        ]
        next_page = result["assets"]["nextPage"]
        page += 1
    return client.get_image(choice(results))


def is_portrait(exif):
    width = exif["exifImageWidth"]
    height = exif["exifImageHeight"]
    if exif["orientation"] == "6":
        return height < width
    else:
        return width < height
