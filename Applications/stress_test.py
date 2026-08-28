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


def discover_executables(target, exclude):
    if os.path.isfile(target):
        if not is_executable(target):
            raise ValueError(f"File is not executable: {target}")
        return [os.path.abspath(target)]
    if os.path.isdir(target):
        execs = []
        for entry in os.listdir(target):
            full = os.path.join(target, entry)
            if not is_executable(full):
                continue
            if os.path.splitext(entry)[0].lower() in exclude:
                continue
            execs.append(os.path.abspath(full))
        if not execs:
            raise ValueError("No executables found in directory.")
        return execs
    raise ValueError(f"Invalid path: {target}")


def start_stress(executables, thread_count):
    stop = False
    failed = False
    stop_lock = threading.Lock()
    processes = {}
    def request_stop():
        nonlocal stop
        stop = True
        for process in processes.values():
            try:
                process.kill()
            except OSError:
                pass
    def handle_sigint(signum, frame):
        with stop_lock:
            request_stop()
    signal.signal(signal.SIGINT, handle_sigint)
    def worker(idx):
        nonlocal failed
        iteration = 0
        exe_index = 0
        while True:
            with stop_lock:
                if stop:
                    return
            exe = executables[exe_index]
            exe_index = (exe_index + 1) % len(executables)
            try:
                process = subprocess.Popen([exe], stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE, text=True)
            except Exception as e:
                with stop_lock:
                    if not stop:
                        failed = True
                        request_stop()
                        print(f"[worker {idx}] crash at iteration {iteration}")
                        print(f"Executable: {exe}")
                        print(f"Exception: {e}")
                return
            with stop_lock:
                if stop:
                    process.kill()
                    process.communicate()
                    return
                processes[idx] = process
            out, err = process.communicate()
            with stop_lock:
                processes.pop(idx, None)
                if stop:
                    return
                if process.returncode != 0:
                    failed = True
                    request_stop()
                    print(
                        f"[worker {idx}] failure at iteration {iteration}")
                    print(f"Executable: {exe}")
                    print(f"Exit code: {process.returncode}")
                    print("\n--- STDOUT ---\n")
                    print(out)
                    print("\n--- STDERR ---\n")
                    print(err)
                    return
            iteration += 1
    threads = [threading.Thread(target=worker, args=(i,))
        for i in range(thread_count)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    return failed


def main():
    cpu_count = multiprocessing.cpu_count()
    default_threads = max(cpu_count - 1, 1)
    parser = argparse.ArgumentParser(description="Parallel stress tester.")
    parser.add_argument("--target", "-t", required=True,
        help="Path to a test executable or a folder of executables.")
    parser.add_argument("--threads", "-j", type=int, default=default_threads,
        help=f"Number of worker threads "
            f"(default: CPU cores - 1 = {default_threads})")
    parser.add_argument("--exclude", "-x", default="",
        help="Comma separated executable names to skip, without extension. "
            "Suites that bind fixed ports fail when several copies run at "
            "once, so exclude them here.")
    args = parser.parse_args()
    if args.threads < 1:
        print("At least one worker thread is required.")
        sys.exit(1)
    exclude = {name.strip().lower()
        for name in args.exclude.split(",") if name.strip()}
    try:
        executables = discover_executables(args.target, exclude)
    except ValueError as e:
        print(e)
        sys.exit(1)
    if start_stress(executables, args.threads):
        sys.exit(1)


if __name__ == "__main__":
    main()
