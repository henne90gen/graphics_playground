import os
import re
import sys
from typing import Tuple, Union

CPP_TEMPLATE = """\
#include "{name}.h"

#include "Main.h"

DEFINE_SCENE_MAIN({name})
DEFINE_DEFAULT_SHADERS({folder_name}_{name})

void {name}::setup() {{
    shader = CREATE_DEFAULT_SHADER({folder_name}_{name});
}}

void {name}::destroy() {{}}

void {name}::tick() {{}}\
"""

H_TEMPLATE = """\
#pragma once

#include "gl/Shader.h"
#include "Scene.h"

#include <functional>

class {name} : public Scene {{
  public:
    explicit {name}() : Scene("{name}"){{}};
    ~{name}() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
}};\
"""

VERTEX_TEMPLATE = """\
#version 330 core

in vec3 position;

void main() {{
    gl_Position = vec4(position, 1.0);
}}\
"""

FRAGMENT_TEMPLATE = """\
#version 330 core

out vec4 color;

void main() {{
    color = vec4(1.0, 1.0, 1.0, 1.0);
}}\
"""

CMAKE_LISTS_TEMPLATE = """
create_scene({cpp_file})\
"""


def convert_camel_to_snake_case(name: str):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def create_folder(scene_name: str) -> Tuple[str, Union[bytes, str]]:
    folder_name = convert_camel_to_snake_case(scene_name)
    folder_path = os.path.join("./src/scenes", folder_name)
    if os.path.exists(folder_path):
        raise ValueError(f"Scene {scene_name} already exists!")
    os.mkdir(folder_path)
    return folder_name, folder_path


def write_template(file_path, template, **kwargs):
    lines = template.format(**kwargs).split("\n")
    with open(file_path, "w+") as f:
        f.writelines(map(lambda line: line + "\n", lines))


def generate_scene_template(scene_name: str):
    folder_name, folder_path = create_folder(scene_name)

    cpp_path = os.path.join(folder_path, f"{scene_name}.cpp")
    write_template(cpp_path, CPP_TEMPLATE,
                   name=scene_name, folder_name=folder_name)

    h_path = os.path.join(folder_path, f"{scene_name}.h")
    write_template(h_path, H_TEMPLATE, name=scene_name)

    vertex_path = os.path.join(folder_path, f"{scene_name}Vert.glsl")
    write_template(vertex_path, VERTEX_TEMPLATE, name=scene_name)

    fragment_path = os.path.join(folder_path, f"{scene_name}Frag.glsl")
    write_template(fragment_path, FRAGMENT_TEMPLATE, name=scene_name)

    cmake_lists_path = os.path.join(folder_path, "CMakeLists.txt")
    write_template(cmake_lists_path, CMAKE_LISTS_TEMPLATE, cpp_file=f"{scene_name}.cpp")


def main():
    if len(sys.argv) != 2:
        print("Usage: python generate_scene_template.py <SceneName>")
        return

    generate_scene_template(sys.argv[1])


if __name__ == "__main__":
    main()
