from datetime import datetime
import io
import logging
import os
import shutil
import zipfile

import requests

logger = logging.getLogger(__name__)


def download_and_extract_zip(url, extract_to, overwrite=False):
    path_exists = os.path.exists(extract_to)
    if overwrite:
        if path_exists:
            shutil.rmtree(extract_to)
    else:
        if path_exists:
            logger.error(
                f"Error: the path {extract_to} already exists and overwrite is false."
            )
            return False
    response = requests.get(url)

    if response.status_code == 200:
        zip_file = io.BytesIO(response.content)

        with zipfile.ZipFile(zip_file, "r") as zip_ref:
            zip_ref.extractall(extract_to)
        return True
    else:
        return False


def create_folder_if_not_exists(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)


def format_if_datetime(value, strfmt="%H:%M"):
    return value.strftime(strfmt) if isinstance(value, datetime) else value


def get_now_timestamp():
    return int(datetime.now().timestamp())