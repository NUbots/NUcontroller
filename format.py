#!/usr/bin/env python3
#
# MIT License
#
# Copyright (c) 2017 NUbots
#
# This file is part of the NUbots codebase.
# See https://github.com/NUbots/NUbots for further info.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import datetime
import multiprocessing
import os
import shutil
import sys
import tempfile
from collections import OrderedDict
from fnmatch import fnmatch
from functools import partial
from subprocess import DEVNULL, PIPE, STDOUT, CalledProcessError
from subprocess import run as sp_run
import argparse

import pygit2

repo = pygit2.Repository(".")

# The extensions that are handled by the various formatters
formatters = OrderedDict()

formatters["clang-format"] = {
    "format": [["clang-format", "-i", "-style=file", "{path}"]],
    "include": ["*.h", "*.c", "*.cc", "*.cxx", "*.cpp", "*.hpp", "*.ipp", "*.frag", "*.glsl", "*.vert", "*.proto"],
    "exclude": [],
}
formatters["isort"] = {
    "format": [["isort", "--quiet", "{path}"]],
    "include": ["*.py"],
    "exclude": [],
}
formatters["black"] = {
    "format": [["black", "--quiet", "{path}"]],
    "include": ["*.py"],
    "exclude": [],
}
formatters["prettier"] = {
    "format": [["prettier", "--write", "{path}"]],
    "include": ["*.js", "*.jsx", "*.ts", "*.tsx", "*.json", "*.css", "*.scss", "*.html", "*.md", "*.yaml", "*.yml"],
    "exclude": ["*.min.*"],
}

# Format or check the file
def _do_format(path, verbose, check=True):
    text = ""
    success = True
    output_path = None
    try:
        # Find the correct formatter and format the file
        formatter = []
        formatter_names = []
        for name, fmt in formatters.items():
            if (any(fnmatch(path, pattern) for pattern in fmt["include"])) and (
                all(not fnmatch(path, pattern) for pattern in fmt["exclude"])
            ):
                formatter_names.append(name)
                formatter.extend(fmt["format"])

        # If we don't have a formatter then skip this file
        if len(formatter) == 0:
            return f"Skipping {path} as it does not match any of the formatters\n" if verbose >= 1 else "", True

        text = f"Formatting {path} with {', '.join(formatter_names)}\n"

        tmp_dir = tempfile.TemporaryDirectory(dir=os.path.dirname(path))

        # Make a copy of the file to do the formatting on
        output_path = os.path.join(tmp_dir.name, os.path.basename(path))
        shutil.copy(path, output_path)

        # Apply our arguments to the formatter command
        args = {"path": output_path}
        formatter = [[arg.format(**args) for arg in tool] for tool in formatter]

        # Format the code
        tool_text = ""
        run_args = {"stderr": STDOUT, "stdout": PIPE, "check": True}
        for c in formatter:
            # Print the command being executed
            if verbose >= 2:
                text += f"\t$ {' '.join(c)}\n"
            cmd = sp_run(c, **run_args)
            tool_text = tool_text + cmd.stdout.decode("utf-8")

        if verbose >= 1 and tool_text:
            text = text + tool_text

        if check:
            # Run the diff command
            cmd = sp_run(["colordiff", "--color=yes", "--unified", path, output_path], **run_args)
        else:
            # Check if the file has changed and if so replace the original using python
            with open(path, "rb") as f:
                with open(output_path, "rb") as g:
                    if f.read() != g.read():
                        os.rename(output_path, path)

    except CalledProcessError as e:
        text = text + e.output.decode("utf-8").strip()
        success = False

    return text, success

def run(verbose, check, format_all, globs):
    # Use git to get all of the files that are committed to the repository or just the ones that are different to main
    if format_all:
        files = sp_run(["git", "ls-files"], stdout=PIPE, check=True).stdout.decode("utf-8").splitlines()
    else:
        files = (
            sp_run(["git", "diff", "--name-only", "origin/main"], stdout=PIPE, check=True)
            .stdout.decode("utf-8")
            .splitlines()
        )

    # Filter to a list containing only existing files (git diff can return deleted files)
    files = [f for f in files if os.path.isfile(f)]

    # Filter the files we found by any globs we are using
    if len(globs) != 0:

        def matches_glob(f):
            for g in globs:
                if fnmatch(f, g):
                    return True
            return False

        files = filter(matches_glob, files)

    success = True
    with multiprocessing.Pool(multiprocessing.cpu_count()) as pool:
        for r, s in pool.imap_unordered(partial(_do_format, verbose=verbose, check=check), files):
            sys.stdout.write(r)
            success = success and s

    sys.exit(0 if success else 1)


arg_parser = argparse.ArgumentParser(description="Format code in a repository")
arg_parser.add_argument(
    "--verbose",
    "-v",
    action="count",
    default=0,
    help="Print more information about what is happening",
)
arg_parser.add_argument(
    "--check",
    "-c",
    action="store_true",
    help="Check if the code is formatted correctly",
)
arg_parser.add_argument(
    "--all",
    "-a",
    action="store_true",
    help="Format all files in the repository",
)

arg_parser.add_argument(
    "globs",
    nargs="*",
    help="Globs to filter the files that are formatted",
)

args = arg_parser.parse_args()

run(args.verbose, args.check, args.all, args.globs)