"""Downloads the matching ARM64 Python libs for the running version of Python.

These libs are used for cross-compiling to ARM64. The directory is passed as
the only argument to this script, and must also be provided to setuptools
by specifying the "-L <dir>" argument to the build_ext command.
"""

import pathlib
import sys
from urllib.request import urlretrieve
from zipfile import ZipFile

try:
    dest = pathlib.Path(sys.argv[1])
except IndexError:
    print("ERROR: Requires destination directory as sole argument")
    sys.exit(1)

dest = dest.absolute()
dest.mkdir(parents=True, exist_ok=True)

VERSION = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
if sys.version_info.releaselevel == "alpha":
    VERSION += f"-a{sys.version_info.serial}"  # pyright: ignore[reportConstantRedefinition]
if sys.version_info.releaselevel == "beta":
    VERSION += f"-b{sys.version_info.serial}"  # pyright: ignore[reportConstantRedefinition]
if sys.version_info.releaselevel == "candidate":
    VERSION += f"-rc{sys.version_info.serial}"  # pyright: ignore[reportConstantRedefinition]

URL = f"https://www.nuget.org/api/v2/package/pythonarm64/{VERSION}"
DEST_PATH = dest / f"pythonarm64.{VERSION}.zip"

if DEST_PATH.is_file():
    print("Skipping download because", DEST_PATH, "exists")
else:
    print("Downloading", URL)
    urlretrieve(URL, DEST_PATH)
    print("Downloaded", DEST_PATH)

with ZipFile(DEST_PATH, "r") as zf:
    for name in zf.namelist():
        zip_path = pathlib.PurePath(name)
        if zip_path.parts[:2] == ("tools", "libs"):
            out_path = dest.joinpath(*zip_path.parts[2:])
            print("-", zip_path, "->", out_path)
            data = zf.read(name)
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_bytes(data)

print("Download and extract complete")
