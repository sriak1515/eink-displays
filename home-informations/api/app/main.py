import logging

from fastapi import FastAPI
from fastapi.concurrency import asynccontextmanager
from fastapi.middleware.cors import CORSMiddleware

from app.database.database import create_db_and_tables
from app.api.v1.gtfs import router as gtfs_router
from app.api.v1.timetable import router as timetable_router
from app.api.v1.eink import eink_routers


logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

@asynccontextmanager
async def lifespan(app: FastAPI):
    create_db_and_tables()

    logging.info("startup: triggered")

    yield

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
for router in eink_routers:
    app.include_router(router, prefix=api_v1_prefix + "/eink")
