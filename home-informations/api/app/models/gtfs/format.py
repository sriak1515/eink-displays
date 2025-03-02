from datetime import datetime, timedelta

from app.models.gtfs.gtfs_dataset import GtfsDataset
from app.models.gtfs.gtfs_realtime import GtfsRealtime


def get_nth_next_departures(
    stop_ids, dataset: GtfsDataset, realtime: GtfsRealtime, n=5
):
    now = datetime.now().replace(second=0, microsecond=0)
    stop_times = dataset.get_time_window_stop_times(stop_ids)
    realtime.update_delays(stop_times)
    return (
        stop_times.loc[stop_times["adjusted_departure_timestamp"] >= now]
        .groupby("stop_name")
        .head(n)
    )


def get_next_departures(
    stop_ids,
    dataset: GtfsDataset,
    realtime: GtfsRealtime,
    max_time_diff_seconds=15 * 60,
):
    now = datetime.now().replace(second=0, microsecond=0)
    stop_times = dataset.get_time_window_stop_times(stop_ids)
    realtime.update_delays(stop_times)
    return stop_times.loc[
        (stop_times["adjusted_departure_timestamp"] >= now)
        & (
            stop_times["adjusted_departure_timestamp"]
            <= now + timedelta(0, max_time_diff_seconds)
        )
    ]
