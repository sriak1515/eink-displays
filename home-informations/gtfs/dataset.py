import io
import os
import shutil
import uuid
import zipfile

import pandas as pd
import requests
from bs4 import BeautifulSoup

from gtfs.consts import dataset_url_format, table_name2dtypes, table_name2index_col


def convert_csv_to_feather(csv_file, output_filename, dtypes=None, index_col=None):
    pd.read_csv(
        csv_file,
        index_col=index_col,
        dtype=dtypes,
        keep_default_na=False,
        engine="pyarrow",
    ).to_feather(output_filename, compression="lz4")

def get_latest_dataset_url(homepage):
    request = requests.get(homepage)
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
    if not dataset_url_format.match(url):
        return None
    return url


def get_date_from_dataset_url(url):
    return url.split("/")[-1].split("_")[-1].split(".")[0].replace("-", "")


def update_symlink(source, target):
    source = os.path.abspath(source)
    target = os.path.abspath(target)
    if os.path.exists(target) and not os.path.islink(target):
        logger.error(f"Cannot update the symlink for {target}, it is not a link")
        return False
    temp_target = target + str(uuid.uuid4)
    try:
        os.symlink(source, temp_target)
        os.replace(temp_target, target)
        return True
    except:
        if os.path.exists(temp_target):
            os.remove(temp_target)
        return False


def download_and_extract_zip(url, extract_to, overwrite=False):
    path_exists = os.path.exists(extract_to)
    if overwrite:
        if path_exists:
            shutil.rmtree(extract_to)
    else:
        if path_exists:
            logger.error(
                f"Error: the path {extract_to} already exists and overwrite is false."
            )
            return False
    response = requests.get(url)

    if response.status_code == 200:
        zip_file = io.BytesIO(response.content)

        with zipfile.ZipFile(zip_file, "r") as zip_ref:
            zip_ref.extractall(extract_to)
        return True
    else:
        return False


def get_local_dataset_folders(base_dir, dataset_regex=dataset_regex):
    local_dataset_folders = []
    for folder in os.listdir(base_dir):
        basename = os.path.basename(folder)
        if dataset_regex.match(basename):
            local_dataset_folders.append(basename)
    return local_dataset_folders


def get_latest_dataset_folder(base_dir, dataset_regex=dataset_regex):
    local_dataset_folders = get_local_dataset_folders(base_dir, dataset_regex)
    return (
        sorted(local_dataset_folders)[-1]
        if len(local_dataset_folders) > 0
        else "19700101"
    )


def get_gtfs_data_if_needed(
    base_dir,
    dataset_homepage,
    force=False,
    cleanup_old_datasets=True,
    table_extension=".txt",
):
    latest_dataset_url = get_latest_dataset_url(dataset_homepage)
    if latest_dataset_url is None:
        return False
    latest_remote_dataset_date = get_date_from_dataset_url(latest_dataset_url)
    latest_local_dataset_date = get_latest_dataset_folder(base_dir)

    if latest_remote_dataset_date <= latest_local_dataset_date and not force:
        logger.info(
            f"Local dataset {latest_local_dataset_date} is same or newer than remote {latest_remote_dataset_date}"
        )
        update_symlink(
            os.path.join(base_dir, latest_local_dataset_date),
            os.path.join(base_dir, "latest"),
        )
        return True

    download_dir = os.path.join(base_dir, latest_remote_dataset_date)
    if not download_and_extract_zip(latest_dataset_url, download_dir, force):
        logger.error("Error while download the dataset")
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
            "Removed the following extra tables: " + "\n".join(extra_tables_string)
        )

    missing_tables = set(table_name2dtypes.keys()).difference(downloaded_tables)
    if len(missing_tables) > 0:
        logger.error(
            "Missing the tables: "
            + "\n".join([f"- {table}" for table in missing_tables])
        )
        return False

    for table, dtypes in table_name2dtypes.items():
        basename = os.path.join(download_dir, table)
        convert_csv_to_feather(
            basename + table_extension, basename + ".feather", dtypes=dtypes
        )
        os.remove(basename + table_extension)

    update_symlink(
        os.path.join(base_dir, latest_remote_dataset_date),
        os.path.join(base_dir, "latest"),
    )

    if cleanup_old_datasets:
        for dataset in set(get_local_dataset_folders(base_dir)).difference(
            [latest_remote_dataset_date]
        ):
            logger.info(f"Removing old dataset {dataset}")
            shutil.rmtree(os.path.join(base_dir, dataset))
    return True

def open_dataset_tables(base_dir, dataset_name="latest"):
    tables = {}
    for table_name, index_col in table_name2index_col.items():
        table_path = os.path.join(base_dir, dataset_name, table_name + ".feather")
        if not os.path.exists(table_path):
            logger.error(f"Missing table {table_path}")
            return None
        table = pd.read_feather(table_path)
        if index_col is not None:
            table.set_index(index_col, inplace=True)
        tables[table_name] = table
    return tables