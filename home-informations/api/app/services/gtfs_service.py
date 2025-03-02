import os

from app.config import settings
from app.models.gtfs.config import local_stops_ids
from app.models.gtfs.format import get_nth_next_departures
from app.models.gtfs.gtfs_dataset import GtfsDataset
from app.models.gtfs.gtfs_realtime import GtfsRealtime
from app.utils.helpers import map_group_row

dataset = GtfsDataset(os.path.join(settings.data_dir, "gtfs"))
realtime = GtfsRealtime(
    os.path.join(settings.data_dir, "gtfs-rt"), settings.gtfs_api_key
)


def fetch_next_departures(n: int = 4):
    next_departures = get_nth_next_departures(local_stops_ids, dataset, realtime, n)
    return [
        {"stop": stop, "entries": entries}
        for stop, entries in next_departures.groupby("stop_name")
        .apply(
            lambda group: group.sort_values("departure_timestamp").apply(
                map_group_row, axis=1
            )
        )
        .groupby("stop_name")
        .apply(list)
        .to_dict()
        .items()
    ]
