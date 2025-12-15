# See https://github.com/actions/runner-images/issues/9701
# Adapted from https://github.com/actions/runner-images/issues/9873#issuecomment-2139288682

import os
import platform
from itertools import chain
from subprocess import check_call, check_output

os.chdir("C:/Program Files (x86)/Microsoft Visual Studio/Installer")
vs_install_path = check_output(
    (
        "vswhere.exe",
        "-latest",
        "-products",
        "*",
        "-requires",
        "Microsoft.Component.MSBuild",
        "-property",
        "installationPath",
    ),
    text=True,
    shell=True,
).strip()
components_to_add = [
    "Microsoft.VisualStudio.Component.VC.14.29.16.11.ATL.ARM64"
    if platform.machine() == "ARM64"
    else "Microsoft.VisualStudio.Component.VC.14.29.16.11.ATL"
]
args = (
    "vs_installer.exe",
    "modify",
    "--installPath",
    vs_install_path,
    *chain.from_iterable([("--add", component) for component in components_to_add]),
    "--quiet",
    "--norestart",
)
print(*args)

# Should be run twice for some reason
print("First run...")
check_call(args)
print("Second run...")
check_call(args)
