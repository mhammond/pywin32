# pywin32

[![CI](https://github.com/mhammond/pywin32/workflows/CI/badge.svg)](https://github.com/mhammond/pywin32/actions?query=workflow%3ACI)
[![PyPI - Version](https://img.shields.io/pypi/v/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Downloads](https://img.shields.io/pypi/dm/pywin32.svg)](https://pypi.org/project/pywin32)
[![License - PSF-2.0](https://img.shields.io/badge/license-PSF--2.0-9400d3.svg)](https://spdx.org/licenses/PSF-2.0.html)

-----

This is the readme for the Python for Win32 (pywin32) extensions, which provides access to many of the Windows APIs from Python.

See [CHANGES.txt](https://github.com/mhammond/pywin32/blob/master/CHANGES.txt) for recent notable changes.

Only Python 3 is supported. If you want Python 2 support, you want build `228`.

## Docs

The docs are a long and sad story, but [there's now an online version](https://mhammond.github.io/pywin32/)
of the helpfile that ships with the installers (thanks [@ofek](https://github.com/mhammond/pywin32/pull/1774)!).
Lots of that is very old, but some is auto-generated and current. Would love help untangling the docs!

## Support

Feel free to [open issues](https://github.com/mhammond/pywin32/issues) for
all bugs (or suspected bugs) in pywin32. [pull-requests](https://github.com/mhammond/pywin32/pulls)
for all bugs or features are also welcome.

However, please **do not open github issues for general support requests**, or
for problems or questions using the modules in this package - they will be
closed. For such issues, please email the
[python-win32 mailing list](http://mail.python.org/mailman/listinfo/python-win32) -
note that you must be subscribed to the list before posting.

## Binaries
[Binary releases are deprecated.](https://mhammond.github.io/pywin32_installers.html)
While they are still provided, [find them here](https://github.com/mhammond/pywin32/releases)

## Installing via PIP

You should install pywin32 via pip - eg,
> python -m pip install --upgrade pywin32

If you encounter any problems when upgrading (eg, "module not found" errors or similar), you
should execute:

> python Scripts/pywin32_postinstall.py -install

This will make some small attempts to cleanup older conflicting installs.

Note that if you want to use pywin32 for "system wide" features, such as
registering COM objects or implementing Windows Services, then you must run
that command from an elevated (ie, "Run as Administrator) command prompt.

For unreleased changes, you can download builds made by [github actions](https://github.com/mhammond/pywin32/actions/) -
choose any "workflow" from the `main` branch and download its "artifacts")

### `The specified procedure could not be found` / `Entry-point not found` Errors?
A very common report is that people install pywin32, but many imports fail with errors
similar to the above.

In almost all cases, this tends to mean there are other pywin32 DLLs installed in your system,
but in a different location than the new ones. This sometimes happens in environments that
come with pywin32 pre-shipped (eg, anaconda?).

The possible solutions are:

* Run the "post_install" script documented above.

* Otherwise, find and remove all other copies of `pywintypesXX.dll` and `pythoncomXX.dll`
  (where `XX` is the Python version - eg, "39")

### Running as a Windows Service

Modern Python installers do not, by default, install Python in a way that is suitable for
running as a service, particularly for other users.

* Ensure Python is installed in a location where the user running the service has
  access to the installation and is able to load `pywintypesXX.dll` and `pythonXX.dll`.

* Manually copy `pythonservice.exe` from the `site-packages/win32` directory to
  the same place as these DLLs.

## Building from source

Building from source has been simplified recently - you just need Visual Studio
and the Windows 10 SDK installed (the free compilers probably work too, but
haven't been tested - let me know your experiences!)

`setup.py` is a standard distutils build script.  You probably want:

> python setup.py install

or

> python setup.py --help

You can run `setup.py` without any arguments to see
specific information about dependencies.  A vanilla MSVC installation should
be able to build most extensions and list any extensions that could not be
built due to missing libraries - if the build actually fails with your
configuration, please [open an issue](https://github.com/mhammond/pywin32/issues).

## Release process

The following steps are performed when making a new release - this is mainly
to form a checklist so mhammond doesn't forget what to do :)

* Ensure CHANGES.txt has everything worth noting, commit it.

* Update setup.py with the new build number.

* Execute build.bat, wait forever, test the artifacts.

* Upload .whl artifacts to pypi - we do this before pushing the tag because they might be
  rejected for an invalid `README.md`. Done via `py -3.5 -m twine upload dist/*XXX*.whl`.

* Commit setup.py (so the new build number is in the repo), create a new git tag

* Upload the .exe installers to github.

* Update setup.py with the new build number + ".1" (eg, 123.1), to ensure
  future test builds aren't mistaken for the real release.

* Make sure everything is pushed to github, including the tag (ie,
  `git push --tags`)

* Send mail to python-win32
