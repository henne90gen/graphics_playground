from dataclasses import dataclass
from collections import defaultdict


@dataclass
class Warning:
    file_path: str
    line_number: int
    column_number: int
    message: str
    warning_type: str


def convert(s: str) -> Warning:
    path_parts, msg_parts = s.split(": warning: ")
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


def main():
    with open("build/build_report.csv") as f:
        lines = f.readlines()
    warnings = list(
        map(convert,
            filter(warnings_only,
                   filter(remove_carets,
                          map(strip,
                              lines)))))

    counter = defaultdict(lambda: 0)
    for warning in warnings:
        counter[warning.warning_type] += 1
    print(sorted(counter.items(), key=lambda k: k[1], reverse=True))


if __name__ == "__main__":
    main()
