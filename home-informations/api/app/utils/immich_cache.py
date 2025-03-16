import logging
import os
import re
import shutil
from collections import OrderedDict
from dataclasses import dataclass
from typing import Annotated, Tuple, Union

from fastapi import Depends
from PIL import Image
from ulid import ULID

from app.config import settings
from app.models.immich import ImmichClient
from app.services.image_processing_service import crop_image, prepare_image_for_eink
from app.utils.helpers import human_readable_to_bytes, remove_empty_folders

filename_regex = re.compile(
    r"^([0-9A-Z]{26})_([0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12})(\....)$"
)

logger = logging.getLogger(__name__)

class ImmichCache:
    def __init__(
        self,
        immich_client: ImmichClient,
        base_dir: str,
        max_size: Union[str, int],
        screen_size: Tuple[int, int] = (480, 800),
    ):
        if isinstance(max_size, str):
            max_size = human_readable_to_bytes(max_size)
        self.max_size = max_size

        if os.path.exists(base_dir):
            if os.path.isfile(base_dir):
                raise ValueError("Cache directory exists, but is a file")
        else:
            os.makedirs(base_dir)

        self.immich_client = immich_client
        self.base_dir = base_dir
        self.cache_size = 0
        self.cache_files: OrderedDict[str, ImmichCache.Entry] = OrderedDict()
        self.screen_size = screen_size
        self.refresh()

    def store(self, immich_uuid: str) -> bool:
        if immich_uuid in self.cache_files:
            return False
        image = self.immich_client.get_image(immich_uuid)
        image = crop_image(image, self.screen_size[0], self.screen_size[1])
        ulid = str(ULID())
        filename = f"{ulid}_{immich_uuid}.bmp"
        path = os.path.join(self.base_dir, filename)
        if image.mode != "RGB":
            image = image.convert("RGB")
        image = prepare_image_for_eink(image)
        image.save(path, "BMP")
        file_size = os.stat(path).st_size
        removed_image = self.make_space_for(file_size)
        self.cache_files[immich_uuid] = ImmichCache.Entry(ulid, immich_uuid, path)
        self.cache_size += file_size
        return removed_image

    def make_space_for(self, size: int) -> bool:
        removed_image = False
        while (self.max_size - self.cache_size) < size:
            self.pop()
            removed_image = True
        return removed_image

    def is_empty(self) -> bool:
        return len(self.cache_files) == 0

    def pop(self) -> Image.Image:
        _, entry = self.cache_files.popitem(False)
        image = Image.open(entry.path)
        self.cleanup(entry)
        return image

    def cleanup(self, entry: "ImmichCache.Entry"):
        self.cache_size -= os.stat(entry.path).st_size
        os.remove(entry.path)

    def delete(self, immich_uuid: str, exists_ok=False):
        entry = self.cache_files.get(immich_uuid)
        if entry is None and not exists_ok:
            raise KeyError(f"Could not find {immich_uuid} in cache.")
        self.cleanup(entry)
        del self.cache_files[immich_uuid]

    def clear(self):
        for name in os.listdir(self.base_dir):
            path = os.path.join(self.base_dir, name)
            if os.path.isfile(path) or os.path.islink(path):
                os.remove(path)
            elif os.path.isdir(path):
                shutil.rmtree(path)
        self.cache_files = OrderedDict()
        self.cache_size = 0

    def refresh(self):
        self.cache_files = OrderedDict()
        entries = []
        for root, _, filenames in os.walk(self.base_dir, topdown=False):
            for filename in filenames:
                path = os.path.join(root, filename)
                filename_match = filename_regex.match(filename)
                if filename_match:
                    ulid = filename_match.group(1)
                    immich_uuid = filename_match.group(2)
                    self.cache_size += os.stat(path).st_size
                    entries.append(ImmichCache.Entry(ulid, immich_uuid, path))
                else:
                    os.remove(path)
        remove_empty_folders(self.base_dir)
        for entry in sorted(entries, key=lambda entry: entry.ulid):
            self.cache_files[entry.immich_uuid] = entry

    @dataclass
    class Entry:
        ulid: str
        immich_uuid: str
        path: str


cache = ImmichCache(
    ImmichClient(settings.immich_base_url, settings.immich_api_key),
    os.path.join(settings.data_dir, "immich_cache"),
    settings.immich_cache_max_size,
    (settings.screen_width, settings.screen_height),
)


def get_immich_cache():
    yield cache


ImmichCacheDep = Annotated[ImmichCache, Depends(get_immich_cache)]
