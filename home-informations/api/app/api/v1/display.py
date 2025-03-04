from datetime import date, datetime
from typing import List

from fastapi import APIRouter, HTTPException, Query

from app.database.database import SessionDep
from app.database.models.eink.display import DisplayCreate, DisplayPublic, DisplayUpdate
from app.database.models.eink.update import UpdateCreate, UpdatePublic
from app.services.display_service import (
    create_display,
    create_update,
    delete_display,
    get_display_by_id,
    get_displays,
    get_updates_by_display_id,
    remove_updates_before,
    update_display,
)

router = APIRouter(prefix="/eink/display", tags=["display"])


@router.get("", response_model=List[DisplayPublic])
def get_displays_endpoint(
    *, offset: int = 0, limit: int = Query(default=100, le=100), session: SessionDep
):
    return get_displays(session, offset, limit)


@router.get("/{display_id}", response_model=DisplayPublic)
def get_display_endpoint(display_id: int, session: SessionDep):
    display_db = get_display_by_id(session, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    return display_db


@router.post("", response_model=DisplayPublic)
def create_display_endpoint(display: DisplayCreate, session: SessionDep):
    return create_display(session, display)


@router.put("/{display_id}", response_model=DisplayPublic)
def update_display_endpoint(
    display_id: int, display: DisplayUpdate, session: SessionDep
):
    display_db = update_display(session, display_id, display)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    return display_db


@router.delete("/{display_id}")
def delete_display_endpoint(display_id: int, session: SessionDep):
    if not delete_display(session, display_id):
        raise HTTPException(404, f"Display with id {display_id} not found.")


@router.get("/{display_id}/update", response_model=List[UpdatePublic])
def get_update_endpoint(
    *,
    display_id: int,
    offset: int = 0,
    limit: int = Query(default=100, le=100),
    after: datetime = date.min,
    before: datetime = date.max,
    session: SessionDep,
):
    return get_updates_by_display_id(session, display_id, offset, limit, after, before)


@router.post("/{display_id}/update", response_model=UpdatePublic)
def create_update_endpoint(display_id: int, update: UpdateCreate, session: SessionDep):
    display_db = get_display_by_id(session, display_id)
    if not display_db:
        raise HTTPException(404, f"Display with id {display_id} not found.")
    return create_update(session, display_id, update)


@router.delete("/{display_id}/update")
def remove_updates_endpoint(*, display_id: int, before: datetime, session: SessionDep):
    remove_updates_before(session, display_id, before)
