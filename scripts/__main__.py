from typing import List

from click import *

from .analyze_build_report import analyze_build_report
from .generate_coding_train import generate_coding_train
from .generate_scene_template import generate_scene_template
from .copy_resources import run_copy_resources
from . import gis_data


@group()
def main():
    pass


@main.command()
@argument("file_name", default=None, required=False)
def analyze(file_name: str):
    analyze_build_report(file_name)


@main.command()
@argument("scene_name", required=True)
def scene_template(scene_name: str):
    generate_scene_template(scene_name)


@main.command()
def coding_train():
    generate_coding_train()


@main.command()
@argument("base_dir", required=True, nargs=1)
@argument("files", required=True, nargs=-1)
@argument("dest", required=True, nargs=1)
def copy_resources(base_dir: str, files: List[str], dest: str):
    run_copy_resources(base_dir, files, dest)


@main.command()
@option("--force", "-f", is_flag=True)
def download_gis_data(force):
    gis_data.download(force)


@main.command()
def analyze_gis_data():
    gis_data.analyze_dtm_files()


if __name__ == "__main__":
    main()
