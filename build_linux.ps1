if (-not $IsLinux) {
    Write-Error 'Linux required: use MSYS2 on Windows for native MinGW builds'
    exit 1
}

uv pip install build setuptools

$Msys2RepoUrl = 'https://repo.msys2.org/mingw/mingw64/'

$Msys2PythonTmpDir = '/tmp/msys2-python'
$PythonVer = uv run python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"
$WinPythonLib = "$Msys2PythonTmpDir/mingw64/lib"
$WinPythonInclude = "$Msys2PythonTmpDir/mingw64/include/python${PythonVer}"

if (-not (Test-Path "$WinPythonInclude/Python.h")) {
    # Find the correct Python package on the MSYS2 repo
    $Html = Invoke-WebRequest $Msys2RepoUrl -UseBasicParsing
    $PkgName = ([regex]::Matches($Html.Content, 'href="([^"]*python-' + [regex]::Escape($PythonVer) + '\.[^"]*)"')
        | Where-Object { $_.Groups[1].Value -notmatch 'docs|test|tkinter|pyc|pip|setuptools' }
        | Select-Object -First 1).Groups[1].Value

    if (-not $PkgName) {
        Write-Error "No MSYS2 package found for Python $PythonVer at ${Msys2RepoUrl}. Run 'uv python pin <ver>' to match an available MSYS2 Python version."
        exit 1
    }

    Write-Host "Downloading $PkgName ..."
    New-Item -ItemType Directory -Force $Msys2PythonTmpDir | Out-Null
    Invoke-WebRequest "${Msys2RepoUrl}${PkgName}" -OutFile "$Msys2PythonTmpDir/python.pkg.tar.zst"
    bash -c "cd $Msys2PythonTmpDir && tar -x --zstd -f python.pkg.tar.zst mingw64/lib/libpython${PythonVer}.dll.a mingw64/include/python${PythonVer}/"
}
else {
    Write-Host "$WinPythonInclude already extracted, skipping download"
}

# Distros mingw-w64 headers lag behind upstream (missing constants pywin32 uses),
# so use the same headers MSYS2 ships, like the native MinGW CI job does.
$Msys2HeadersTmpDir = '/tmp/msys2-headers'
$WinHeadersInclude = "$Msys2HeadersTmpDir/mingw64/include"

if (-not (Test-Path "$WinHeadersInclude/windows.h") -or -not (Test-Path "$WinHeadersInclude/pthread_time.h")) {
    $Html = Invoke-WebRequest $Msys2RepoUrl -UseBasicParsing
    New-Item -ItemType Directory -Force $Msys2HeadersTmpDir | Out-Null
    # Distros ship winpthreads headers alongside the others (time.h includes pthread_time.h), MSYS2 splits them
    foreach ($Pkg in 'headers', 'winpthreads') {
        # Pick the newest snapshot, versions look like 14.0.0.r569.g<commit>
        $PkgName = ([regex]::Matches($Html.Content, "href=""(mingw-w64-x86_64-$Pkg-(\d+\.\d+\.\d+)\.r(\d+)\.[^""]*\.pkg\.tar\.zst)""")
            | Sort-Object { [version]"$($_.Groups[2].Value).$($_.Groups[3].Value)" }
            | Select-Object -Last 1).Groups[1].Value

        if (-not $PkgName) {
            Write-Error "No MSYS2 mingw-w64 $Pkg package found at ${Msys2RepoUrl}"
            exit 1
        }

        Write-Host "Downloading $PkgName ..."
        Invoke-WebRequest "${Msys2RepoUrl}${PkgName}" -OutFile "$Msys2HeadersTmpDir/$Pkg.pkg.tar.zst"
        bash -c "cd $Msys2HeadersTmpDir && tar -x --zstd -f $Pkg.pkg.tar.zst mingw64/include/"
    }
}
else {
    Write-Host "$WinHeadersInclude already extracted, skipping download"
}


# Swap the distro mingw-w64 headers for the MSYS2 ones in the compiler's own search list.
# Prepending with -I mixes both trees: libstdc++'s #include_next <stdlib.h> skips past it.
function Get-SystemIncludeFlags([string]$Compiler, [string]$Language) {
    $Lines = '' | & $Compiler "-x$Language" -E -v - 2>&1 | ForEach-Object ToString
    $Start = [array]::IndexOf($Lines, '#include <...> search starts here:') + 1
    $End = [array]::IndexOf($Lines, 'End of search list.') - 1
    $Dirs = $Lines[$Start..$End] | ForEach-Object { [IO.Path]::GetFullPath($_.Trim()) }
    $Native = $Dirs | Where-Object { $_ -like '*/x86_64-w64-mingw32/include' }
    if (-not $Native) {
        Write-Error "Couldn't find the mingw-w64 system header dir in $Compiler's search list: $Dirs"
        exit 1
    }
    $Dirs = $Dirs | ForEach-Object { if ($_ -eq $Native) { $WinHeadersInclude } else { $_ } }
    return '-nostdinc ' + (($Dirs | ForEach-Object { "-isystem $_" }) -join ' ')
}

$Env:CC = 'x86_64-w64-mingw32-gcc ' + (Get-SystemIncludeFlags 'x86_64-w64-mingw32-gcc' 'c')
$Env:CXX = 'x86_64-w64-mingw32-g++ ' + (Get-SystemIncludeFlags 'x86_64-w64-mingw32-g++' 'c++')
$Env:AR = 'x86_64-w64-mingw32-ar'
$Env:RANLIB = 'x86_64-w64-mingw32-ranlib'
$Env:LDSHARED = 'x86_64-w64-mingw32-gcc -shared'

$Env:WIN_PYTHON_LIB = $WinPythonLib
$Env:WIN_PYTHON_INC = $WinPythonInclude
$Env:LDFLAGS = '-Wl,--allow-shlib-undefined'

$Env:_PYTHON_HOST_PLATFORM = 'win-amd64'

$Env:RC = 'x86_64-w64-mingw32-windres'
$Env:WINDRES = 'x86_64-w64-mingw32-windres'
$Env:WINDMC = 'x86_64-w64-mingw32-windmc'
$Env:SWIG = 'SWIG/swig.sh' | Resolve-Path

# $WinProgramFilesSearchRoots = @($Env:HOME, '/mnt', '/media', '/run/media') | Where-Object { Test-Path $_ }
# $Env:MFC_INCLUDE_PATH = Get-ChildItem -Path $WinProgramFilesSearchRoots -Recurse -Directory -Filter 'Program Files (x86)' -ErrorAction SilentlyContinue |
#     ForEach-Object { Get-Item "$($_.FullName)/Microsoft Visual Studio/*/BuildTools/VC/Tools/MSVC/*/atlmfc/include" -ErrorAction SilentlyContinue } |
#     Select-Object -First 1 -ExpandProperty FullName

uv run python -m build --wheel --no-isolation --config-setting=--build-option="build_ext --plat-name=win-amd64"
