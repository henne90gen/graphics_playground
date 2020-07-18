from collections import defaultdict
from dataclasses import dataclass
from typing import Optional


@dataclass
class Warning:
    file_path: str
    line_number: int
    column_number: int
    message: str
    warning_type: str


def convert(s: str) -> Optional[Warning]:
    if "warning" in s:
        try:
            path_parts, msg_parts = s.split(": warning: ")
        except ValueError:
            return None
    elif "error" in s:
        try:
            path_parts, msg_parts = s.split(": error: ")
        except ValueError:
            return None
    else:
        raise Exception(f"Could not determine warning type for: {s}")
    path_parts = path_parts.split(":")
    path = path_parts[0]
    line_number = int(path_parts[1])
    column_number = int(path_parts[2])
    msg_parts = msg_parts.split("[")
    msg = msg_parts[0].strip()
    warning_type = msg_parts[1][:-1]
    return Warning(
        file_path=path,
        line_number=line_number,
        column_number=column_number,
        message=msg,
        warning_type=warning_type
    )


def warnings_only(s: str) -> bool:
    return "warning" in s


def remove_carets(s: str) -> bool:
    return "^" != s


def strip(s: str) -> str:
    return s.strip()


def show_warnings_for_file(warnings, file_path):
    print(f"Warnings for {file_path}:")
    for warning in warnings:
        if file_path not in warning.file_path:
            continue

        print(warning.line_number, warning.warning_type)


def file_path_count(warnings):
    counter = defaultdict(lambda: 0)
    for warning in warnings:
        counter[warning.file_path] += 1
    return counter


def warning_type_count(warnings):
    counter = defaultdict(lambda: 0)
    for warning in warnings:
        counter[warning.warning_type] += 1
    return counter


def show_counter(counter):
    for elem, count in sorted(counter.items(), key=lambda k: k[1], reverse=False):
        print(count, elem)


def analyze_build_report(file_name: Optional[str]):
    with open("build/clang-tidy-report.txt") as f:
        lines = f.readlines()

    warnings = list(
        filter(lambda x: x is not None,
               map(convert,
                   filter(warnings_only,
                          filter(remove_carets,
                                 map(strip, lines)
                                 )
                          )
                   )
               )
    )

    show_counter(warning_type_count(warnings))

    print()
    show_counter(file_path_count(warnings))

    if file_name is not None:
        print()
        show_warnings_for_file(warnings, file_name)
