/*
Documentation File for Pythonwin

@doc

@topic Keyboard Bindings|Pythonwin has a new, flexible keyboard
binding mechanism. Bindings (and even code) can be defined in a
configuration file, stored in the pywin directory. 

<nl>Many bindings are still builtin to Python using Window's accelerators -
see the Pythonwin menus for the specific keyboard shortcuts.

<nl>The default configuration file is named default.cfg. You can view
this file with a text editor (eg, Pythonwin) This file has extensive
comments, including how to create your own configuration based on the
default. An example configuration which provides keyboard bindings
similar to IDLE exists in IDLE.cfg, and this makes a good example of a
custom configuration.

<nl>Please see default.cfg for a complete list, but the default bindings
provided with Pythonwin are:
@flagh Common Keystrokes|Description
@flag Alt+Q|Reformat the current paragraph/comment block.  Note this does NOT reformat code correctly - use only within comment blocks!
@flag Ctrl+W|Toggle view whitespace.
@flag Alt+/|Expand the word at the cursor.  Eg, pressing "st\<Alt+/\>"
will complete based on all words in the current file - eg, "string"
would be likely to result assuming the code has an "import string"
statement.  Pressing the key again expands to the next match.
@flag .|Auto expand the attribute.  Eg, typing "string." will display a listbox with the contents of the string module.
@flag Alt+I|Toggle focus to/from the interactive window.
@flag Ctrl+Keypad+Plus|Zoom-in for the current window (builtin to Scintilla)
@flag Ctrl+Keypad+Minus|Zoom-out for the current window (builtin to Scintilla)

@flagh Editor Specific Keystrokes|Description
@flag F2|Move to the next bookmark.
@flag Ctrl+F2|Add or remove a bookmark on the current line.
@flag Ctrl+G|Prompt for and goto a specific line number.
@flag Alt+B|Adds a simple comment banner at the current location.
@flag Alt+3|Block comment the selected region.
@flag Shift+Alt+3|Uncomment the selected region.
@flag Alt+4|Uncomment the selected region (IDLE default keystroke)
@flag Alt+5|Tabify the selected region.
<nl>
@flag Alt+6|Untabify the selected region.
@flag BackSpace|Remove one indent to the left.
@flag Ctrl+T|Toggle the use of tabs for the current file (after confirmation)
@flag Alt+U|Change the indent width for the current file.
@flag Enter|Insert a newline and indent.
@flag Tab|Insert an indent, or perform a block indent if a selection
exists.
@flag Shift-Tab|Block dedent the selection
<nl>
@flag Keypad-Plus|If the current line is a collapsed fold, expand it (see <t Folding>)
@flag Alt-Keypad-Plus|Expand all folds in the current file (see <t Folding>)
@flag Keypad-Minus|If the current line is an expanded fold, collapse it (see <t Folding>)
@flag Alt-Keypad-Minus|Collapse all folds in the current file. regardless of how deep the fold becomes. (see <t Folding>)
@flag Keypad-Multiply|Expand or collapse all top-level folds in the current file.  No second level or deeper folds are changed.
If the first fold in the file is collapsed, all top-level folds are opened.  Otherwise, all top-level folds are collapsed (see <t Folding>)

@flagh Debugger Keystrokes|Description
@flag F9|Toggle breakpoint
@flag F5|Run (ie, go)
@flag Shift+F5|Stop debugging
@flag F11|Single step into functions
@flag F10|Step over functions
@flag Shift+F11|Step out of the current function

@flagh Interactive Window Specific Keystrokes|Description
@flag Ctrl+Up|Recall the previous command in the history list.
@flag Ctrl+Down|Recall the next command in the history list.

@topic Folding|
Thanks to Scintilla (http://www.scintilla.org), Pythonwin supports
source code folding.  Folding is the ability to collapse sections of
your source-code into a single line, making it easier to navigate
around large files.  Any Python statement which introduces a new block
can be folded either by clicking on the indicator in the folding
margin (if enabled via the View-\>Options-\>Editor dialog), by
selecting one of the folding keystrokes (see <t Keyboard Bindings>, or
by using View->Folding menu.)  <nl>

All find/replace or 'goto linenumber' functions work correctly when
code is folded - the code is simply unfolded if necessary before the
relevant operation.  <nl>

You may configure Pythonwin so that all files have their top-levels
folded when opened.  Only the first level folds are collapsed using
this function, so expanding the top-level fold reveals the entire
class/method that was folded.  Alternatively, you can use the
Keypad-Multiply key to toggle the first level folds for the entire
file at any time.  <nl>


@topic Source Safe Integration|
Note you will need to restart Pythonwin for this option to take effect.

Before using the VSS integration, you must create a "mssccprj.scc" file
in the directory, or a parent directory, of the files you wish to
integrate. There are no limits on how many of these files exist. This is
the same name and format as VB uses for VSS integration - a Windows INI
file. 

This file must have a section [Python] with entry "Project=ProjectName".
The project name is the name of the VSS project used to check the out
the file. If the .scc file is in a parent directory, the correct
relative VSS path is built - so if your file system matches your VSS
structure, you only need a single .scc file in the VSS "root" directory.

<nl>For example, assuming you have the file c:\src\mssccprj.scc with the contents:
<nl>[Python]
<nl>Project=OurProject
<nl>-eof-
<nl>The file c:\src\source1.py will be checked out from project OurProject,
c:\src\sub\source2.py will be checked out from project OurProject\sub,
etc.


*/
