This is a copy of Scintilla 4.4.6 used for Pythonwin.

Scintilla's home is <https://www.scintilla.org/>

Only the sources relevant to Scintilla under Pythonwin are
included (plus the Scintilla `License.txt` and `README`).
For the full set of Scintilla sources, including its documentation
and companion editor SciTE, see <https://www.scintilla.org/>.

When updating the Scintilla source, also update the copyright year
in Pythonwin/pywin/framework/app.py and regenerate
Pythonwin/pywin/scintilla/scintillacon.py by running
`nmake /f makefile_pythonwin scintillacon.py`.
(nmake will be found in your VS Build Tools, for instance:
`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.41.34120\bin\Hostx86\x86\nmake.exe`)
