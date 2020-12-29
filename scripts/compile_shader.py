import os


def run_compile_shader(input_file: str, output_file: str):
    parts = input_file.split("/")
    file_name_parts = parts[-1].split(".")
    shader_name = parts[-2] + "_" + file_name_parts[0]

    with open(input_file) as f:
        shader_lines = f.readlines()

    shader_lines = list(map(lambda l: l.replace("\n", ""), shader_lines))

    max_line_length = 0
    for line in shader_lines:
        if len(line) > max_line_length:
            max_line_length = len(line)
    max_line_length += 1

    lines = [
        "extern \"C\" {\n"
        f"unsigned int {shader_name}_len = {len(shader_lines)};\n",
        f"const char *{shader_name}[] = {{\n"
    ]
    for line in shader_lines:
        line = line.replace("\"", "\\\"")
        lines.append(f"      \"{line}\\n\",\n")
    lines.append("};\n")

    lines.append(f"int {shader_name}_line_lens[{len(shader_lines)}] = {{")

    lens = list(map(str, map(lambda l: l + 1, map(len, shader_lines))))
    lines.append(", ".join(lens))
    lines.append("};\n")
    lines.append(f"const char* {shader_name}_file_path = \"{input_file}\";\n")
    lines.append("}\n")

    output_dir = os.path.split(output_file)[0]
    if output_dir and not os.path.exists(output_dir):
        os.mkdir(output_dir)

    with open(output_file, "w+") as f:
        f.writelines(lines)
