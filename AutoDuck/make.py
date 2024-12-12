# Ported from a horrible, old .bat file, but this uses vswhere ...
import os
import subprocess
import sys

if __name__ == "__main__":
    # *sob* - we should kill .chm file support!
    # "hhc" is the "html help compiler" - part of some obscure SDK.
    if not os.environ.get("HHC"):
        os.environ["HHC"] = os.path.expandvars(
            r'"%ProgramFiles(x86)%\HTML Help Workshop\hhc.exe"'
        )

    # find nmake
    nmake = subprocess.check_output(
        [
            os.path.expandvars(
                r"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
            ),
            "-utf8",
            "-prerelease",
            "-find",
            r"VC\Tools\MSVC\**\nmake.exe",
        ],
        encoding="utf-8",
    ).splitlines()[-1]

    subprocess.run(
        [nmake, f'PYTHON="{sys.executable}"', "-E", "-f", "pywin32.mak"],
        cwd=os.path.dirname(sys.argv[0]),
        check=True,
    )
