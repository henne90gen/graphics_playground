import subprocess
import pandas as pd
import matplotlib.pyplot as plt


def execute_benchmark(bench_name: str) -> pd.DataFrame:
    subprocess.run(["make", "build"])
    process = subprocess.run(["./" + bench_name], cwd="build/src/logic", stdout=subprocess.PIPE)
    output = process.stdout.decode("utf-8")

    lines = output.split("\n")
    lines = [lines[1], *lines[3:-1]]
    lines = list(map(lambda line: list(filter(lambda s: s != "ns", line.split())), lines))

    df = pd.DataFrame(lines[1:], columns=lines[0])
    df["Time"] = df["Time"].astype(float)
    df["CPU"] = df["CPU"].astype(float)
    df["Iterations"] = df["Iterations"].astype(int)
    df["Category"] = df["Benchmark"].apply(lambda s: s.split("/")[0])
    return df


def run_bench():
    bench_name = "MetaBallsBench"
    df = execute_benchmark(bench_name)
    print(df)
    # iterate over categories
    # for each one move the contents of 'Time' into its own column
    df[["Category", "Time"]].groupby("Category").plot.line()
    plt.show()
