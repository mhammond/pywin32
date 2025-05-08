import os
import platform
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
).strip()
components_to_add = (
    ["Microsoft.VisualStudio.Component.VC.14.29.16.11.ATL.ARM64"]
    if platform.machine() == "ARM64"
    else ["Microsoft.VisualStudio.Component.VC.14.29.16.11.ATL"]
)
args = (
    "vs_installer.exe",
    "modify",
    "--installPath",
    f'"{vs_install_path}"',
    *[f"--add {component}" for component in components_to_add],
    "--quiet",
    "--norestart",
    "--nocache",
)

# Should be run twice for some reason
check_call(args)
check_call(args)
