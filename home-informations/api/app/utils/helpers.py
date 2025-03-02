from datetime import datetime, timedelta
import enum
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


def get_today():
    return datetime.today().replace(hour=12, minute=0, second=0, microsecond=0)


def get_yesterday():
    return get_today() - timedelta(days=1)


def get_tomorrow():
    return get_today() + timedelta(days=1)

def map_group_row(row):
    return {
        "line": row["route_short_name"],
        "direction": row["trip_headsign"],
        "time": row["departure_timestamp"].strftime("%H:%M"),
        "delay": row["departure_delay"],
        "is_canceled": row["is_canceled"],
    }

def enum_values(enum_class: type[enum.Enum]) -> list:
    """Get values for enum."""
    return [status.value for status in enum_class]