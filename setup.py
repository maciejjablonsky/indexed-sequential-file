import subprocess
import argparse
import os
import shutil
import sys

parser = argparse.ArgumentParser()
parser.add_argument(
    "--run-cmake",
    help="Runs cmake command to setup build directory",
    action="store_true",
)
parser.add_argument(
    "--clean",
    help="Deletes build directory before running any other command",
    action="store_true",
)
parser.add_argument(
    "--open-code", help="Opens repository in Visual Studio Code", action="store_true"
)
parser.add_argument(
    "--open-vs", help="Open solution in Visual Studio", action="store_true"
)
parser.add_argument(
    "--run-tests", help="Run tests after release build", action="store_true"
)
parser.add_argument("--build-release", help="Build release target", action="store_true")
parser.add_argument("--build-debug", help="Build debug target", action="store_true")
args = parser.parse_args(args=None if sys.argv[1:] else ["--help"])

repository_path = os.path.dirname(os.path.realpath(__file__))
build_dir_path = repository_path + "/../build-{}".format(
    os.path.basename(repository_path)
)

if args.clean:
    shutil.rmtree(build_dir_path, ignore_errors=True)

if not os.path.isdir(build_dir_path):
    os.mkdir(build_dir_path)
    os.chmod(build_dir_path, 0o777)

if args.open_code:
    subprocess.run(["code", repository_path], stderr=subprocess.STDOUT, shell=True)

os.chdir(build_dir_path)
cmake_build_command = ["cmake", repository_path]

if args.run_cmake:
    subprocess.check_call(cmake_build_command, stderr=subprocess.STDOUT, shell=True)

if args.open_vs:
    os.startfile("indexed-sequential-file.sln")

configuration = "Release"

if args.build_release:
    subprocess.check_call(
        ["cmake", "--build", ".", "--config", "Release", "-j"],
        stderr=subprocess.STDOUT,
        shell=True,
    )
if args.build_debug:
    configuration = "Debug"
    subprocess.check_call(
        ["cmake", "--build", ".", "--config", "Debug", "-j"],
        stderr=subprocess.STDOUT,
        shell=True,
    )
if args.run_tests:
    subprocess.check_call(
        ["ctest", "-C", configuration], stderr=subprocess.STDOUT, shell=True
    )
os.chdir(repository_path)