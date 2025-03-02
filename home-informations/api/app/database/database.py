from typing import Annotated

from app.config import settings
from fastapi import Depends
from sqlmodel import Session, SQLModel, create_engine

SQLALCHEMY_DATABASE_URL = f"sqlite:///{settings.sqlite_db_path}"

engine = create_engine(
    SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False}
)


def create_db_and_tables():
    SQLModel.metadata.create_all(engine)


def get_session():
    with Session(engine) as session:
        yield session


SessionDep = Annotated[Session, Depends(get_session)]
