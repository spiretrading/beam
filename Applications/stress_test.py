import os
import platform
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
        if is_executable(target):
            return [os.path.abspath(target)]
        else:
            raise ValueError(f"File is not executable: {target}")
    elif os.path.isdir(target):
        execs = []
        for entry in os.listdir(target):
            full = os.path.join(target, entry)
            if is_executable(full):
                execs.append(os.path.abspath(full))
        if not execs:
            raise ValueError("No executables found in directory.")
        return execs
    raise ValueError(f"Invalid path: {target}")


def stress_test_binary(binary, parallel):
    stop = False
    stop_lock = threading.Lock()

    def worker(idx):
        nonlocal stop
        iteration = 0
        while True:
            with stop_lock:
                if stop:
                    return
            try:
                result = subprocess.run([binary], stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE, text=True)
            except Exception as e:
                with stop_lock:
                    if not stop:
                        stop = True
                        print(f"[{os.path.basename(binary)}][worker {idx}] "
                              "crash at iteration {iteration}")
                        print(f"Exception: {e}")
                return
            if result.returncode != 0:
                with stop_lock:
                    if not stop:
                        stop = True
                        print(f"[{os.path.basename(binary)}][worker {idx}] "
                              "failure at iteration {iteration}")
                        print(f"Exit code: {result.returncode}")
                        print("\n--- STDOUT ---\n")
                        print(result.stdout)
                        print("\n--- STDERR ---\n")
                        print(result.stderr)
                return
            iteration += 1
    threads = [
        threading.Thread(target=worker, args=(i,)) for i in range(parallel)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()


def main():
    if len(sys.argv) < 3:
        print("Usage: python stress.py <binary_or_folder> <parallel>")
        sys.exit(1)
    target = sys.argv[1]
    parallel = int(sys.argv[2])
    try:
        executables = discover_executables(target)
    except ValueError as e:
        print(e)
        sys.exit(1)
    print("Executables to test:")
    for e in executables:
        print("  ", e)
    print()
    for exe in executables:
        print(f"=== Stress testing: {exe} ===")
        stress_test_binary(exe, parallel)
        print()


if __name__ == "__main__":
    main()
