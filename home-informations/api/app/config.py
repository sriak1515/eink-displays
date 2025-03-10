from datetime import datetime
from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    data_dir: str = "/data"
    gtfs_api_key: str
    screen_width: int = 480
    screen_height: int = 800
    immich_cache_cuttof_date: datetime = datetime.fromisoformat("2023-12-15T00:00:00.000Z")
    immich_base_url: str
    immich_api_key: str
    immich_cache_max_size: str = "1GB"
    sqlite_db_path: str = "/data/db.sqlite"
    update_clear_interval_in_hours: int = 6

settings = Settings()
