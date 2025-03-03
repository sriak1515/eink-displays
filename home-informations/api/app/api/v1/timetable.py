from fastapi import APIRouter, Response
from fastapi.responses import FileResponse

from app.services.timetable_service import (
    fetch_previous_timetable,
    generate_timetable,
)

router = APIRouter(prefix="/timetable", tags=["timetable"])


@router.get(
    "",
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
    return FileResponse(
        generate_timetable(
            width, height, font_header_size, font_entries_size, n, rotation, key
        ),
        media_type="image/bmp",
    )


@router.get(
    "/previous",
    responses={200: {"content": {"image/png": {}}}},
    response_class=Response,
)
def get_previous_timetable(
    width: int = 220, height: int = 480, rotation: int = 0, key="timetable"
):
    return FileResponse(
        fetch_previous_timetable(width, height, rotation, key), media_type="image/bmp"
    )
