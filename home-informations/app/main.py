import logging

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.api.v1.gtfs import router as gtfs_router
from app.api.v1.timetable import router as timetable_router


logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

app = FastAPI()

origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(gtfs_router)
app.include_router(timetable_router)
