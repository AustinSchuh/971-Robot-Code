import argparse
import os
import shutil
import subprocess
import sys
import tempfile

DESCRIPTION = """
This script provides a convenient way to experiment with the starter
and starter_cmd in particular. To run this, run:
$ bazel run  //aos/starter:starter_demo
This will then print out instructions for running starter_cmd.

If running via bazel, you should not need to specify the positional
arguments.
"""

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description=DESCRIPTION,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("starterd", help="Location of starterd")
    parser.add_argument("configs", help="Location of the config files")
    parser.add_argument("ping", help="Location of ping")
    parser.add_argument("pong", help="Location of pong")
    parser.add_argument("starter_cmd", help="Location of starter_cmd")
    args = parser.parse_args()

    # Copy all the interesting files into a temporary directory and run
    # starterd. starterd expects the binaries it is going to run to be
    # in its working directory; ping and pong also expect the configs
    # to be placed in a specific location.
    # TODO(james): Add piping (or just modify the config) such that
    # ping and pong don't need specially placed configs and so that everything
    # can take a new --shm_base to allow cleaner running on shared systems.
    with tempfile.TemporaryDirectory() as tmpdir:
        shutil.copy(args.starterd, tmpdir + "/starterd")
        shutil.copy(args.ping, tmpdir + "/ping")
        shutil.copy(args.pong, tmpdir + "/pong")
        shutil.copy(args.starter_cmd, tmpdir + "/starter_cmd")
        print(f"Running starter from {tmpdir}")

        print(f"\n\nTo run starter_cmd, do:\ncd {tmpdir}\n./starter_cmd\n\n")

        for config in args.configs.split(' '):
            basename = os.path.basename(config)
            suffix = '.'.join(basename.split('.')[1:])
            # ping/pong expect configs at aos/events/pingpong_config.* but
            # starter* expect them at ./config.*
            destination = f"{tmpdir}/aos/events/{basename}"
            os.makedirs(os.path.dirname(destination), exist_ok=True)
            shutil.copy(config, destination)
            shutil.copy(config, f"{tmpdir}/config.{suffix}")
        args = [tmpdir + "/starterd"]
        subprocess.run(args, check=True, cwd=tmpdir)
