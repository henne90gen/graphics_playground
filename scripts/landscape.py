import os
import zipfile

import requests

textures = [
    "https://cc0textures.com/get?file=Ground039_1K-PNG.zip",
    "https://cc0textures.com/get?file=Ground037_1K-PNG.zip",
    "https://cc0textures.com/get?file=Ground022_1K-PNG.zip",
]


def download_textures(force: bool):
    print("Downloading Textures")

    for texture in textures:
        dir_name = "src/scenes/landscape/assets/textures/"
        zip_file = dir_name + texture.split("=")[1]
        if not os.path.exists(zip_file) or force:
            print(f"Downloading {texture}")

            headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:77.0) Gecko/20100101 Firefox/77.0"}
            r = requests.get(texture, headers=headers)

            with open(zip_file, 'wb') as f:
                f.write(r.content)
        else:
            print(f"Already downloaded {texture}")

        print(f"Extracting {texture}")
        with zipfile.ZipFile(zip_file, 'r') as zip_ref:
            zip_ref.extractall(dir_name)

    print("Downloading Textures - Done.")


def download(force: bool = False):
    download_textures(force)
