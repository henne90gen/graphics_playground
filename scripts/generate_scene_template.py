import os
import re
from typing import Tuple, Union

CPP_TEMPLATE = """\
#include "{name}.h"

void {name}::setup() {{}}

void {name}::destroy() {{}}

void {name}::tick() {{}}\
"""

H_TEMPLATE = """\
#pragma once

#include "scenes/Scene.h"

#include <functional>

class {name} : public Scene {{
public:
    explicit {name}(SceneData &data)
        : Scene(data, "{name}"){{}};
    ~{name}() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
}};\
"""

VERTEX_TEMPLATE = """\
#version 130

attribute vec3 a_Position;

void main() {{
    gl_Position = vec4(a_Position, 1.0);
}}\
"""

FRAGMENT_TEMPLATE = """\
#version 130

void main() {{
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}}\
"""


def convert_camel_to_snake_case(name: str):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def create_folder(scene_name: str) -> Tuple[str, Union[bytes, str]]:
    folder_name = convert_camel_to_snake_case(scene_name)
    folder_path = os.path.join("./src/app/scenes", folder_name)
    if os.path.exists(folder_path):
        raise ValueError(f"Scene {scene_name} already exists!")
    os.mkdir(folder_path)
    return folder_name, folder_path


def write_template(file_path, template, **kwargs):
    lines = template.format(**kwargs).split("\n")
    with open(file_path, "w+") as f:
        f.writelines(map(lambda line: line + "\n", lines))


def correctly_indent_for_cmakelists(path: str):
    path = path.replace("./", "").replace("src/app/", "")
    return f"        {path}\n"


def add_to_cmake_lists(cpp_path: str, vertex_path: str, fragment_path: str):
    cpp_path = correctly_indent_for_cmakelists(cpp_path)
    vertex_path = correctly_indent_for_cmakelists(vertex_path)
    fragment_path = correctly_indent_for_cmakelists(fragment_path)

    cmake_path = "src/app/CMakeLists.txt"
    with open(cmake_path) as f:
        lines = f.readlines()

    result_lines = []
    inside_sources = False
    inside_resources = False
    for line in lines:
        if inside_sources:
            if cpp_path in line:
                inside_sources = False
            elif ")" in line:
                inside_sources = False
                result_lines.append(cpp_path)
        if inside_resources:
            if ")" in line:
                inside_resources = False
                result_lines.append(vertex_path)
                result_lines.append(fragment_path)

        if "set(SOURCES" in line:
            inside_sources = True

        if "set(RESOURCES" in line:
            inside_resources = True

        result_lines.append(line)

    with open(cmake_path, "w") as f:
        f.writelines(result_lines)


def generate_scene_template(scene_name: str):
    folder_name, folder_path = create_folder(scene_name)

    cpp_path = os.path.join(folder_path, f"{scene_name}.cpp")
    h_path = os.path.join(folder_path, f"{scene_name}.h")
    vertex_path = os.path.join(folder_path, f"{scene_name}Vert.glsl")
    fragment_path = os.path.join(folder_path, f"{scene_name}Frag.glsl")

    write_template(cpp_path, CPP_TEMPLATE,
                   name=scene_name, folder_name=folder_name)
    write_template(h_path, H_TEMPLATE, name=scene_name)
    write_template(vertex_path, VERTEX_TEMPLATE, name=scene_name)
    write_template(fragment_path, FRAGMENT_TEMPLATE, name=scene_name)

    add_to_cmake_lists(cpp_path, vertex_path, fragment_path)
