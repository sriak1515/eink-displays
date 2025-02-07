import os
from dataclasses import dataclass
from typing import List, Tuple
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


@dataclass
class Bounds:
    left: int
    top: int
    width: int
    height: int

    def grow_percentage(self, percentage):
        new_bounds = Bounds(self.left, self.top, self.width, self.height)
        new_bounds.width += max(round(percentage / 100 * self.width), 1)
        new_bounds.height += max(round(percentage / 100 * self.height), 1)
        return new_bounds


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


def get_text_bounds(text: str, font):
    left, top, right, bottom = font.getbbox(text.encode("utf-8"), language="fr")
    return Bounds(max(left, 0), max(top, 0), right - left, bottom - top)


def compute_time(entry: TimeTableEntry) -> str:
    return entry.time + (f"+{int(entry.delay)}" if entry.delay > 1 else "")


def ellipsis_until_fit(text: str, font, width):
    ellipsis = text
    while (get_text_bounds(text, font).width > width) and len(ellipsis) > 0:
        ellipsis = ellipsis[:-1]
        # text = ellipsis + "..."
        text = ellipsis + "…"
    return text


def compute_timetable_x_offsets(
    entries: List[TimeTableEntry], width: int, font, padding: int = 5
) -> Tuple[int, int, int]:
    max_line_width = max(
        map(lambda entry: get_text_bounds(entry.line, font).width, entries)
    )
    max_time_width = max(
        map(lambda entry: get_text_bounds(compute_time(entry), font).width, entries)
    )
    line_x_offset = padding
    direction_x_offset = max_line_width + 2 * padding + padding
    time_x_offset = width - max_time_width - padding
    return line_x_offset, direction_x_offset, time_x_offset


def compute_timetable_max_height(entries: List[TimeTableEntry], font) -> int:
    max_line_height = max(
        map(lambda entry: get_text_bounds(entry.line, font).height, entries)
    )
    max_direction_height = max(
        map(lambda entry: get_text_bounds(entry.direction, font).height, entries)
    )
    max_time_height = max(
        map(lambda entry: get_text_bounds(compute_time(entry), font).height, entries)
    )
    return max(max_line_height, max_direction_height, max_time_height)


def draw_timetables(
    canvas: Image,
    bounds: Bounds,
    timetables: List[TimeTable],
    font_path="./fonts/OpenSans-Regular.ttf",
    font_header_size=22,
    font_entries_size=18,
    vertical_padding=4,
):
    drawer = ImageDraw.Draw(canvas)
    width = bounds.width
    height = bounds.height
    font_header = ImageFont.truetype(font_path, font_header_size, encoding="unic")
    font_entries = ImageFont.truetype(font_path, font_entries_size, encoding="unic")
    all_entries = [
        y for x in map(lambda timetable: timetable.entries, timetables) for y in x
    ]

    x_offsets = compute_timetable_x_offsets(
        all_entries,
        width,
        font_entries,
    )
    max_height = compute_timetable_max_height(
        all_entries,
        font_entries,
    )
    height_with_padding = height - len(timetables) * vertical_padding * 2
    height_per_timetable = height_with_padding // len(timetables)
    leftover_height = height_with_padding % len(timetables)
    current_y = bounds.top
    for timetable in timetables:
        bounds = Bounds(
            bounds.left, current_y + vertical_padding * 2, width, height_per_timetable
        )
        if leftover_height > 0:
            bounds.height += 1
            leftover_height -= 1
        draw_timetable(
            drawer, bounds, timetable, x_offsets, max_height, font_header, font_entries
        )
        current_y += bounds.height


def draw_timetable(
    drawer: ImageDraw,
    bounds: Bounds,
    timetable: TimeTable,
    x_offsets: Tuple[int, int, int],
    max_height: int,
    font_header,
    font_entries,
):
    header_y_offset = 16
    header_bounds = draw_timetable_header(
        drawer, bounds.grow_percentage(5), timetable.stop_name, font_header
    )
    remaining_height = bounds.height - header_bounds.height + header_y_offset + len(timetable.entries)
    height_per_entry = min(remaining_height // len(timetable.entries), max_height)
    leftover_height = (
        remaining_height % len(timetable.entries)
        if height_per_entry != max_height
        else 0
    )
    current_y = bounds.top + header_bounds.height + header_y_offset
    for timetable_entry in timetable.entries:
        bounds = Bounds(bounds.left, current_y, bounds.width, height_per_entry + 1)
        if leftover_height > 0:
            bounds.height += 1
            leftover_height -= 1
        current_y += bounds.height
        draw_time_table_entry(drawer, bounds, timetable_entry, x_offsets, font_entries)


def draw_timetable_header(drawer: ImageDraw, bounds, stop_name: str, font) -> Bounds:

    header_text_bounds = get_text_bounds(stop_name, font)
    header_bounds = header_text_bounds.grow_percentage(5)
    header_x_offset = bounds.left + max(
        round((bounds.width - header_text_bounds.width) / 2), 0
    )
    header_y_offset = bounds.top + max(
        round((header_bounds.height - header_text_bounds.height) / 2), 0
    )

    drawer.text((header_x_offset, header_y_offset), stop_name, "black", font=font)
    return header_bounds


def draw_time_table_entry(
    drawer: ImageDraw,
    bounds: Bounds,
    entry: TimeTableEntry,
    x_offsets: Tuple[int, int, int],
    font,
):
    line_x_offset, direction_x_offset, time_x_offset = x_offsets

    time_text = compute_time(entry)
    time_text_bounds = get_text_bounds(time_text, font)

    direction_text = ellipsis_until_fit(
        entry.direction, font, time_x_offset - direction_x_offset
    )

    y_offset = bounds.top + bounds.height // 2

    drawer.text(
        (bounds.left + line_x_offset, y_offset),
        entry.line,
        "black",
        font=font,
        anchor="lm"
    )
    drawer.text(
        (bounds.left + direction_x_offset, y_offset),
        direction_text,
        "black",
        font=font,
        anchor="lm"
    )
    drawer.text(
        (bounds.left + time_x_offset, y_offset),
        time_text,
        "black",
        font=font,
        anchor="lm"
    )
    if entry.is_canceled:
        drawer.line(
            [
                (line_x_offset, y_offset),
                (time_x_offset + time_text_bounds.width, y_offset),
            ],
            "black",
            1,
        )
