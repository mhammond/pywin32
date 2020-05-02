# pywin32

[![CI](https://github.com/mhammond/pywin32/workflows/CI/badge.svg)](https://github.com/mhammond/pywin32/actions?query=workflow%3ACI)
[![PyPI - Version](https://img.shields.io/pypi/v/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Downloads](https://img.shields.io/pypi/dm/pywin32.svg)](https://pypi.org/project/pywin32)
[![License - PSF-2.0](https://img.shields.io/badge/license-PSF--2.0-9400d3.svg)](https://spdx.org/licenses/PSF-2.0.html)

-----

This is the readme for the Python for Win32 (pywin32) extensions, which provides access to many of the Windows APIs from Python.

See [CHANGES.txt](https://github.com/mhammond/pywin32/blob/master/CHANGES.txt) for recent notable changes.

Build 228 is the last build supporting Python 2, and as part of this transition,
all code in the repository is now using Python 3 syntax.
To highlight and celebrate this change, build 228 is the last numbered 2XX - the
following build numbers start at 300.

In other words, there is no build 229 - the build numbers jump from 228 to 300.

As of build 222, pywin32 has a new home at [github](https://github.com/mhammond/pywin32).
You can find build 221 and later on github and older versions can be found on
the old project home at [sourceforge](https://sourceforge.net/projects/pywin32/)

A special shout-out to @xoviat who provided enormous help with the github move!

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
By far the easiest way to use pywin32 is to grab binaries from the [most recent release](https://github.com/mhammond/pywin32/releases)

## Installing via PIP

Note that PIP support is experimental.

You can install pywin32 via pip:
> pip install pywin32

Note that if you want to use pywin32 for "system wide" features, such as
registering COM objects or implementing Windows Services, then you must run
the following command from an elevated command prompt:

> python Scripts/pywin32_postinstall.py -install

## Building from source
Building from source is extremely complicated due to the fact we support building
old versions of Python using old versions of Windows SDKs. If you just want to
build the most recent version, you can probably get away with installing th
same MSVC version used to build that version of Python, grabbing a recent
Windows SDK and running `setup.py`

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

* Commit setup.py (so the new build number is in the repo), create a new git tag

* Update setup.py with the new build number + ".1" (eg, 123.1), to ensure
  future test builds aren't mistaken for the real release.

* Make sure everything is pushed to github, including the tag (ie,
  `git push --tags`)

* Upload the .exe installers to github (using the web UI), the .whl files to
  pypi (using `py -3.5 -m twine upload dist/*XXX*.whl` where `XXX` is the build
  number).

* Send mail to python-win32
