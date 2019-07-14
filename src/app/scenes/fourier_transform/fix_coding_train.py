from typing import List
from dataclasses import dataclass


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

    return list(map(float, map(remove_comma, filter(clean_parts, parts))))


def to_vector(l: List[float]) -> Vec:
    return Vec(l[0], l[1])


def to_string(v: Vec) -> str:
    return f"\t{{ {v.x}, {v.y} }},\n"


def scale(s: float):
    return lambda v: Vec(v.x * s, v.y * s)


def flip_vertically(v: Vec):
    return Vec(v.x, -1 * v.y)


def main():
    with open('codingtrain.js') as f:
        lines = f.readlines()

    scale_factor = 1 / 150
    lines = list(map(to_string,
                     map(flip_vertically,
                         map(scale(scale_factor),
                             map(to_vector,
                                 map(extract_numbers,
                                     filter(clean_up_lines,
                                            lines)
                                     )
                                 )
                             )
                         )
                     )
                 )

    with open('CodingTrain.h', 'w') as f:
        f.write("#pragma once\n")
        f.write("\n")
        f.write("std::vector<glm::vec2> dataPoints = {\n")
        f.writelines(lines)
        f.write("};\n")


if __name__ == "__main__":
    main()
