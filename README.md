# pywin32

[![CI](https://github.com/mhammond/pywin32/workflows/CI/badge.svg)](https://github.com/mhammond/pywin32/actions?query=workflow%3ACI)
[![PyPI - Version](https://img.shields.io/pypi/v/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/pywin32.svg)](https://pypi.org/project/pywin32)
[![PyPI - Downloads](https://img.shields.io/pypi/dm/pywin32.svg)](https://pypi.org/project/pywin32)
[![License - PSF-2.0](https://img.shields.io/badge/license-PSF--2.0-9400d3.svg)](https://spdx.org/licenses/PSF-2.0.html)

-----

This is the readme for the Python for Win32 (pywin32) extensions, which provides access to many of the Windows APIs from Python.

See [CHANGES.txt](https://github.com/mhammond/pywin32/blob/master/CHANGES.txt) for recent notable changes.

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

```shell
python -m pip install --upgrade pywin32
```

There is a post-install script (see below) which should *not* be run inside virtual environments;
it should only be run in "global" installs.

For unreleased changes, you can download builds made by [github actions](https://github.com/mhammond/pywin32/actions/) -
choose any "workflow" from the `main` branch and download its "artifacts")

### Installing globally

Outside of a virtual environment you might want to install COM objects, services, etc. You can do
this by executing:

```shell
python Scripts/pywin32_postinstall.py -install
```

From the root of your Python installation.

If you do this with normal permissions it will be global for your user (a few files will be
copied to the root of your Python install and some changes made to HKCU). If you execute this from
an elevated process, it will be global for the machine (files will be copied to System32, HKLM
will be changed, etc)

### Running as a Windows Service

To run as a service, you probably want to install pywin32 globally from an elevated
command prompt - see above.

You also need to ensure Python is installed in a location where the user running
the service has access to the installation and is able to load `pywintypesXX.dll` and `pythonXX.dll`. In particular, the `LocalSystem` account typically will not have access
to your local `%USER%` directory structure.

## Troubleshooting

If you encounter any problems when upgrading like the following:

```text
The specified procedure could not be found
Entry-point not found
```

It usually means one of 2 things:

* You've upgraded an install where the post-install script has previously run.
So you should run it again:

    ```shell
    python Scripts/pywin32_postinstall.py -install
    ```

    This will make some small attempts to cleanup older conflicting installs.

* There are other pywin32 DLLs installed in your system,
but in a different location than the new ones. This sometimes happens in environments that
come with pywin32 pre-shipped (eg, anaconda?).

  The possible solutions here are:

  * Run the "post_install" script documented above.
  * Otherwise, find and remove all other copies of `pywintypesXX.dll` and `pythoncomXX.dll`
  (where `XX` is the Python version - eg, "39")

## Building from source

Install Visual Studio 2019 (later probably works, but options might be different),
select "Desktop Development with C++", then the following options:

* Windows 10 SDK (latest offered I guess? At time of writing, 10.0.18362)
* "C++ for MFC for ..."
* ARM build tools if necessary.

(the free compilers probably work too, but haven't been tested - let me know your experiences!)

`setup.py` is a standard distutils build script, so you probably want:

```shell
python setup.py install
```

or

```shell
python setup.py --help
```

Some modules need obscure SDKs to build - `setup.py` should succeed, gracefully
telling you why it failed to build them - if the build actually fails with your
configuration, please [open an issue](https://github.com/mhammond/pywin32/issues).

## Release process

The following steps are performed when making a new release - this is mainly
to form a checklist so @mhammond doesn't forget what to do :)

* Ensure CHANGES.txt has everything worth noting. Update the header to reflect
  the about-to-be released build and date, commit it.

* Update setup.py with the new build number.

* Execute `make.bat`, wait forever, test the artifacts.

* Upload .whl artifacts to pypi - we do this before pushing the tag because they might be
  rejected for an invalid `README.md`. Done via `py -3.? -m twine upload dist/*XXX*.whl`.

* Commit setup.py (so the new build number is in the repo), create a new git tag

* Upload the .exe installers to github.

* Update setup.py with the new build number + ".1" (eg, 123.1), to ensure
  future test builds aren't mistaken for the real release.

* Make sure everything is pushed to github, including the tag (ie,
  `git push --tags`)

* Send mail to python-win32
