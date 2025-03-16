import logging
from datetime import datetime
from random import choice, shuffle
from threading import Lock, Thread

from app.config import settings
from app.models.immich import ImmichClient
from app.utils.immich_cache import ImmichCache
from PIL.Image import Image

logger = logging.getLogger(__name__)

fill_cache_lock = Lock()

def run_fill_cache(cache: ImmichCache):
    global fill_cache_lock
    if fill_cache_lock.acquire(blocking=False):
        try:
            thread = Thread(target=fill_cache, args=(cache,))
            logger.info("Started cache filling")
            thread.start()
        finally:
            fill_cache_lock.release()
    else:
        logger.info("Cache filling is already running")


def fill_cache(cache: ImmichCache) -> bool:
    portrait = settings.screen_width < settings.screen_height
    client = ImmichClient(settings.immich_base_url, settings.immich_api_key)
    if not client.is_available:
        logger.info("Immich is not available, aborting cache filling")
        return False
    results = []
    page = 1
    next_page = "1"
    while next_page is not None:
        result = client.search(page, settings.immich_cache_cuttof_date)
        results += [
            x["id"]
            for x in result["assets"]["items"]
            if is_portrait(x["exifInfo"]) == portrait
        ]
        next_page = result["assets"]["nextPage"]
        page += 1
    shuffle(results)
    for result in results:
        if cache.store(result):
            logger.info("Cache is full, stopping")
            break
    return True


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
