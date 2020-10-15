#!/usr/bin/env python3
# Copyright 2017 Christoph Reiter
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

import sys
import argparse
import os
import json
import shutil
from collections import OrderedDict
import hashlib
import time
import subprocess
from concurrent.futures import ThreadPoolExecutor

from typing import List, Iterator, Tuple, Dict, Optional, Union, Collection


CacheEntry = Dict[str, Union[str, Collection[str]]]
CacheTuple = Tuple[str, CacheEntry]
Cache = Dict[str, CacheEntry]


def normalize_repo(repo: str) -> str:
    if repo.endswith(".git"):
        repo = repo.rsplit(".", 1)[0]
    return repo


def normalize_path(path: str) -> str:
    return path.replace("\\", "/")


def get_cache_key(pkgbuild_path: str) -> str:
    pkgbuild_path = os.path.abspath(pkgbuild_path)
    git_cwd = os.path.dirname(pkgbuild_path)
    git_path = os.path.relpath(pkgbuild_path, git_cwd)
    h = hashlib.new("SHA1")

    with open(pkgbuild_path, "rb") as f:
        h.update(f.read())

    fileinfo = subprocess.check_output(
        ["git", "ls-files", "-s", "--full-name", git_path],
        cwd=git_cwd).decode("utf-8").strip()
    h.update(normalize_path(fileinfo).encode("utf-8"))

    repo = subprocess.check_output(
        ["git", "ls-remote", "--get-url", "origin"],
        cwd=git_cwd).decode("utf-8").strip()
    repo = normalize_repo(repo)
    h.update(repo.encode("utf-8"))

    return h.hexdigest()


def get_srcinfo_for_pkgbuild(args: Tuple[str, str]) -> Optional[CacheTuple]:
    pkgbuild_path, mode = args
    pkgbuild_path = os.path.abspath(pkgbuild_path)
    git_cwd = os.path.dirname(pkgbuild_path)
    git_path = os.path.relpath(pkgbuild_path, git_cwd)
    key = get_cache_key(pkgbuild_path)

    bash = shutil.which("bash")
    if bash is None:
        print("ERROR: bash not found")
        return None

    print("Parsing %r" % pkgbuild_path)
    try:
        srcinfos = {}

        if mode == "mingw":
            for name in ["mingw32", "mingw64"]:
                env = os.environ.copy()
                env["MINGW_INSTALLS"] = name
                srcinfos[name] = subprocess.check_output(
                    [bash, "/usr/bin/makepkg-mingw",
                    "--printsrcinfo", "-p", git_path],
                    cwd=git_cwd,
                    env=env).decode("utf-8")
        else:
            srcinfos["msys"] = subprocess.check_output(
                [bash, "/usr/bin/makepkg",
                "--printsrcinfo", "-p", git_path],
                cwd=git_cwd).decode("utf-8")

        repo = subprocess.check_output(
            ["git", "ls-remote", "--get-url", "origin"],
            cwd=git_cwd).decode("utf-8").strip()
        repo = normalize_repo(repo)

        relpath = subprocess.check_output(
            ["git", "ls-files", "--full-name", git_path],
            cwd=git_cwd).decode("utf-8").strip()
        relpath = normalize_path(os.path.dirname(relpath))

        date = subprocess.check_output(
            ["git", "log", "-1", "--format=%aI", git_path],
            cwd=git_cwd).decode("utf-8").strip()

        meta = {"repo": repo, "path": relpath, "date": date, "srcinfo": srcinfos}
    except subprocess.CalledProcessError as e:
        print("ERROR: %s %s" % (pkgbuild_path, e.output.splitlines()))
        return None

    return (key, meta)


def iter_pkgbuild_paths(repo_path: str) -> Iterator[str]:
    repo_path = os.path.abspath(repo_path)
    print("Searching for PKGBUILD files in %s" % repo_path)
    for base, dirs, files in os.walk(repo_path):
        for f in files:
            if f == "PKGBUILD":
                # in case we find a PKGBUILD, don't go deeper
                del dirs[:]
                path = os.path.join(base, f)
                yield path


def get_srcinfo_from_cache(args: Tuple[str, Cache]) -> Tuple[str, Optional[CacheTuple]]:
    pkgbuild_path, cache = args
    key = get_cache_key(pkgbuild_path)
    if key in cache:
        return (pkgbuild_path, (key, cache[key]))
    else:
        return (pkgbuild_path, None)


def iter_srcinfo(repo_path: str, mode: str, cache: Cache) -> Iterator[Optional[CacheTuple]]:
    with ThreadPoolExecutor() as executor:
        to_parse: List[Tuple[str, str]] = []
        pool_iter = executor.map(
            get_srcinfo_from_cache, ((p, cache) for p in iter_pkgbuild_paths(repo_path)))
        for pkgbuild_path, srcinfo in pool_iter:
            if srcinfo is not None:
                yield srcinfo
            else:
                to_parse.append((pkgbuild_path, mode))

        print("Parsing PKGBUILD files...")
        for srcinfo in executor.map(get_srcinfo_for_pkgbuild, to_parse):
            yield srcinfo


def main(argv: List[str]) -> Optional[Union[int, str]]:
    parser = argparse.ArgumentParser(description="Create SRCINFOs for all packages in a repo", allow_abbrev=False)
    parser.add_argument('mode', choices=['msys', 'mingw'], help="The type of the repo")
    parser.add_argument("repo_path", help="The path to GIT repo")
    parser.add_argument("json_cache", help="The path to the json file used to fetch/store the results")
    parser.add_argument("--time-limit", action="store",
        type=int, dest="time_limit", default=0,
        help='time after which it will stop and save, 0 means no limit')
    args = parser.parse_args(argv[1:])

    t = time.monotonic()

    srcinfo_path = os.path.abspath(args.json_cache)
    cache: Cache = {}
    try:
        with open(srcinfo_path, "rb") as h:
            cache = json.loads(h.read())
    except FileNotFoundError:
        pass

    srcinfos = []
    for entry in iter_srcinfo(args.repo_path, args.mode, cache):
        if entry is None:
            continue
        srcinfos.append(entry)
        # So we stop before CI times out
        if args.time_limit and time.monotonic() - t > args.time_limit:
            print("time limit reached, stopping")
            break

    srcinfos_dict = OrderedDict(sorted(srcinfos))
    with open(srcinfo_path, "wb") as h:
        h.write(json.dumps(srcinfos_dict, indent=2).encode("utf-8"))

    return None


if __name__ == "__main__":
    sys.exit(main(sys.argv))
