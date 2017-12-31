# pywin32

This is the readme for the Python for Win32 (pywin32) extensions, which provides access to many of the Windows APIs from Python.

See [CHANGES.txt](https://github.com/mhammond/pywin32/blob/master/CHANGES.txt) for recent notable changes.

## Binaries
By far the easiest way to use pywin32 is to grab binaries from the [most recent release](https://github.com/mhammond/pywin32/releases)

Feel free to [open issues](https://github.com/mhammond/pywin32/issues) or [pull-requests](https://github.com/mhammond/pywin32/pulls)

## Building from source
Building from source is extremely complicated due to the fact we support building old versions of Python using old versions of Windows SDKs. If you just want to build the most recent version, you can probably get away with installing the same MSVC version used to build that version of Python, grabbing a recent Windows SDK, setting the `MSSDK` environment variable to point at the root of the SDK, and running `setup.py` (or `setup3.py` for Python 3.x versions)

'setup.py' is a standard distutils build script.  You probably want to:

> % setup.py install

or

> % setup.py --help

You can run 'setup.py' without any arguments to see 
specific information about dependencies.  A vanilla MSVC installation should 
be able to build most extensions and list any extensions that could not be 
built due to missing libraries - if the build actually fails with your 
configuration, please [open an issue](https://github.com/mhammond/pywin32/issues).
