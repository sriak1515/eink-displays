import logging
import os
import shutil
import tempfile
from datetime import datetime, timedelta
from pathlib import Path

import duckdb
import requests
from bs4 import BeautifulSoup
from gtfs.config import gtfs_dataset_homepage
from gtfs.consts import (
    dataset_name_regex,
    dataset_extension,
    dataset_url_regex,
    table_name2dtypes,
)
from utils import (
    create_folder_if_not_exists,
    download_and_extract_zip,
)

logger = logging.getLogger(__name__)


class GtfsDataset:
    def __init__(self, save_dir: str, dataset_homepage=gtfs_dataset_homepage):
        self.save_dir = save_dir
        self.dataset_homepage = dataset_homepage
        self.latest_dataset_update_check = datetime(1970, 1, 1)
        self.transport_abbr_mappings_path = (
            Path(__file__)
            .parent.joinpath("assets")
            .joinpath("transport_name_mapping.csv")
        )


    def get_date_stop_times(self, date: datetime, stop_ids = []):
        date_yyymmdd = int(date.strftime("%Y%m%d"))
        date_yy = int(date.strftime("%Y"))
        date_mm = int(date.strftime("%m"))
        date_dd = int(date.strftime("%d"))
        weekday_str = date.strftime("%A").lower()
        stop_ids_filter = "" if len(stop_ids) == 0 else f"AND stop_times.stop_id in {str(tuple(stop_ids))}"
        logger.info(f"Opening dataset {self.get_latest_local_dataset_file()}")
        with duckdb.connect(os.path.join(self.save_dir, self.get_latest_local_dataset_file())) as db:
