from click import *
from .analyze_build_report import analyze_build_report
from .generate_scene_template import generate_scene_template
from .generate_coding_train import generate_coding_train


@group()
def main():
    pass


@main.command()
@argument("file_name", default=None, required=False)
def analyze(file_name):
    analyze_build_report(file_name)


@main.command()
@argument("scene_name", required=True)
def scene_template(scene_name):
    generate_scene_template(scene_name)


@main.command()
def coding_train():
    generate_coding_train()


if __name__ == "__main__":
    main()
