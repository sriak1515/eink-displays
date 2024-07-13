import re

table_name2dtypes = {
    "agency": {
        "agency_id": str,
        "agency_name": str,
        "agency_url": str,
        "agency_timezone": str,
        "agency_lang": str,
        "agency_phone": str,
    },
    "calendar": {
        "service_id": str,
        "monday": int,
        "tuesday": int,
        "wednesday": int,
        "thursday": int,
        "friday": int,
        "saturday": int,
        "sunday": int,
        "start_date": int,
        "end_date": int,
    },
    "calendar_dates": {
        "service_id": str,
        "date": int,
        "exception_type": int,
    },
    "feed_info": {
        "feed_publisher_name": str,
        "feed_publisher_url": str,
        "feed_lang": str,
        "feed_start_date": str,
        "feed_end_date": str,
        "feed_version": str,
    },
    "routes": {
        "route_id": str,
        "agency_id": str,
        "route_short_name": str,
        "route_long_name": str,
        "route_desc": str,
        "route_type": str,
    },
    "stops": {
        "stop_id": str,
        "stop_name": str,
        "stop_lat": float,
        "stop_lon": float,
        "location_type": str,
        "parent_station": str,
    },
    "stop_times": {
        "trip_id": str,
        "arrival_time": str,
        "departure_time": str,
        "stop_id": str,
        "stop_sequence": int,
        "pickup_type": str,
        "drop_off_type": str,
    },
    "trips": {
        "route_id": str,
        "service_id": str,
        "trip_id": str,
        "trip_headsign": str,
        "trip_short_name": str,
        "direction_id": str,
        "block_id": str,
    },
    "transfers": {
        "from_stop_id": str,
        "to_stop_id": str,
        "transfer_type": str,
        "min_transfer_time": str,
    },
}

table_name2index_col = {
    "agency": "agency_id",
    "calendar": "service_id",
    "calendar_dates": "service_id",
    "feed_info": None,
    "routes": "route_id",
    "stops": "stop_id",
    "stop_times": None,
    "trips": "trip_id",
    "transfers": None,
}

dataset_url_format = re.compile(r".*/.*\d{4}-\d{2}-\d{2}.zip")
dataset_regex = re.compile(r"\d{8}")
