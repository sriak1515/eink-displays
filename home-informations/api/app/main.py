import logging
from datetime import datetime, timedelta, timezone

from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers.interval import IntervalTrigger
from fastapi import FastAPI
from fastapi.concurrency import asynccontextmanager
from fastapi.middleware.cors import CORSMiddleware

from app.api.v1.display import router as display_router
from app.api.v1.gtfs import router as gtfs_router
from app.api.v1.immich import router as immich_router
from app.api.v1.timetable import router as timetable_router
from app.config import settings
from app.database.database import create_db_and_tables, get_session
from app.services.display_service import remove_all_updates_before
from app.services.immich_service import run_fill_cache
from app.utils.immich_cache import get_immich_cache

logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

scheduler = BackgroundScheduler()
trigger = IntervalTrigger(hours=settings.update_clear_interval_in_hours)

logger = logging.getLogger(__name__)

def cleanup_updates():
    session = get_session()
    remove_all_updates_before(
        session,
        datetime.now(timezone.utc)
        - timedelta(hours=settings.update_clear_interval_in_hours),
    )


def fill_immich_cache():
    logger.info("Filling immich cache")
    cache = get_immich_cache()
    run_fill_cache(cache)


scheduler.add_job(
    cleanup_updates,
    trigger,
)
scheduler.add_job(fill_immich_cache, IntervalTrigger(hours=1))
scheduler.start()


@asynccontextmanager
async def lifespan(app: FastAPI):
    create_db_and_tables()

    logging.info("startup: triggered")

    yield
    scheduler.shutdown()
    logging.info("shutdown: triggered")


app = FastAPI(lifespan=lifespan)

origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

api_v1_prefix = "/api/v1"
app.include_router(gtfs_router, prefix=api_v1_prefix)
app.include_router(timetable_router, prefix=api_v1_prefix)
app.include_router(display_router, prefix=api_v1_prefix)
app.include_router(immich_router, prefix=api_v1_prefix)
