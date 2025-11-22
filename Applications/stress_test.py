import argparse
import multiprocessing
import os
import platform
import signal
import subprocess
import sys
import threading

IS_WINDOWS = platform.system().lower().startswith("win")


def is_executable(path):
    if not os.path.isfile(path):
        return False
    if IS_WINDOWS:
        ext = os.path.splitext(path)[1].lower()
        return ext in (".exe", ".bat", ".cmd")
    else:
        return os.access(path, os.X_OK)


def discover_executables(target):
    if os.path.isfile(target):
        if not is_executable(target):
            raise ValueError(f"File is not executable: {target}")
        return [os.path.abspath(target)]
    if os.path.isdir(target):
        execs = []
        for entry in os.listdir(target):
            full = os.path.join(target, entry)
            if is_executable(full):
                execs.append(os.path.abspath(full))
        if not execs:
            raise ValueError("No executables found in directory.")
        return execs
    raise ValueError(f"Invalid path: {target}")


def start_stress(executables, thread_count):
    stop = False
    stop_lock = threading.Lock()
    def handle_sigint(signum, frame):
        nonlocal stop
        with stop_lock:
            stop = True
    signal.signal(signal.SIGINT, handle_sigint)
    def worker(idx):
        nonlocal stop
        iteration = 0
        exe_index = 0
        while True:
            with stop_lock:
                if stop:
                    return
            exe = executables[exe_index]
            exe_index = (exe_index + 1) % len(executables)
            try:
                result = subprocess.run([exe], stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE, text=True)
            except Exception as e:
                with stop_lock:
                    if not stop:
                        stop = True
                        print(f"[worker {idx}] crash at iteration {iteration}")
                        print(f"Executable: {exe}")
                        print(f"Exception: {e}")
                return
            if result.returncode != 0:
                with stop_lock:
                    if not stop:
                        stop = True
                        print(
                            f"[worker {idx}] failure at iteration {iteration}")
                        print(f"Executable: {exe}")
                        print(f"Exit code: {result.returncode}")
                        print("\n--- STDOUT ---\n")
                        print(result.stdout)
                        print("\n--- STDERR ---\n")
                        print(result.stderr)
                return
            iteration += 1
    threads = [threading.Thread(target=worker, args=(i,))
        for i in range(thread_count)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()


def main():
    cpu_count = multiprocessing.cpu_count()
    default_threads = max(cpu_count - 1, 1)
    parser = argparse.ArgumentParser(description="Parallel stress tester.")
    parser.add_argument("--target", "-t", required=True,
        help="Path to a test executable or a folder of executables.")
    parser.add_argument("--threads", "-j", type=int, default=default_threads,
        help=f"Number of worker threads "
            "(default: CPU cores - 1 = {default_threads})")
    args = parser.parse_args()
    try:
        executables = discover_executables(args.target)
    except ValueError as e:
        print(e)
        sys.exit(1)
    start_stress(executables, args.threads)


if __name__ == "__main__":
    main()
