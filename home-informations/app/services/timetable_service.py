import os
import re
from datetime import datetime
from pathlib import Path

from PIL import Image

from app.config import settings
from app.models.gtfs.config import local_stops_ids_merge
from app.renderer.timetable import Bounds, TimeTable, TimeTableEntry, draw_timetables
from app.services.gtfs_service import fetch_next_departures

timetable_cache_path = os.path.join(settings.data_dir, "timetable_cache")
if not os.path.exists(timetable_cache_path):
    os.makedirs(timetable_cache_path, exist_ok=True)


def generate_timetable(
    width: int,
    height: int,
    font_header_size: int,
    font_entries_size: int,
    n: int,
    rotation: int,
    key: str,
) -> str:
    timetables = [
        TimeTable(
            departures["stop"],
            [TimeTableEntry(**entry) for entry in departures["entries"]],
        )
        for departures in fetch_next_departures(n)
    ]
    timetables += [TimeTable("", [TimeTableEntry("", "", "", 0, False)])] * max(
        len(set(local_stops_ids_merge.values())) - len(timetables), 0
    )
    canvas = Image.new("P", (width, height), "white")
    font_path = (
        Path(__file__)
        .parent.parent.joinpath("fonts")
        .joinpath("OpenSans-Regular.ttf")
        .resolve()
    )
    draw_timetables(
        canvas,
        Bounds(0, 0, width, height),
        timetables,
        font_path=font_path,
        font_header_size=font_header_size,
        font_entries_size=font_entries_size,
    )
    image_name = datetime.now().strftime(f"{key}_%Y%m%d%H%M%S%f.bmp")
    image_path = os.path.join(timetable_cache_path, image_name)
    canvas.rotate(rotation, expand=True).save(image_path, format="BMP")
    return image_path


def fetch_previous_timetable(width: int, height: int, rotation: int, key: str) -> str:
    now = datetime.now()
    cache_file_name_pattern = re.compile("^" + key + r"_\d{20}.bmp$")
    cache_files = [os.path.basename(x) for x in os.listdir(timetable_cache_path)]
    cache_files = [
        (
            file_name,
            datetime.strptime(file_name, f"{key}_%Y%m%d%H%M%S%f.bmp"),
        )
        for file_name in cache_files
        if cache_file_name_pattern.match(file_name) is not None
    ]
    old_cache_files = sorted(
        [(file_name, time) for file_name, time in cache_files if time < now],
        key=lambda x: x[1],
    )
    if len(old_cache_files) == 0:
        image = Image.new("P", (int(width), int(height)), "white")
        image_path = os.path.join(
            timetable_cache_path, f"{key}_{now.strftime('%Y%m%d%H%M%S%f')}.bmp"
        )
        image.rotate(int(rotation), expand=True).save(image_path, format="BMP")
        return image_path

    if len(old_cache_files) > 1:
        for file_name, _ in old_cache_files[:-1]:
            print(f"Clearing {file_name} from image cache")
            os.remove(os.path.join(timetable_cache_path, file_name))

    return os.path.join(timetable_cache_path, old_cache_files[-1][0])
