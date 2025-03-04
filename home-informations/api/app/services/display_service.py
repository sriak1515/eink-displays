from datetime import datetime
from typing import List

from app.database.models.eink.display import Display, DisplayCreate, DisplayUpdate
from app.database.models.eink.update import Update, UpdateCreate
from sqlmodel import Session, delete, select


def get_displays(session: Session, offset: int = 0, limit: int = 100) -> List[Display]:
    return session.exec(select(Display).offset(offset).limit(limit)).all()


def get_display_by_id(session: Session, display_id: int) -> Display | None:
    return session.get(Display, display_id)


def create_display(session: Session, display: DisplayCreate) -> Display:
    display_db = Display.model_validate(display)
    session.add(display_db)
    session.commit()
    session.refresh(display_db)
    return display_db


def update_display(
    session: Session, display_id: int, display: DisplayUpdate
) -> Display:
    display_db = session.get(Display, display_id)
    if display_db:
        display_data = display.model_dump(exclude_unset=True)
        display_db.sqlmodel_update(display_data)
        session.add(display_db)
        session.commit()
        session.refresh(display_db)
    return display_db


def delete_display(session: Session, display_id: int) -> bool:
    display_db = session.get(Display, display_id)
    if display_db:
        session.delete(display_db)
        session.commit()
        return True
    return False


def get_updates_by_display_id(
    session: Session,
    display_id: int,
    offset: int = 0,
    limit: int = 100,
    after: datetime = datetime.min,
    before: datetime = datetime.max,
) -> List[Update]:
    return session.exec(
        select(Update)
        .where(Update.display_id == display_id)
        .where(Update.updated_datetime >= after)
        .where(Update.updated_datetime < before)
        .order_by(Update.updated_datetime.desc())
        .offset(offset)
        .limit(limit)
    ).all()


def create_update(session: Session, display_id: int, update: UpdateCreate) -> Update:
    update_data = update.model_dump(exclude_unset=True)
    update_data["display_id"] = display_id
    update_db = Update(**update_data)
    session.add(update_db)
    session.commit()
    session.refresh(update_db)
    return update_db


def remove_updates_before(session: Session, display_id: int, before: datetime) -> None:
    session.exec(
        delete(Update)
        .where(Update.display_id == display_id)
        .where(Update.updated_datetime < before)
    )
    session.commit()

def remove_all_updates_before(session: Session, before: datetime) -> None:
    session.exec(delete(Update).where(Update.updated_datetime < before))
    session.commit()
