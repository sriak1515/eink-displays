import re

table_name2dtypes = {
    "agency": {
        "agency_id": "varchar",
        "agency_name": "varchar",
        "agency_url": "varchar",
        "agency_timezone": "varchar",
        "agency_lang": "varchar",
        "agency_phone": "varchar",
    },
    "calendar": {
        "service_id": "varchar",
        "monday": "int",
        "tuesday": "int",
        "wednesday": "int",
        "thursday": "int",
        "friday": "int",
        "saturday": "int",
        "sunday": "int",
        "start_date": "int",
        "end_date": "int",
    },
    "calendar_dates": {
        "service_id": "varchar",
        "date": "int",
        "exception_type": "int",
    },
    "feed_info": {
        "feed_publisher_name": "varchar",
        "feed_publisher_url": "varchar",
        "feed_lang": "varchar",
        "feed_start_date": "varchar",
        "feed_end_date": "varchar",
        "feed_version": "varchar",
    },
    "routes": {
        "route_id": "varchar",
        "agency_id": "varchar",
        "route_short_name": "varchar",
        "route_long_name": "varchar",
        "route_desc": "varchar",
        "route_type": "varchar",
    },
    "stops": {
        "stop_id": "varchar",
        "stop_name": "varchar",
        "stop_lat": "float",
        "stop_lon": "float",
        "location_type": "varchar",
        "parent_station": "varchar",
    },
    "stop_times": {
        "trip_id": "varchar",
        "arrival_time": "varchar",
        "departure_time": "varchar",
        # "arrival_time": "time",
        # "departure_time": "time",
        "stop_id": "varchar",
        "stop_sequence": "int",
        "pickup_type": "varchar",
        "drop_off_type": "varchar",
    },
    "trips": {
        "route_id": "varchar",
        "service_id": "varchar",
        "trip_id": "varchar",
        "trip_headsign": "varchar",
        "trip_short_name": "varchar",
        "direction_id": "varchar",
        "block_id": "varchar",
    },
    "transfers": {
        "from_stop_id": "varchar",
        "to_stop_id": "varchar",
        "transfer_type": "varchar",
        "min_transfer_time": "varchar",
    },
}

dataset_url_regex = re.compile(r".*/.*\d{4}-\d{2}-\d{2}.zip")
dataset_extension = ".ddb"
dataset_name_regex = re.compile(r"\d{8}" + dataset_extension)
