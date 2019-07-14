from typing import List
from dataclasses import dataclass
import streams as s

BASE_PATH = "src/app/scenes/fourier_transform"


@dataclass
class Vec:
    x: float
    y: float


def clean_up_lines(line: str) -> bool:
    if "// " in line:
        return False
    if "let drawing" in line:
        return False
    if line.strip() == "":
        return False
    if "];" in line:
        return False
    return True


def extract_numbers(line: str):
    parts = line.split(" ")

    def clean_parts(part: str):
        result = part == '' or part == '{' or part == 'x:' or part == 'y:' or part == '},' or part == '},\n' or part == '}\n' or part == '\n'
        return not result

    def remove_comma(part: str):
        return part.replace(",", "")

    return s.stream(parts) > \
           s.filter(clean_parts) > \
           s.map(remove_comma) > \
           s.map(float) > \
           s.collect()


def to_vector(l: List[float]) -> Vec:
    return Vec(l[0], l[1])


def to_string(v: Vec) -> str:
    return f"\t{{ {v.x}, {v.y} }},\n"


def scale(s: float):
    return lambda v: Vec(v.x * s, v.y * s)


def flip_vertically(v: Vec):
    return Vec(v.x, -1 * v.y)


def read_js_file():
    with open(f"{BASE_PATH}/codingtrain.js") as f:
        lines = f.readlines()
    print("Read coding_train.js")
    return lines


def write_header_file(lines: List[str]):
    with open(f"{BASE_PATH}/CodingTrain.h", 'w') as f:
        f.write("#pragma once\n")
        f.write("\n")
        f.write("std::vector<glm::vec2> dataPoints = {\n")
        f.writelines(lines)
        f.write("};\n")
    print("Wrote CodingTrain.h")


def generate_coding_train():
    lines = read_js_file()

    scale_factor = 1 / 150
    # noinspection PyStatementEffect
    s.stream(lines) > \
    s.filter(clean_up_lines) > \
    s.map(extract_numbers) > \
    s.map(to_vector) > \
    s.map(scale(scale_factor)) > \
    s.map(flip_vertically) > \
    s.map(to_string) > \
    s.collect(write_header_file)
