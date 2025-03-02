from datetime import datetime, timezone
from typing import TYPE_CHECKING, Optional
from typing_extensions import Annotated

from pydantic import AfterValidator, BeforeValidator, field_serializer
from sqlmodel import (
    TIMESTAMP,
    Field,
    Relationship,
    SQLModel,
)

from app.database.models.eink.status import Status

if TYPE_CHECKING:
    from .display import Display, DisplayPublic

class UpdateBase(SQLModel):
    message: str


class Update(UpdateBase, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    updated_datetime: Optional[datetime] = Field(
        default_factory=lambda: datetime.now(timezone.utc),
        nullable=False,
        sa_column_kwargs={
            "onupdate": lambda: datetime.now(timezone.utc),
        },
        sa_type=TIMESTAMP(timezone=True),
    )
    status: int = Field(default=Status.PASS.value, nullable=False)
    display: "Display" = Relationship(back_populates="updates")

    display_id: int = Field(foreign_key="display.id")

class UpdatePublic(UpdateBase):
    id: int
    updated_datetime: datetime
    status: Annotated[str, BeforeValidator(lambda status: Status.get_label(status))]

class UpdatePublicWithDisplay(UpdatePublic):
    display: Optional["DisplayPublic"] = None

class UpdateCreate(UpdateBase):
    status: Annotated[str, AfterValidator(lambda status: Status.validate_label(status))]    

    @field_serializer("status")
    def serialize_status(self, status: str, _info):
        return Status.get_value(status)

class UpdateUpdate(UpdateBase):
    display_id: Optional[int] = None
    message: Optional[str] = None
    status: Optional[Annotated[str, AfterValidator(lambda status: Status.validate_label(status))]] = None

    @field_serializer("status")
    def serialize_status(self, status: str, _info):
        return Status.get_value(status)
