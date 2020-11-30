import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt


def run_build():
    subprocess.run(["make", "build"])


def collect_benchmarks():
    result = []
    base_dir = "build/bin"
    for f in os.listdir(base_dir):
        if ".cmake" in f:
            continue
        if ".png" in f:
            continue
        if os.path.isdir(os.path.join(base_dir, f)):
            continue
        if "bench" not in f:
            continue
        result.append(f)
    return result


def construct_df(output: str) -> pd.DataFrame:
    lines = output.split("\n")
    lines = [lines[1], *lines[3:-1]]
    lines = list(map(lambda line: list(filter(lambda s: s != "ns", line.split())), lines))

    df = pd.DataFrame(lines[1:], columns=lines[0])
    df["Time"] = df["Time"].astype(float)
    df["CPU"] = df["CPU"].astype(float)
    df["Iterations"] = df["Iterations"].astype(int)
    df["Function"] = df["Benchmark"].apply(lambda s: s.split("/")[0])
    df["Parameters"] = df["Benchmark"].apply(lambda s: "/".join(s.split("/")[1:]))

    result = pd.DataFrame()
    funcs = df["Function"].unique()
    result["Parameters"] = df["Parameters"][df["Function"] == funcs[0]]

    for func in funcs:
        result[func] = df["Time"][df["Function"] == func].reset_index()["Time"]

    return result


def execute_benchmark(bench_name: str) -> pd.DataFrame:
    process = subprocess.run(["./" + bench_name], cwd="build/bin", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = process.stdout.decode("utf-8")
    return construct_df(output)


def create_graph(df: pd.DataFrame, bench_name: str) -> None:
    df.plot.line(
        x="Parameters",
        ylabel="ns",
        xlabel="",
        title=bench_name,
        grid=True
    )


def run_bench(show_plots: bool, graph_dir: str) -> None:
    run_build()
    benchmarks = collect_benchmarks()

    for bench_name in benchmarks:
        print()
        print("Running", bench_name)
        df = execute_benchmark(bench_name)
        print(df)
        create_graph(df, bench_name)
        plt.savefig(os.path.join(graph_dir, bench_name + ".png"))

    if show_plots:
        plt.show()


def main() -> None:
    test_output = """-------------------------------------------------------------
Benchmark                   Time             CPU   Iterations
-------------------------------------------------------------
BM_Exp/8                  506 ns          505 ns      1379283
BM_Exp/64                2716 ns         2714 ns       255015
BM_Exp/512              20502 ns        20488 ns        34210
BM_InverseDist/8          461 ns          461 ns      1527887
BM_InverseDist/64        2389 ns         2387 ns       293121
BM_InverseDist/512      17786 ns        17766 ns        39418
"""
    df = construct_df(test_output)
    print(df)
    create_graph(df, "MetaBallsBench")
    plt.show()


if __name__ == "__main__":
    main()
