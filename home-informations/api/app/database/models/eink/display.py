from typing import TYPE_CHECKING, List, Optional
from sqlmodel import Field, Relationship, SQLModel

if TYPE_CHECKING:
    from .update import Update, UpdatePublic



class DisplayBase(SQLModel):
    name: str = Field(index=True)
    url: str
    previous_url: Optional[str] = None
    refresh_frequency: int = 60
    full_refresh_frequency: Optional[int] = None


class Display(DisplayBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    updates: List["Update"] = Relationship(back_populates="display", cascade_delete=True)

class DisplayPublic(DisplayBase):
    id: int

class DisplayPublicWithUpdates(DisplayPublic):
    updates: List["UpdatePublic"] = []

class DisplayCreate(DisplayBase):
    pass

class DisplayUpdate(DisplayBase):
    name: Optional[str] = None
    url: Optional[str] = None
    refresh_frequency: Optional[int] = None
