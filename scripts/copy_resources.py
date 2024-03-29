import os
import shutil
import sys

from typing import List


def run_copy_resources(base_dir: str, files: List[str], dest_dir: str):
    for f in files:
        src = f"{base_dir}/{f}"
        dst = f"{dest_dir}/{f}"

        if not os.path.exists(src):
            print(f"Could not find resource '{src}'")
            continue

        destination_directory = "/".join(dst.split("/")[:-1])
        if not os.path.exists(destination_directory):
            os.makedirs(destination_directory)

        if os.path.isfile(src):
            shutil.copyfile(src, dst)
        else:
            if os.path.exists(dst):
                shutil.rmtree(dst)
            shutil.copytree(src, dst)


def main():
    run_copy_resources(sys.argv[1], [sys.argv[2]], sys.argv[3])


if __name__ == "__main__":
    main()
