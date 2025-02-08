from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    data_dir: str = "./data"
    gtfs_api_key: str

settings = Settings()
