import logging
import time

from fastapi import APIRouter, BackgroundTasks, HTTPException, Response
from fastapi.responses import FileResponse

from app.services.immich_service import run_fill_cache
from app.utils.immich_cache import ImmichCache, ImmichCacheDep

router = APIRouter(prefix="/immich", tags=["immich"])

logger = logging.getLogger(__name__)

def remove_from_cache(entry: ImmichCache.Entry, cache: ImmichCache):
    cache.cleanup(entry)

@router.get(
    "/image",
    responses={200: {"content": {"image/bmp": {}}}},
    response_class=Response,
)
def get_immich_image(*, cache: ImmichCacheDep, background_tasks: BackgroundTasks):
    if cache.size() < 5:
        run_fill_cache(cache)
        time.sleep(2)
        if cache.is_empty():
            raise HTTPException(503, "Cache is empty and Immich is unavailable")
    entry = cache.pop()
    background_tasks.add_task(remove_from_cache, entry=entry, cache=cache)
    return FileResponse(entry.path, media_type="image/bmp")
