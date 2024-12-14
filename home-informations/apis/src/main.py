from datetime import datetime
import io
import os
import re
import logging
import pathlib

from fastapi import FastAPI, Response
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from pydantic_settings import BaseSettings
from gtfs.config import local_stops_ids, local_stops_ids_merge
from gtfs.format import get_nth_next_departures
from gtfs.gtfs_dataset import GtfsDataset
from gtfs.gtfs_realtime import GtfsRealtime
from PIL import Image
from renderer.timetable import Bounds, TimeTable, TimeTableEntry, draw_timetables


class Settings(BaseSettings):
    data_dir: str = "./data"
    gtfs_api_key: str


settings = Settings()

logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

dataset = GtfsDataset(os.path.join(settings.data_dir, "gtfs"))

realtime = GtfsRealtime(
    os.path.join(settings.data_dir, "gtfs-rt"), settings.gtfs_api_key
)

timetable_cache_path = os.path.join(settings.data_dir, "timetable_cache")
if not os.path.exists(timetable_cache_path):
    os.makedirs(timetable_cache_path, exist_ok=True)

app = FastAPI()

origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


def map_group_row(row):
    return {
        "line": row["route_short_name"],
        "direction": row["trip_headsign"],
        "time": row["departure_timestamp"].strftime("%H:%M"),
        "delay": row["departure_delay"],
        "is_canceled": row["is_canceled"],
    }


@app.get("/gtfs/next")
def get_gtfs_next_departures(n: int = 4):
    next_depatures = get_nth_next_departures(local_stops_ids, dataset, realtime, n)
    return [
        {"stop": stop, "entries": entries}
        for stop, entries in next_depatures.groupby("stop_name")
        .apply(
            lambda group: group.sort_values("departure_timestamp").apply(
                map_group_row, axis=1
            )
        )
        .groupby("stop_name")
        .apply(list)
        .to_dict()
        .items()
    ]


@app.get(
    "/timetable",
    responses={200: {"content": {"image/png": {}}}},
    response_class=Response,
)
def get_timetable(
    width: int = 220,
    height: int = 480,
    font_header_size: int = 24,
    font_entries_size: int = 18,
    n: int = 4,
    rotation: int = 0,
    key="timetable",
):
    next_departures = get_nth_next_departures(local_stops_ids, dataset, realtime, n)
    timetables = [
        TimeTable(stop, [TimeTableEntry(**entry) for entry in entries])
        for stop, entries in next_departures.groupby("stop_name")
        .apply(
            lambda group: group.sort_values("departure_timestamp").apply(
                map_group_row, axis=1
            )
        )
        .groupby("stop_name")
        .apply(list)
        .to_dict()
        .items()
    ]
    timetables += [TimeTable("", [TimeTableEntry("", "", "", 0, False)])] * max(
        len(set(local_stops_ids_merge.values())) - len(timetables), 0
    )
    canvas = Image.new("P", (width, height), "white")
    font_path = (
        pathlib.Path(__file__)
        .parent.parent.joinpath("fonts")
        # .joinpath("scientifica").joinpath("scientifica.ttf")
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
    return FileResponse(image_path, media_type="image/bmp")


@app.get(
    "/previous_timetable",
    responses={200: {"content": {"image/png": {}}}},
    response_class=Response,
)
def get_previous_timetable(width: int, height: int, rotation: int = 0, key="timetable"):
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
        output = io.BytesIO()
        image.rotate(int(rotation), expand=True).save(output, format="BMP")
        output.seek(0)
        return Response(output.read(), media_type="image/bmp")

    if len(old_cache_files) > 1:
        for file_name, _ in old_cache_files[:-1]:
            print(f"Clearing {file_name} from image cache")
            os.remove(os.path.join(timetable_cache_path, file_name))

    return FileResponse(
        os.path.join(timetable_cache_path, old_cache_files[-1][0]),
        media_type="image/bmp",
    )
