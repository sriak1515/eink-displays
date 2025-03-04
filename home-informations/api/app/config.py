from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    data_dir: str = "/data"
    gtfs_api_key: str
    immich_api_key: str = ""
    sqlite_db_path: str = "/data/db.sqlite"
    update_clear_interval_in_hours: int = 6

settings = Settings()
