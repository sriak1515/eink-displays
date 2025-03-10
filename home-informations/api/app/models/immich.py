from datetime import datetime
from io import BytesIO
import logging

import requests
from PIL import Image
from pillow_heif import register_heif_opener

logger = logging.getLogger(__name__)

register_heif_opener()


class ImmichClient:
    def __init__(self, base_url: str, api_key: str):
        self.base_url = base_url
        self.auth_headers = {"x-api-key": api_key}

    def is_available(self):
        r = requests.get(self.base_url + "/api/server/ping")
        if r.status_code != 200 or r.json()["res"] != "pong":
            return False
        else:
            return True

    def search(
        self,
        page: int,
        start_date: datetime = datetime.fromisoformat("2023-12-15T00:00:00.000Z"),
    ):
        r = requests.post(
            self.base_url + "/api/search/metadata",
            json={
                "page": page,
                "withExif": True,
                "isVisible": True,
                "type": "IMAGE",
                "takenAfter": start_date.isoformat(sep="T", timespec="milliseconds"),
            },
            headers=self.auth_headers,
        )
        r.raise_for_status()
        return r.json()

    def get_image_infos(self, id: str):
        r = requests.get(self.base_url + f"/api/assets/{id}", headers=self.auth_headers)
        r.raise_for_status()
        return r.json()

    def get_image(self, id: str) -> Image.Image:
        image_infos = self.get_image_infos(id)
        r = requests.get(
            self.base_url + f"/api/assets/{id}/original", headers=self.auth_headers
        )
        r.raise_for_status()
        image = Image.open(BytesIO(r.content))
        if image_infos["exifInfo"] and image_infos["exifInfo"]["orientation"] == "6":
            image = image.rotate(-90, expand=True)
        return image
