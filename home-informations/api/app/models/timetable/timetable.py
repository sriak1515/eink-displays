from dataclasses import dataclass
from typing import List


@dataclass
class TimeTableEntry:
    line: str
    direction: str
    time: str
    delay: int
    is_canceled: bool


@dataclass
class TimeTable:
    stop_name: str
    entries: List[TimeTableEntry]
