from datetime import date, datetime
from typing import List

from fastapi import APIRouter, HTTPException, Query
from sqlmodel import delete, select

from app.database.database import SessionDep
from app.database.models.eink.display import (
    Display,
    DisplayCreate,
    DisplayPublic,
    DisplayUpdate,
)
from app.database.models.eink.update import Update, UpdateCreate, UpdatePublic

router = APIRouter(prefix="/display", tags=["eink"])


@router.get("", response_model=List[DisplayPublic])
def get_displays(
    *, offset: int = 0, limit: int = Query(default=100, le=100), session: SessionDep
):
    return session.exec(select(Display).offset(offset).limit(limit)).all()


@router.get("/{display_id}", response_model=DisplayPublic)
def get_display(display_id: int, session: SessionDep):
    display_db = session.get(Display, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    return display_db


@router.post("", response_model=DisplayPublic)
def create_display(display: DisplayCreate, session: SessionDep):
    display_db = Display.model_validate(display)
    session.add(display_db)
    session.commit()
    session.refresh(display_db)
    return display_db


@router.put("/{display_id}", response_model=DisplayPublic)
def update_display(display_id: int, display: DisplayUpdate, session: SessionDep):
    display_db = session.get(Display, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    display_data = display.model_dump(exclude_unset=True)
    display_db.sqlmodel_update(display_data)
    session.add(display_db)
    session.commit()
    session.refresh(display_db)
    return display_db


@router.delete("/{display_id}")
def delete_display(display_id: int, session: SessionDep):
    display_db = session.get(Display, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    session.delete(display_db)
    session.commit()


@router.get("/{display_id}/update", response_model=List[UpdatePublic])
def get_update(
    *,
    display_id: int,
    offset: int = 0,
    limit: int = Query(default=100, le=100),
    after: datetime = date.min,
    before: datetime = date.max,
    session: SessionDep,
):
    return session.exec(
        select(Update)
        .where(Update.display_id == display_id)
        .where(Update.updated_datetime >= after)
        .where(Update.updated_datetime < before)
        .order_by(Update.updated_datetime)
        .offset(offset)
        .limit(limit)
    ).all()


@router.post("/{display_id}/update", response_model=UpdatePublic)
def create_update(display_id: int, update: UpdateCreate, session: SessionDep):
    display_db = session.get(Display, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")

    update_data = update.model_dump(exclude_unset=True)
    update_data["display_id"] = display_id
    update_db = Update(**update_data)
    session.add(update_db)
    session.commit()
    session.refresh(update_db)
    return update_db


@router.delete("/{display_id}/update")
def remove_updates(display_id: int, before: datetime, session: SessionDep):
    session.exec(
        delete(Update)
        .where(Update.display_id == display_id)
        .where(Update.updated_datetime < before)
    )
    session.commit()
