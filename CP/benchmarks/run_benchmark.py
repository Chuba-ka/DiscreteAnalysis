import csv
import hashlib
import json
import platform
import random
import statistics
import subprocess
import tempfile
import time
from datetime import datetime
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark_results"
SIZES = [1000, 5000, 10000, 50000, 100000]
FAMILIES = ["constant", "periodic", "random2", "random26", "blocks"]


def make_text(family, size):
    rng = random.Random(8062026 + size)
    if family == "constant":
        return "a" * size
    if family == "periodic":
        return ("abracadabra" * ((size + 10) // 11))[:size]
    if family == "random2":
        return "".join(rng.choices("ab", k=size))
    if family == "random26":
        return "".join(rng.choices("abcdefghijklmnopqrstuvwxyz", k=size))
    chunks = []
    length = 0
    while length < size:
        chunk = rng.choice("abcdefghijklmnopqrstuvwxyz") * rng.randint(1, 100)
        chunks.append(chunk)
        length += len(chunk)
    return "".join(chunks)[:size]


def main():
    OUTPUT.mkdir(exist_ok=True)
    metadata = {
        "date": datetime.now().astimezone().isoformat(),
        "platform": platform.platform(),
        "compiler": subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
        "flags": "-std=c++23 -O2 -DNDEBUG",
        "source_sha256": hashlib.sha256((ROOT / "main.cpp").read_bytes()).hexdigest(),
        "seed": "8062026 + длина строки",
        "samples": 5,
        "batch_min_ms": 20,
        "cli_repetitions": 3,
        "cpu": next(line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")),
        "memory_kib": int(Path("/proc/meminfo").read_text().splitlines()[0].split()[1]),
    }
    raw_rows, cli_rows, summaries = [], [], []
    with tempfile.TemporaryDirectory(prefix="bwt-benchmark-") as directory:
        directory = Path(directory)
        binary, harness = directory / "solution", directory / "benchmark"
        common = ["g++", "-std=c++23", "-O2", "-DNDEBUG"]
        subprocess.run(common + ["-Wall", "-Wextra", "-Wpedantic", str(ROOT / "main.cpp"), "-o", str(binary)], check=True)
        subprocess.run(common + ["-Wno-return-type", str(ROOT / "benchmarks/benchmark.cpp"), "-o", str(harness)], check=True)
        for family in FAMILIES:
            for size in SIZES:
                text = make_text(family, size)
                measured = subprocess.check_output([str(harness)], input=text + "\n", text=True, timeout=60)
                groups = {}
                for operation, sample, repetitions, elapsed in csv.reader(measured.splitlines()):
                    raw_rows.append([family, size, operation, sample, repetitions, elapsed])
                    groups.setdefault(operation, []).append(float(elapsed))
                source = directory / "source.txt"
                source.write_text("compress\n" + text + "\n")
                encoded = subprocess.check_output([str(binary)], input=source.read_bytes())
                compressed = directory / "compressed.txt"
                compressed.write_bytes(b"decompress\n" + encoded)
                timings = {"compress": [], "decompress": []}
                memory = {"compress": [], "decompress": []}
                for operation, data in [("compress", source), ("decompress", compressed)]:
                    for sample in range(3):
                        rss = directory / "rss.txt"
                        with data.open("rb") as stream:
                            begin = time.perf_counter_ns()
                            result = subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(rss), str(binary)], stdin=stream, capture_output=True, check=True, timeout=5)
                            elapsed = (time.perf_counter_ns() - begin) / 1e6
                        expected = encoded if operation == "compress" else (text + "\n").encode()
                        if result.stdout != expected:
                            raise RuntimeError("Результат полного запуска не совпадает с ожидаемым")
                        peak = int(rss.read_text().strip())
                        timings[operation].append(elapsed)
                        memory[operation].append(peak)
                        cli_rows.append([family, size, operation, sample, elapsed, peak])
                summaries.append({
                    "family": family, "size": size,
                    "input_sha256": hashlib.sha256(text.encode()).hexdigest(),
                    "runs": len(encoded.splitlines()), "encoded_bytes": len(encoded),
                    "ratio": len(encoded) / size,
                    "zero_fraction": sum(int(line.split()[0]) for line in encoded.splitlines() if int(line.split()[1]) == 0) / (size + 1),
                    "stages": {op: {"median_ms": statistics.median(v), "min_ms": min(v), "max_ms": max(v)} for op, v in groups.items()},
                    "cli_ms": {op: statistics.median(v) for op, v in timings.items()},
                    "rss_kib": {op: max(v) for op, v in memory.items()},
                })
                print(f"{family:10} {size:6}: сжатие {statistics.median(groups['compress']):.3f} мс; размер {len(encoded)} байт", flush=True)
    for name, headers, rows in [
        ("stages.csv", ["family", "size", "operation", "sample", "batch_repetitions", "ms_per_operation"], raw_rows),
        ("cli.csv", ["family", "size", "operation", "sample", "wall_ms", "peak_rss_kib"], cli_rows),
    ]:
        with (OUTPUT / name).open("w", newline="") as stream:
            writer = csv.writer(stream)
            writer.writerow(headers)
            writer.writerows(rows)
    (OUTPUT / "results.json").write_text(json.dumps({"metadata": metadata, "cases": summaries}, ensure_ascii=False, indent=2))
    print("Замеры сохранены в benchmark_results")


if __name__ == "__main__":
    main()
