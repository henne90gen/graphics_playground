import os
from typing import Tuple

CPP_TEMPLATE = """\
#include "scenes/{folder_name}/{name}.h"

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
    {name}(GLFWwindow *window, std::function<void(void)>& backToMainMenu)
        : Scene(window, backToMainMenu, "{name}"){{}};
    virtual ~{name}(){{}};

    virtual void setup() override;
    virtual void tick() override;
    virtual void destroy() override;
}};\
"""

VERTEX_TEMPLATE = """\
#version 130

attribute vec3 position;

void main() {{
    gl_Position.xyz = position;
    gl_Position.w = 1.0;
}}\
"""

FRAGMENT_TEMPLATE = """\
#version 130

void main() {{
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}}\
"""


def create_folder(name: str) -> Tuple[str, os.PathLike]:
    folder_name = name.lower()
    folder_path = os.path.join("./src/scenes", folder_name)
    if os.path.exists(folder_path):
        raise ValueError(f"Scene {name} already exists!")
    os.mkdir(folder_path)
    return folder_name, folder_path


def write_template(file_path, template, **kwargs):
    lines = template.format(**kwargs).split("\n")
    with open(file_path, "w+") as f:
        f.writelines(map(lambda line: line + "\n", lines))


def add_to_cmake_lists(cpp_path):
    cpp_path = cpp_path.replace("./", "").replace("src", "${SRC_DIR}")
    cpp_path = f"        {cpp_path}\n"

    with open("CMakeLists.txt") as f:
        lines = f.readlines()

    result_lines = []
    inside_sources = False
    for line in lines:
        if inside_sources:
            if cpp_path in line:
                inside_sources = False
            elif ")" in line:
                inside_sources = False
                result_lines.append(cpp_path)

        if "set(SOURCES" in line:
            inside_sources = True

        result_lines.append(line)

    with open("CMakeLists.txt", "w") as f:
        f.writelines(result_lines)


def main():
    name = input("Name of the scene: ")
    folder_name, folder_path = create_folder(name)

    cpp_path = os.path.join(folder_path, f"{name}.cpp")
    h_path = os.path.join(folder_path, f"{name}.h")
    vertex_path = os.path.join(folder_path, f"{name}.vertex")
    fragment_path = os.path.join(folder_path, f"{name}.fragment")

    write_template(cpp_path, CPP_TEMPLATE,
                   name=name, folder_name=folder_name)
    write_template(h_path, H_TEMPLATE, name=name)
    write_template(vertex_path, VERTEX_TEMPLATE, name=name)
    write_template(fragment_path, FRAGMENT_TEMPLATE, name=name)

    add_to_cmake_lists(cpp_path)


if __name__ == "__main__":
    main()
