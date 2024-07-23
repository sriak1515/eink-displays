from datetime import datetime
import logging
from gtfs.config import local_stops_ids
from gtfs.format import get_nth_next_departures, get_next_departures
from gtfs.gtfs_dataset import GtfsDataset
from gtfs.gtfs_realtime import GtfsRealtime

logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

dataset = GtfsDataset("./data/gtfs")

realtime = GtfsRealtime("./data/gtfs-rt")

today = datetime.now()

date = datetime(2024, 8, 1)

cully = [
    "8501124:0:1",  # Cully, voie 1?
    "8501124:0:2",  # Cully, voie 2?
    "8501124:0:3",  # Cully, voie 3?
]

#local_todays_stop_times = dataset.get_date_stop_times(date, cully)

#if not realtime.update_delays(local_todays_stop_times):
#    print("Could not update delays!")

#local_todays_stop_times.sort_values("departure_timestamp").to_excel("cully_20240801.xlsx", index=False)

get_nth_next_departures(local_stops_ids, dataset).sort_values(["stop_name", "departure_timestamp"]).to_excel("next_departures.xlsx", index=False)


# next_5_departures = get_nth_next_departure(local_stops_ids, local_stops_ids_merge)
# print(next_5_departures.display())
#
# print(map_stop_times_to_infos(next_5_departures).display())
