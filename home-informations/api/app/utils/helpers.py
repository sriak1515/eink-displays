import enum
import io
import logging
import os
import shutil
import zipfile
from datetime import datetime, timedelta

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


# https://gist.github.com/beugley/ccd69945346759eb6142272a6d69b4e0?permalink_comment_id=4658323#gistcomment-4658323
def human_readable_to_bytes(size: str) -> int:
    """Given a human-readable byte string (e.g. 2G, 30M, 20K),
    return the number of bytes.  Will raise an exception if the argument has
    unexpected form.
    """
    # Try to parse the size as if the unit was coded on 1 char.
    try:
        numeric_size = float(size[:-1])
        unit = size[-1]
    except ValueError:
        try:
            # Try to parse the size as if the unit was coded on 2 chars.
            numeric_size = float(size[:-2])
            unit = size[-2:-1]
        except ValueError:
            raise ValueError("Can't convert %r to bytes" % size)

    unit = unit.upper()

    # Now we have a numeric value and a unit. Check the unit and convert to bytes.
    if unit == "G":
        bytes = numeric_size * 1073741824
    elif unit == "M":
        bytes = numeric_size * 1048576
    elif unit == "K":
        bytes = numeric_size * 1024
    else:
        bytes = numeric_size

    return int(bytes)

def remove_empty_folders(root: str):
    deleted = set()
    for current_dir, dirnames, filenames in os.walk(root, topdown=False):
        has_subdirs = False
        for dirname in dirnames:
            if os.path.join(current_dir, dirname) not in deleted:
                has_subdirs = True
                break
        if current_dir != root and not any(filenames) and not has_subdirs:
            os.rmdir(current_dir)
            deleted.add(current_dir)
    return deleted
