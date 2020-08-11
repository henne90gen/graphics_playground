import os
import zipfile
from dataclasses import dataclass

import requests


@dataclass
class Artifact:
    name: str
    url: str


artifacts = [
    Artifact("gravimetry",
             "https://www.geologie.sachsen.de/download/GravUeK400dig.zip"),
    # Artifact("dtm",
    #         "https://geocloud.landesvermessung.sachsen.de/index.php/s/MQq77ZzhgPWqUew/download?path=%2F&files=dgm20_sn.zip")
]


def download(force: bool = False):
    for artifact in artifacts:
        dir_name = f"gis_data/{artifact.name}"
        if not force and os.path.exists(dir_name):
            print(f"Skipping {artifact.name}")
            continue

        print(f"Downloading {artifact.name}")
        r = requests.get(artifact.url)

        zip_file = f"{dir_name}.zip"
        with open(zip_file, 'wb') as f:
            f.write(r.content)

        with zipfile.ZipFile(zip_file, 'r') as zip_ref:
            zip_ref.extractall(dir_name)

        os.remove(zip_file)
