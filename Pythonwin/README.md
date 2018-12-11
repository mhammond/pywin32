# Pythonwin Readme.

## Introduction

There are a few known problems (and probably lots of unknown ones!)

Pythonwin is a Windows only IDE and GUI framework for Python.  It has an integrated debugger, and a rich Python editing environment.

Pythonwin is implemented as a 'wrapper' for the Microsoft Foundation Class library. With it, you can use MFC in an interactive, interpreted environment, or write full blown stand-alone applications tightly coupled with the Windows environment. Over 30 MFC objects are exposed, including Common Controls, Property Pages/Sheets, Control/Toolbars, Threads, etc.

Pythonwin could almost be considered a sample program for the MFC UI environment. This Python UI environment can be embedded in almost any other application - such as OLE clients/servers, Netscape plugins, as a Macro language etc.

Recent changes can be found at the end of this document.

## Demos

There are many demos in the pywin\demos directory. To see a list of all the demos, run the program "pywin\demos\guidemo.py" from inside Pythonwin.

## Documentation

Almost all win32ui methods are document in the Pythonwin Help file. This is available from the Help Menu in the Pythonwin enviroment.

Below is a list of external Pythonwin specific documentation.

* There is separate documentation for the debugger
* Check out documentation on the Pythonwin GUI environment
* For a brief description of how to embed win32ui.pyd into your MFC application see the embedding win32ui documentation.
* There is some general documentation on the MFC Architecture and more specifically, document and view architecture.
* A Reference Manual of all available MFC functions etc. is released as a Windows Help file with the main Pythonwin release.
* Check out the demos, and the source code to the Pythonwin environment.

## Known Problems

* Some of the configuration options (eg, "Docking Windows") do not take affect until you restart Pythonwin.
* Under Windows 9x, any attempt to use Pythonwin features from an MS-DOS prompt seems to cause immediate death.  This means that you can not use the Pythonwin debugger for programs running under Python.exe on Windows 9x.  You should still be able to use the debugger normally from inside Pythonwin itself.
* Pythonwin will occasionally crash upon exiting under Windows 9x. The crash is in "user.exe" (old 16 bit code) which makes debugging difficult. It does not happen on Windows NT, so I'm stumped on this one. If someone could even just narrow it down to exactly what code will cause the crash at exit I would really appreciate it.
* Some of the menu items are always grey.  This functionality is simply not yet implemented in Pythonwin.

## Recent Changes

Fixed alot of the interactive window formatting problems.  Pythonwin now always prints output as it receives it - this should stop Pythonwin from looking like it has completely hung when infact it is just waiting for some code to finish.

Support for Scintilla's indentation guides, that gives a nice indication of the block structure.

New, improved color editor, using the Scintilla control by Neil Hodgson (see http://hare.net.au/~neilh/ScintillaTide.html). The debugger now requires use of this editor.

Much better printing support from Roger Burnham. Pythonwin itself still can't print anything, but the framework can (meaning some kind soul could now add the support to Pythonwin :-)

DDE support is complete.

Reference helpfile is far more complete.

Lots of new methods from Kleanthis Kleanthous.

Better tool-tip and region support
