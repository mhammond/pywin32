This is the readme for the Python for Win32 (pywin32) extensions source code.

See [CHANGES.md] for recent changes.

`setup.py` is a standard distutils build script.  You probably want to:


```powershell
python setup.py install
```

or

```powershell
python setup.py --help
```

These extensions require the same version of MSVC as used for the 
corresponding version of Python itself.  Some extensions require a recent 
"Platform SDK"  from Microsoft, and in general, the latest service packs 
should be  installed, but run `setup.py` without any arguments to see 
specific information about dependencies.  A vanilla MSVC installation should 
be able to build most extensions and list any extensions that could not be 
built due to missing libraries - if the build actually fails with your 
configuration, please log a bug via https://github.com/mhammond/pywin32/issues/new.
