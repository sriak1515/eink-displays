import logging
import os
import re
from datetime import datetime, timedelta

import requests
from google.transit import gtfs_realtime_pb2
from utils import create_folder_if_not_exists

logger = logging.getLogger(__name__)

gtfs_rt_feeder_regex = re.compile(r"\d{12}")

CANCELED_RELATIONSHIP_TERMS = ["CANCELED", "SKIPPED"]


class GtfsRealtime:
    def __init__(self, save_dir: str, api_key: str) -> None:
        self.save_dir = save_dir
        self.auth_headers = {"Authorization": api_key}
        self._feed = None

    @property
    def feed(self):
        if not self.get_latest_gtfs_rt():
            logger.error("Could not get latest RT data")
            return None
        return self._feed

    def update_delays(
        self, stop_times, minimum_arrival_delay=0, minimum_departure_delay=0
    ) -> bool:
        # reset delays
        stop_times["arrival_delay"] = 0
        stop_times["departure_delay"] = 0
        stop_times["is_canceled"] = False
        stop_times["adjusted_departure_timestamp"] = stop_times["departure_timestamp"]
        stop_times["adjusted_arrival_timestamp"] = stop_times["arrival_timestamp"]
        if self.feed is None:
            return False

        trip_ids = set(stop_times["trip_id"].unique().tolist())
        stops_ids = set(stop_times["stop_id"].unique().tolist())

        stop_times.set_index(["trip_id", "stop_id", "stop_sequence"], inplace=True)
        stop_times.sort_index(inplace=True)

        for entity in self.feed.entity:
            if (
                entity.HasField("trip_update")
                and entity.trip_update.trip.trip_id in trip_ids
            ):
                trip_id = entity.trip_update.trip.trip_id
                for stop_update in entity.trip_update.stop_time_update:
                    if stop_update.stop_id in stops_ids:
                        stop_id = stop_update.stop_id
                        stop_sequence = stop_update.stop_sequence
                        arrival_delay = (
                            stop_update.arrival.delay
                            if stop_update.HasField("arrival")
                            and stop_update.arrival.HasField("delay")
                            else 0
                        )
                        departure_delay = (
                            stop_update.departure.delay
                            if stop_update.HasField("departure")
                            and stop_update.departure.HasField("delay")
                            else 0
                        )
                        if (
                            arrival_delay >= minimum_arrival_delay
                            and departure_delay >= minimum_departure_delay
                        ):
                            arrival_delay = round(arrival_delay / 60)
                            departure_delay = round(departure_delay / 60)
                            stop_times.loc[
                                (trip_id, stop_id, stop_sequence), "arrival_delay"
                            ] = arrival_delay
                            stop_times.loc[
                                (trip_id, stop_id, stop_sequence), "departure_delay"
                            ] = departure_delay
                            stop_times.loc[
                                (trip_id, stop_id, stop_sequence),
                                "adjusted_arrival_timestamp",
                            ] += timedelta(minutes=arrival_delay)
                            stop_times.loc[
                                (trip_id, stop_id, stop_sequence),
                                "adjusted_departure_timestamp",
                            ] += timedelta(minutes=departure_delay)
                        stop_times.loc[
                            (trip_id, stop_id, stop_sequence), "is_canceled"
                        ] = (
                            stop_update.HasField("schedule_relationship")
                            and stop_update.schedule_relationship
                            in CANCELED_RELATIONSHIP_TERMS
                        )
        stop_times.reset_index(inplace=True)
        stop_times.sort_values(by="adjusted_departure_timestamp", inplace=True)
        return True

    def get_latest_gtfs_rt(self, cleanup_old=True):
        create_folder_if_not_exists(self.save_dir)
        feed = gtfs_realtime_pb2.FeedMessage()
        latest_local_gtfs_rt = self.get_latest_local_gtfs_rt()

        now_timestamp = datetime.now().strftime("%Y%m%d%H%M")

        # if we are less than max_difference in seconds from the latest timestamp, do not fetch
        if now_timestamp <= latest_local_gtfs_rt:
            logger.info(
                f"Current gtfs rt data {latest_local_gtfs_rt} is withing the same minute, using it."
            )
            now_timestamp = latest_local_gtfs_rt
            with open(
                os.path.join(self.save_dir, latest_local_gtfs_rt), "rb"
            ) as infile:
                feed.ParseFromString(infile.read())
        else:
            r = requests.get(
                "https://api.opentransportdata.swiss/gtfsrt2020",
                headers=self.auth_headers,
            )
            if bytes("error", "utf-8") in r.content:
                logger.error(
                    "Please check API key. Server returned {}".format(
                        r.content.decode()
                    )
                )
                return False
            try:
                feed.ParseFromString(r.content)
            except:
                return False

            with open(os.path.join(self.save_dir, now_timestamp), "wb") as outfile:
                outfile.write(r.content)

        if cleanup_old:
            for file in set(self.get_local_gtfs_rt()).difference([now_timestamp]):
                logger.info(f"Removing old data {file}")
                os.remove(os.path.join(self.save_dir, file))

        self._feed = feed
        return True

    def get_local_gtfs_rt(self):
        local_gtfs_rt = []
        files = os.listdir(self.save_dir)
        for file in files:
            basename = os.path.basename(file)
            if gtfs_rt_feeder_regex.match(basename):
                local_gtfs_rt.append(basename)
        return local_gtfs_rt

    def get_latest_local_gtfs_rt(self):
        local_gtfs_rt = self.get_local_gtfs_rt()
        return sorted(local_gtfs_rt)[-1] if len(local_gtfs_rt) > 0 else "197001010000"
