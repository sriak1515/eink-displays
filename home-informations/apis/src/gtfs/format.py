from datetime import datetime, timedelta

from gtfs.gtfs_dataset import GtfsDataset


def get_nth_next_departures(stop_ids, dataset: GtfsDataset, n=5):
    now = datetime.now()
    todays_stop_times = dataset.get_date_stop_times(now, stop_ids)
    return (
        todays_stop_times.loc[todays_stop_times["departure_timestamp"] >= now]
        .groupby("stop_name")
        .head(n)
    )


def get_next_departures(stop_ids, dataset: GtfsDataset, max_time_diff_seconds=15 * 60):
    now = datetime.now()
    todays_stop_times = dataset.get_date_stop_times(now, stop_ids)
    return todays_stop_times.loc[
        (todays_stop_times["departure_timestamp"] >= now)
        & (
            todays_stop_times["departure_timestamp"]
            <= now + timedelta(0, max_time_diff_seconds)
        )
    ]