#                JOIN (
#                    SELECT stop_times.trip_id, last_stop_sequence, stop_name as last_stop_name
#                    FROM (
#                        SELECT trip_id, MAX(stop_sequence) as last_stop_sequence
#                        FROM stop_times
#                        GROUP BY trip_id
#                    ) as max_sequence
#                    INNER JOIN stop_times ON (max_sequence.trip_id = stop_times.trip_id AND max_sequence.last_stop_sequence = stop_times.stop_sequence)
#                    INNER JOIN stops ON (stops.stop_id = stop_times.stop_id)
#                ) as last_stop ON (last_stop.trip_id = stop_times.trip_id)
#
            #"""SELECT stop_times.trip_id, stop_times.stop_id, stop_times.stop_sequence, routes.route_short_name, stops.stop_name, trips.trip_headsign, stop_times.arrival_time, stop_times.departure_time, transport_name_mapping.FR"""
            return db.execute(f"""
                SELECT  *,
                        CASE
                            WHEN regexp_full_match(departure_time, '^([01]?[0-9]|2[0-4]):[0-5][0-9]:[0-5][0-9]$')
                            THEN make_timestamp({date_yy}, {date_mm}, {date_dd},
                                cast(split_part(departure_time, ':', 1) as INTEGER) % 24,
                                cast(split_part(departure_time, ':', 2) as INTEGER),
                                cast(split_part(departure_time, ':', 3) AS INTEGER))
                            ELSE NULL
                        END as departure_timestamp,
                        CASE
                            WHEN regexp_full_match(arrival_time, '^([01]?[0-9]|2[0-4]):[0-5][0-9]:[0-5][0-9]$')
                            THEN make_timestamp({date_yy}, {date_mm}, {date_dd},
                                cast(split_part(arrival_time, ':', 1) as INTEGER) % 24,
                                cast(split_part(arrival_time, ':', 2) as INTEGER),
                                cast(split_part(arrival_time, ':', 3) AS INTEGER))
                            ELSE NULL
                        END as arrival_timestamp
                FROM stop_times
                JOIN stops ON (stops.stop_id = stop_times.stop_id)
                JOIN trips ON (trips.trip_id = stop_times.trip_id)
                JOIN routes on (routes.route_id = trips.route_id)
                JOIN calendar ON (calendar.service_id = trips.service_id)
                JOIN (
                     SELECT trip_id, MAX(stop_sequence) as last_stop_sequence
                     FROM stop_times
                     GROUP BY trip_id
                ) as last_stop ON (last_stop.trip_id = stop_times.trip_id)
                JOIN transport_name_mapping ON (transport_name_mapping.Abbreviation = routes.route_desc)
                WHERE
                stop_times.stop_sequence != last_stop.last_stop_sequence
                {stop_ids_filter}
                AND (
                        (
                        calendar.{weekday_str} = 1
                        AND calendar.start_date < {date_yyymmdd}
                        AND calendar.end_date > {date_yyymmdd}
                        )
                    OR (
                        trips.service_id IN (
                            SELECT service_id
                            FROM calendar_dates
                            WHERE date={date_yyymmdd}
                            AND exception_type = 1
                            )
                        )
                    )
                AND trips.service_id NOT IN (
                    SELECT service_id
                    FROM calendar_dates
                    WHERE date={date_yyymmdd}
                    AND exception_type = 2)
                ORDER BY departure_timestamp, departure_time
                """).df()

    def update_dataset(
        self,
        force=False,
        cleanup_old_datasets=True,
        table_extension=".txt",
    ):
        create_folder_if_not_exists(self.save_dir)
        latest_dataset_url = self.get_latest_dataset_url()
        if latest_dataset_url is None:
            return False
        latest_remote_dataset_date = self.get_date_from_dataset_url(latest_dataset_url)
        latest_local_dataset_date = self.get_latest_local_dataset_date()

        self.latest_dataset_update_check = datetime.now()

        if latest_remote_dataset_date <= latest_local_dataset_date and not force:
            logger.info(
                f"Local dataset {latest_local_dataset_date} is same or newer than remote {latest_remote_dataset_date}"
            )
            return True

        # Create a temporary directory
        with tempfile.TemporaryDirectory() as temp_dir:
            download_dir = os.path.join(
                temp_dir, latest_remote_dataset_date + "_source"
            )
            logger.info(f"Downloading dataset {latest_dataset_url} to {download_dir}")
            if not download_and_extract_zip(latest_dataset_url, download_dir, force):
                logger.error("Error while downloading the dataset")
                return False

            downloaded_tables = set(
                [
                    os.path.basename(path).replace(table_extension, "")
                    for path in os.listdir(download_dir)
                ]
            )
            extra_tables = downloaded_tables.difference(table_name2dtypes.keys())
            if len(extra_tables) > 0:
                extra_tables_string = []
                for table in extra_tables:
                    os.remove(os.path.join(download_dir, table + table_extension))
                    extra_tables_string.append(f"- {table}")
                logger.warn(
                    "Removed the following extra tables: "
                    + "\n".join(extra_tables_string)
                )

            missing_tables = set(table_name2dtypes.keys()).difference(downloaded_tables)
            if len(missing_tables) > 0:
                logger.error(
                    "Missing the tables: "
                    + "\n".join([f"- {table}" for table in missing_tables])
                )
                return False

            dataset_name = latest_remote_dataset_date + dataset_extension
            dataset_temp_path = os.path.join(temp_dir, dataset_name)
            with duckdb.connect(dataset_temp_path) as db:
                db.sql(
                    f"create table transport_name_mapping as select * from read_csv('{self.transport_abbr_mappings_path}')"
                )
                for table_name, dtypes in table_name2dtypes.items():
                    db.sql(
                        f"create table {table_name} as select * from read_csv('{os.path.join(download_dir, table_name + '.txt')}', dtypes={list(dtypes.values())}, ignore_errors=True)"
                    )

            # Move the dataset to the correct location
            shutil.move(dataset_temp_path, os.path.join(self.save_dir, dataset_name))

        # Remove the old datasets if needed
        if cleanup_old_datasets:
            for dataset in set(self.get_local_dataset_files()).difference(
                [latest_remote_dataset_date + dataset_extension]
            ):
                logger.info(f"Removing old dataset {dataset}")
                os.remove(os.path.join(self.save_dir, dataset))

        return True

    def get_local_dataset_files(self):
        local_dataset_files = []
        for folder in os.listdir(self.save_dir):
            basename = os.path.basename(folder)
            if dataset_name_regex.match(basename):
                local_dataset_files.append(basename)
        return local_dataset_files

    def get_latest_local_dataset_file(self):
        if datetime.now() - self.latest_dataset_update_check > timedelta(hours=24):
            logger.info(f"Not updated since {datetime.now() - self.latest_dataset_update_check}, last update was ({self.latest_dataset_update_check}), updating.")
            self.update_dataset()

        local_dataset_files = self.get_local_dataset_files()
        return sorted(local_dataset_files)[-1] if len(local_dataset_files) > 0 else None

    def get_latest_local_dataset_date(self):
        local_dataset_files = self.get_local_dataset_files()
        return (
            sorted(local_dataset_files)[-1].replace(dataset_extension, "")
            if len(local_dataset_files) > 0
            else "19700101"
        )

    def get_latest_dataset_url(self):
        request = requests.get(self.dataset_homepage)
        if request.status_code != 200:
            logger.error(
                f"Error while requestion the dataset homepage, got status code {request.status_code}"
            )
            return None

        soup = BeautifulSoup(request.content, "html.parser")
        resource_list = soup.find("ul", {"class": "resource-list"})
        if resource_list is None:
            logger.error("Could not find resource list in dataset page")
            return None
        resource = resource_list.find("li")
        if resource is None:
            logger.error("Could not find resource in resources list in dataset page")
            return None
        link = resource.find("a", attrs={"href": True, "download": True})
        if link is None:
            logger.error("Could not find link in resource in dataset page")
            return None
        url = link["href"]
        if not dataset_url_regex.match(url):
            return None
        return url

    def get_date_from_dataset_url(self, url):
        return url.split("/")[-1].split("_")[-1].split(".")[0].replace("-", "")
