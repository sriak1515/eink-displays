from datetime import datetime
from io import BytesIO
import time

from fastapi import APIRouter, HTTPException, Response

from app.services.image_processing_service import prepare_image_for_eink
from app.services.immich_service import run_fill_cache
from app.utils.immich_cache import ImmichCacheDep

router = APIRouter(prefix="/immich", tags=["immich"])


@router.get(
    "/image",
    responses={200: {"content": {"image/bmp": {}}}},
    response_class=Response,
)
def get_immich_image(*, cache: ImmichCacheDep):
    run_fill_cache(cache)
    if cache.is_empty():
        time.sleep(2)
        if cache.is_empty():
            raise HTTPException(503, "Cache is empty and Immich is unavailable")
    image = cache.pop()
    buffer = BytesIO()
    image.save(buffer, "BMP")
    buffer.seek(0)
    return Response(content=buffer.getvalue(), media_type="image/bmp")
