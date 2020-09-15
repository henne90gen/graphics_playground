from typing import List, Optional

from click import *

from .analyze_build_report import analyze_build_report
from .generate_coding_train import generate_coding_train
from .generate_scene_template import generate_scene_template
from .copy_resources import run_copy_resources
from .compile_shader import run_compile_shader
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
@argument("input_file", required=True, nargs=1)
@argument("output_file", required=True, nargs=1)
def compile_shader(input_file: str, output_file: str):
    run_compile_shader(input_file, output_file)


@main.command()
@option("--force", "-f", is_flag=True)
def download_gis_data(force: bool):
    gis_data.download(force)


@main.command()
def analyze_gis_data():
    gis_data.analyze_dtm_files()


@main.command()
@option("--show-plots", "-s", is_flag=True)
@argument("graph_dir", required=False, nargs=1)
def bench(show_plots: bool, graph_dir: Optional[str]):
    if graph_dir is None:
        graph_dir = "build/src/bench"
    from .bench import run_bench
    run_bench(show_plots, graph_dir)


if __name__ == "__main__":
    main()
