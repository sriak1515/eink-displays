from fastapi import APIRouter

from app.services.gtfs_service import fetch_next_departures

router = APIRouter(prefix="/gtfs", tags=["gtfs"])


@router.get("/next")
def get_gtfs_next_departures(n: int = 4):
    return fetch_next_departures(n)
