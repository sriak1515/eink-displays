local_stops_ids = tuple([
    "8501124:0:1", # Cully, voie 1?
    "8501124:0:2", # Cully, voie 2?
    "8501124:0:3", # Cully, voie 3?
    "8570559", # Cully gare Post?
    "8501125:0:1", # Epesses CFF, voie 1?
    "8501125:0:2", # Epesses CFF, voie 2?
    "8510137", # Epesses, gare Post?
    "8570561", # Epesses, village
])

local_stops_ids_merge = {
    "8501124:0:1": "Cully",
    "8501124:0:2": "Cully",
    "8501124:0:3": "Cully",
    "8570559": "Cully, gare",
    "8501125:0:1": "Epesses",
    "8501125:0:2": "Epesses",
    "8510137": "Epesses",
    "8570561": "Epesses, village"
}

gtfs_dataset_homepage = "https://opentransportdata.swiss/de/dataset/timetable-2024-gtfs2020"