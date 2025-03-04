from datetime import datetime
from io import BytesIO

from fastapi import APIRouter, Response

from app.services.image_processing_service import prepare_image_for_eink
from app.services.immich_service import get_random_image

router = APIRouter(prefix="/immich", tags=["immich"])


@router.get(
    "/random",
    responses={200: {"content": {"image/bmp": {}}}},
    response_class=Response,
)
def get_immich_random_image(
    *,
    portrait: bool = True,
    width: int = 480,
    height: int = 800,
    after: datetime = datetime.fromisoformat("2023-12-15T00:00:00.000Z"),
):
    image = get_random_image(portrait, after)
    image = prepare_image_for_eink(image, width, height)
    buffer = BytesIO()
    image.save(buffer, "BMP")
    return Response(content=buffer.getvalue(), media_type="image/bmp")
