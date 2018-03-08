# pywin32

This is the readme for the Python for Win32 (pywin32) extensions, which provides access to many of the Windows APIs from Python.

See [CHANGES.txt](https://github.com/mhammond/pywin32/blob/master/CHANGES.txt) for recent notable changes.

Note that as of build 222, pywin32 has a new home at [github](https://github.com/mhammond/pywin32).
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
Windows SDK, setting the `MSSDK` environment variable to point at the root of
the SDK, and running `setup.py` (or `setup3.py` for Python 3.x versions)

`setup.py` is a standard distutils build script.  You probably want:

> python setup.py install

or

> python setup.py --help

You can run `setup.py` without any arguments to see
specific information about dependencies.  A vanilla MSVC installation should
be able to build most extensions and list any extensions that could not be
built due to missing libraries - if the build actually fails with your
configuration, please [open an issue](https://github.com/mhammond/pywin32/issues).
