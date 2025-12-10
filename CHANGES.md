# Notable changes in recent builds

Maintained by hand, so what's "notable" is subjective! Contributors are
encouraged to add entries for their work.

All changes can be found via git - eg, for all changes after a build:

  ```shell
  git log -rb3xx:
  ```

or
  > <https://github.com/mhammond/pywin32/compare/b3xx...main>

As of build 305, installation .exe files have been deprecated; see
<https://mhammond.github.io/pywin32_installers.html>.

Coming in build 312, as yet unreleased
--------------------------------------

* Fixed missing version stamp on built `.dll` and `.exe` files (mhammond#2647, [@Avasam][Avasam])
* Removed considerations for Windows 2000 and Windows Vista (mhammond#2667, [@Avasam][Avasam])
  * This mostly updates obsolete documentation and tests
* Removed considerations for Windows 95/98/ME (mhammond#2400, [@Avasam][Avasam])
  This removes the following constants:
  * `win32con.FILE_ATTRIBUTE_ATOMIC_WRITE`
  * `win32con.FILE_ATTRIBUTE_XACTION_WRITE`
* Removed considerations for MFC < 9 (VS 2008) (mhammond#2669, [@Avasam][Avasam])
  * This removes the unusable `PyCSliderCtrl.VerifyPos` method
* Dropped support for Python 3.8 (mhammond#2413, [@Avasam][Avasam])
  * Note that whilst pywin32 hasn't explicitly dropped support for Windows 7 / Windows Server 2008,
    Python 3.8 was the last official CPython version to support it.

Build 311, released 2025/07/14
------------------------------

* Fix use-after-free in CertDeleteCertificateFromStore (mhammond#2637)
* Better support for COM identifiers with non-ascii characters (mhammond#2632)
* pywin32's own warnings will now refer to the caller, rather than to the internal source of warning itself (mhammond#2594, [@Avasam][Avasam])
* Fixed a regression that broke special `__dunder__` methods with CoClass. (mhammond#1870, mhammond#2493, [@Avasam][Avasam], [@geppi][geppi])
* Fixed `TypeError: cannot unpack non-iterable NoneType object` when registering an axscript client `ScriptItem` (mhammond#2513, [@Avasam][Avasam])
* Fixed a memory leak when SafeArrays are used as out parameters ([@the-snork][the-snork])
* Fixed dispatch handling for properties ([@the-snork][the-snork])
* Resolved a handful of deprecation warnings (mhammond#2567, mhammond#2576, [@Avasam][Avasam])
* The following classes now produce a valid `eval` string representation when calling `repr`: (mhammond#2573, [@Avasam][Avasam])
  * `pywin.tools.browser.HLIPythonObject`
  * `win32com.server.exception.COMException`
  * `win32comext.axscript.client.error.AXScriptException`
  * `win32comext.axscript.client.pyscript.NamedScriptAttribute`
* Added initial `DECIMAL/VT_DECIMAL` support (mhammond#1501, [@gesslerpd][gesslerpd])

Build 310, released 2025/03/16
------------------------------

* Fixed a regression where `win32com.client.DispatchWithEvents` and `win32com.client.WithEvents` would throw a `TypeError` on the second call (mhammond#2491, [@Avasam][Avasam])
* Fixed regression causing `win32com.shell.shell` to be missing a number of `IID`s. (mhammond#2487, [@Avasam][Avasam])
* As part of the above, Windows 7 is now minimum supported.

Build 309, released 2025/03/09
------------------------------

### pywin32

* Fixed Access Violation crashes in 3.12 by moving `PyWInObject_Free*` methods so GIL is acquired (mhammond#2467, [@Mscht][Mscht])
* Added support for relative path for `pywin32_postinstall`'s `-destination` argument (mhammond#2454, [@Avasam][Avasam])
* The postinstall script is now available as a console script. You can invoke it in one of two new methods: (mhammond#2408, [@Avasam][Avasam])
  1. `python -m pywin32_postinstall -install` (recommended)
  2. `pywin32_postinstall -install` (shorter but you don't have control over which python environment is used)
* Changed the implementation of 'com_record' to a subclassable Python type (mhammond#2437, mhammond#2361, [@geppi][geppi])
* Removed param `hIcon` from `win32comext.shell.ShellExecuteEx`. It was unusable since Windows Vista (mhammond#2423, [@Avasam][Avasam])
* Fixed `nbios.NCBStruct` packing (mhammond#2406, [@Avasam][Avasam])
* Restored axdebug builds on Python 3.10 (mhammond#2416, [@Avasam][Avasam])
* Fix for Python 3.12 interpreter crashes when accessing a COM Record field (mhammond#2415, [@geppi][geppi])
* Pythonwin: Bumped Scintilla from 1.77 to 4.4.6. The full changelog can be found here: <https://www.scintilla.org/ScintillaHistory.html>
* Fixed Pythonwin's editor failing due to invalid regex import (mhammond#2419, [@Avasam][Avasam])
* Last error wrongly set by some modules (mhammond#2302, [@CristiFati][CristiFati])
* Dropped support for Python 3.7 (mhammond#2207, [@Avasam][Avasam])
* Implement the creation of SAFEARRAY(VT_RECORD) from a sequence of COM Records (mhammond#2317, [@geppi][geppi])
* Implement record pointers as [in, out] method parameters of a Dispatch Interface (mhammond#2304, mhammond#2310, [@geppi][geppi])
* Fix memory leak converting to PyObject from some SAFEARRAY elements (mhammond#2316)
* Fix bug where makepy support was unnecessarily generated (mhammond#2354, mhammond#2353, [@geppi][geppi])
* Fail sooner on invalid `win32timezone.TimeZoneInfo` creation (mhammond#2338, [@Avasam][Avasam])
* Removed temporary `win32com.server.policy` reexports hack (mhammond#2344, [@Avasam][Avasam])
  Import `DispatcherWin32trace` and `DispatcherTrace` from `win32com.server.dispatcher` instead.
* Fixed `win32timezone.TimeZoneInfo` initialization from a `[DYNAMIC_]TIME_ZONE_INFORMATION` (mhammond#2339, [@Avasam][Avasam])
* Added runtime deprecation warning of `win2kras`, use `win32ras` instead (mhammond#2356, [@Avasam][Avasam])
* Improved handling of dict iterations and fallbacks (removes Python 2 support code, small general speed improvement) (mhammond#2332, mhammond#2330, [@Avasam][Avasam])
* Fixed accidentally trying to raise an undefined name instead of an `Exception` in `Pythonwin/pywin/debugger/debugger.py` (mhammond#2326, [@Avasam][Avasam])
* Fixed PythonService DoLogMessage raising fatal GIL lock error (mhammond#2426, JacobNolan1)
* Fixed and improved the following demos: `ddeclient`, `ddeserver`, `EvtSubscribe_push`, `openGLDemo`, `guidemo`, `ocxserialtest`, `ocxtest`, `testMSOffice.TestWord8` (mhammond#2290, mhammond#2281, mhammond#2291, mhammond#2478 [@Avasam][Avasam])

### adodbapi

* Fixes `NameError: name 'os' is not defined` error for `"getenv"` macro in `adodbapi.process_connect_string.macro_call` (mhammond#2283, [@Avasam][Avasam])

Build 308, released 2024-10-12
------------------------------

* Fix Pythonwin displaying syntax errors in Python 3.13 (mhammond#2393)
* Allowed installs from source w/o having pywin32 pre-installed (for instance, from GitHub) (mhammond#2349, [@Avasam][Avasam])
* Restored version stamping of installed DLLs (mhammond#2349, [@Avasam][Avasam])
* Fixed a circular import between `win32comext.axscript.client.framework` and `win32comext.axscript.client.error` (mhammond#2381, [@Avasam][Avasam])
* Remove long-deprecated `win32com.server.dispatcher.DispatcherWin32dbg` (mhammond#2382, [@Avasam][Avasam])

Build 307, released 2024-10-04
------------------------------

### Release process changes

pywin32 is now released from artifacts created by Github actions, whereas previously they were
created from an environment where certain tools and libraries were located and installed
by hand.

This means some capabilities are no longer provided - this includes some documentation artifacts, such
as the .chm file, certain MAPI libraries etc, and .exe installers.

### pywin32

* Remove obsolete and unused `pywin.is_platform_unicode` (mhammond#2343, [@Avasam][Avasam])
* Fix `isapi.ThreadPoolExtension`'s printing of exception traceback broken on Python 3.8+ (mhammond#2312, [@Avasam][Avasam])
* Add RealGetWindowClass (mhammond#2299, [@CristiFati][CristiFati])
* Make it compile on Python 3.13 (mhammond#2260, [@clin1234][clin1234])
* Fixed accidentally trying to raise a `str` instead of an `Exception` in (mhammond#2270, [@Avasam][Avasam])
  * `Pythonwin/pywin/debugger/debugger.py`
  * `Pythonwin/pywin/framework/dlgappcore.py`
  * `com/win32com/server/policy.py`
  * `win32/Lib/regutil.py`
  * `win32/scripts/VersionStamp/vssutil.py`
* Removed the following unused symbols. They were meant to be used as Exceptions, but were accidentally strings (mhammond#2270, mhammond#2269, [@Avasam][Avasam])
  * `pywin.debugger.debugger.error`
  * `pywin.framework.dlgappcore.error`
  * `win32com.server.policy.error`
  * `regutil.error`
  * `win32.scripts.VersionStamp.vssutil.error`
  * `win32com.universal.com_error`
  * `win32com.client.build.error`
  * `win32com.client.genpy.error`
* Add EnumDesktopWindows (mhammond#2219, [@CristiFati][CristiFati])
* Marked `exc_type` and `exc_traceback` in `win32comext.axscript.client.error.AXScriptException.__init__` as deprecated. (mhammond#2236, [@Avasam][Avasam])  
  They are now unused and all information is taken from the `exc_value` parameter.
* Fixed non-overriden `pywin.scintilla.formatter.Formatter.ColorizeString` raising `TypeError` instead of `RuntimeError` due to too many parameters (mhammond#2216, [@Avasam][Avasam])
* Fixed broken since Python 3 tokenization in `win32comext.axdebug.codecontainer.pySourceCodeContainer.GetSyntaxColorAttributes` (mhammond#2216, [@Avasam][Avasam])
* Fixed a `TypeError` due to incorrect kwargs in `win32comext.axscript.client.pydumper.Register` (mhammond#2216, [@Avasam][Avasam])
* Fixed error reporting of file copy failure for for installing debug dlls (mhammond#2216, [@Avasam][Avasam])
* Fixed `py.exe -m win32verstamp` command and other quote typos caused by Implied String Concatenation (mhammond#2225, [@Avasam][Avasam])
* Fixed tons of quote-related typos in strings, docs and comments (mhammond#2271 , [@Avasam][Avasam])
* Fixed VT_SAFEARRAY(VT_RECORD) which were missing the last element (mhammond#2247)
* Fixed `MFC redist DLLs not found` by preferring corresponding version but accepting different version (mhammond#2248, [@andreabravetti][andreabravetti])
* Fixed `pywintypes.error: (5, 'RegOpenKeyEx', 'Access is denied.')` when running service with debug parameter and no elevation (mhammond#2238, [@jmartens][jmartens])
* Fixed handling of `SyntaxError` exception from a Windows Scripting Host Python Script on Python 3.10+ (mhammond#2235, [@nbbeatty][nbbeatty])
* Add `CredGetSessionTypes` support (mhammond#2232, [@CristiFati][CristiFati])
* Fixed `win32clipboard` increasing size of data when `SetClipboardData` used with `CF_DIB` (mhammond#2184, [@CristiFati][CristiFati])
* Add `StoreLogoff` to `PyIMsgStore` to prevent possible hang when MAPI uninitializes or during session logoff (mhammond#2196, [@avivbrg][avivbrg])
* Enhance CredDelete to work with dictionaries (mhammond#2198, [@CristiFati][CristiFati])
* Add UnregisterHotKey support (mhammond#2185, [@CristiFati][CristiFati])
* IFolderView COM client support (mhammond#2180, mhammond#2181, mhammond#2182, [@CristiFati][CristiFati])
* Release GIL when calling CreateService or StartService (mhammond#2062, [@adamkbmc][adamkbmc])
* Drop support for Internet Explorer 10 (mhammond#2229, [@Avasam][Avasam])
* Removed considerations for never-built Windows CE (mhammond#2218, [@Avasam][Avasam])
* Stopped building `winxpgui` (mhammond#2217, [@Avasam][Avasam])
  * Raise `DeprecationWarning` when importing `winxpgui`
  * Added `GetWindowRgnBox` to `win32gui`
  * `winxpgui.GetConsoleWindow` now aliases `win32console.GetConsoleWindow`
  * Everything else is re-exported from `win32gui`
* Fixed and improved the following demos: `ietoolbar`, `fontdemo`, `msoffice`, `shell_view`, `context_menu`, `win32clipboardDemo` (mhammond#2217, mhammond#2101, [@Avasam][Avasam])
* Fixed undefined names reported by Flake8/Ruff (mhammond#2101, [@Avasam][Avasam], [@kxrob][kxrob])
  * Fixed the following public API:
    * Fixed `NameError` in `WordFrame.Create`, even though it wasn't used
    * Fixed a handful of `NameError` in `pywin.dialogs.ideoptions.OptionsPropPage` with format
    * Fixed `AttributeError` in `pywin.framework.dlgappcore.AppDialog.OnPaint`
    * Fixed trying to write banner to `sdterr` in `pywin.framework.interact.InteractiveCore.Init`
    * Fixed a `NameError` in `pywin.framework.mdi_pychecker.TheDocument.doSearch`
    * Removes unusable `HandleToUlong`, `UlongToHandle`, `UlongToPtr` and `UintToPtr` from `pywin.scintilla.scintillacon`
    * Fixed a `NameError` in `win32comext.axscript.client.pydumper.Register`
  * The following methods no longer throw errors (although their implementation is still unvalidated):
    * `mmsystem.MEVT_EVENTTYPE`
    * `mmsystem.MEVT_EVENTPARM`
    * `mmsystem.MCI_MSF_MINUTE`
    * `mmsystem.MCI_MSF_SECOND`
    * `mmsystem.MCI_TMSF_TRACK`
    * `mmsystem.MCI_TMSF_MINUTE`
    * `mmsystem.MCI_TMSF_SECOND`
    * `mmsystem.MCI_TMSF_FRAME`
    * `mmsystem.MCI_HMS_HOUR`
    * `mmsystem.MCI_HMS_MINUTE`
    * `mmsystem.MCI_HMS_SECOND`
    * `mmsystem.DIBINDEX`
    * `winnt.IMAGE_SNAP_BY_ORDINAL`
    * `winnt.IMAGE_ORDINAL`
* Removed `Unicode` and `UnicodeType` from `pywintypes` and `win32api` (mhammond#2200, [@Avasam][Avasam])
* Deprecate `afxres` in favor of `pywin.mfc.afxres`. The modules were identical (mhammond#2177, [@Avasam][Avasam])
* Improved `DispatcherWin32dbg`'s deprecation warning and raise an error when used (mhammond#2145, [@Avasam][Avasam])
* Removed obsolete/legacy way of registering a Pythonwin app and its Idle handlers from `pywin.framework.app` (mhammond#2144, [@Avasam][Avasam])
* Removed unused `win32comext.axscript.server.error` (mhammond#2202, [@Avasam][Avasam])
* Removed deprecated `win32com.server.exception.Exception` (mhammond#2142, [@Avasam][Avasam])
* Removed long-deprecated `UnicodeToString` param from multiple methods (mhammond#2143, [@Avasam][Avasam])
* Fixed `win32api.SetClassWord` being overwritten by `win32api.SetWindowWord` (mhammond#2199, [@Avasam][Avasam])
  * If you were using `win32api.SetClassWord` for its current behaviour, use `win32api.SetWindowWord` instead.
  * This also adds missing support for `win32api.SetWindowWord`
* Annotated module-level variables with ambiguous typing (mhammond#2175, [@Avasam][Avasam])
* `win32com.client.build.NoTranslateMap` is now a `set` (mhammond#2176, [@Avasam][Avasam])
* Fixed `ModuleNotFoundError: No module named 'dialog'` in `pywin.tools.regpy` (mhammond#2187, [@Avasam][Avasam])
* Fixed passing a `float` to `range` in `win32pdhquery.Query.collectdatafor` (mhammond#2170, [@Avasam][Avasam])
* Check that the filename w/o extension ends with `_d` rather than checking for `_d` anywhere in the file path (mhammond#2169, [@Avasam][Avasam])
* Cleaned up and fixed Axdebug (mhammond#2126, [@Avasam][Avasam])
  * `win32comext.axdebug.codecontainer.SourceCodeContainer` now uses the `debugDocument` parameter
  * `win32comext.axdebug.codecontainer` script can now be run independent of location
  * Fixed Method Resolution Order issue in `win32comext.axdebug.documents` (also mhammond#2071, [@wxinix-2022][wxinix-2022])
  * Fixed undefined names (`NameError`) in `win32comext.axdebug.expressions.DebugProperty.GetPropertyInfo`
  * Removed unused `win32comext.axdebug.util.all_wrapped`
  * Fixed multiple `ModuleNotFoundError` in `win32comext.axdebug` (mhammond#1983, [@Avasam][Avasam])
* Change `mbcs` encoding to `utf-8` in `com.win32com.client` (mhammond#2097, [@Avasam][Avasam])
* Avoid using `importlib` directly (mhammond#2123, [@Avasam][Avasam])
* Replace most usages of deprecated `distutils`:
  * Replace distutils.dep_util with setuptools.modified (mhammond#2148, [@Avasam][Avasam])
  * Replaced `distutils.FileList` usage with `pathlib` (mhammond#2138, [@Avasam][Avasam])
  * Replace `distutils.log` with `logging` (mhammond#2134, [@Avasam][Avasam])
  * Replace `distutils` with direct `setuptools` equivalents where possible (mhammond#2134, [@Avasam][Avasam])
* Replaced usages of the removed (in Python 3.12) `imp` module (mhammond#2113, [@Avasam][Avasam])
  * Fixed registering Python as a scripting language for `axscript`
  * Fixed `isapi` install
* Use collection literals and comprehensions where applicable (slight performance improvement) (mhammond#2108, [@Avasam][Avasam])
* Cleanup obsolete code for unsupported Python versions (mhammond#1990, mhammond#2127, mhammond#2205, mhammond#2214, [@Avasam][Avasam])
  * The following public names have been removed:
    * `pywin.framework.app.Win32RawInput`
    * `win32com.client.makepy.error`
    * Long obsoleted `dbi` module, use the `odbc` module instead
    * `win32com.client.dynamic.MakeMethod`
  * Added support for the following Python 3 methods:
    * `pywin.mfc.dialog.Dialog.__contains__`
    * `win32com.client.CoClassBaseClass.__bool__`
    * `win32com.client.combrowse.HLIRoot.__lt__`
    * `win32com.client.genpy.WritableItem.__lt__`
    * `__bool__` in classes generated by `win32com.client.genpy.WritableItem.WriteClassBody`
    * `win32timezone._SimpleStruct.__le__` (subclassed by `SYSTEMTIME`, `TIME_ZONE_INFORMATION`, `DYNAMIC_TIME_ZONE_INFORMATION`, `TimeZoneDefinition`)
  * The following methods no longer throw errors (although their implementation is still unvalidated):
    * `winnt.PRIMARYLANGID`
    * `winnt.SUBLANGID`
    * `winnt.LANGIDFROMLCID`
    * `winnt.SORTIDFROMLCID`
* Removed obsolete compatibility aliases (mhammond#2087, [@Avasam][Avasam])
  The following public names have been removed:
  * `win32comext.mapi.mapiutil.TupleType`
  * `win32comext.mapi.mapiutil.ListType`
  * `win32comext.mapi.mapiutil.IntType`
  * `netbios.byte_to_int`
* Resolved invalid string escapes warnings (mhammond#2045, mhammond#2124, [@Avasam][Avasam])
* Idiomatic type comparisons. Better handling of subclasses. (mhammond#1991, [@Avasam][Avasam])
* Cleaned up obsolete and redundant code (this should not directly affect the end-user):
  * Update and standardise obsolete `OSError` aliases (mhammond#2107, [@Avasam][Avasam])
  * Removed redundant and obsolete references to older python unicode compatibility (mhammond#2085, [@Avasam][Avasam])
  * Use byte-string (`b""`) for constant bytes values instead of superfluous `.encode` calls (mhammond#2046, [@Avasam][Avasam])
  * Cleaned up unused imports (mhammond#1986, mhammond#2051, mhammond#1990, mhammond#2124, mhammond#2126, [@Avasam][Avasam])
  * Removed duplicated declarations, constants and definitions (mhammond#2050, mhammond#1950, mhammond#1990, [@Avasam][Avasam])
* Small generalized optimization by using augmented assignments (in-place operators) where possible (mhammond#2274, [@Avasam][Avasam])
* General speed and size improvements due to all the removed code. (mhammond#2046, mhammond#1986, mhammond#2050, mhammond#1950, mhammond#2085, mhammond#2087, mhammond#2051, mhammond#1990, mhammond#2106, mhammond#2127, mhammond#2124, mhammond#2126, mhammond#2177, mhammond#2218, mhammond#2202, mhammond#2205, mhammond#2217)

### adodbapi

* Remove references to outdated IronPython (mhammond#2049, [@Avasam][Avasam])  
  This removes the following public names:
  * `adodbapi.adodbapi.onWin32`
  * `adodbapi.apibase.onIronPython`
  * `adodbapi.apibase.NullTypes`
  * `adodbapi.apibase.DateTime`
* Remove references to outdated `mxDateTime` (mhammond#2048, [@Avasam][Avasam])  
  This removes the following public names:
  * `adodbapi.apibase.mxDateTime`
  * `adodbapi.apibase.mxDateTimeConverter`
* Removed obsolete Python 2 aliases (mhammond#2088, [@Avasam][Avasam])  
  This removes the following public names:
  * `adodbapi.adodbapi.unicodeType`
  * `adodbapi.adodbapi.longType`
  * `adodbapi.adodbapi.StringTypes`
  * `adodbapi.adodbapi.maxint`
  * `adodbapi.apibase.unicodeType`
  * `adodbapi.apibase.longType`
  * `adodbapi.apibase.StringTypes`
  * `adodbapi.apibase.makeByteBuffer`
  * `adodbapi.apibase.memoryViewType`
* Remove outdated and unused remote feature (mhammond#2098, [@Avasam][Avasam])
* Migrated from `distutils` to `setuptools` (mhammond#2133, [@Avasam][Avasam])

Build 306, released 2023-03-26
------------------------------

* Add GetSystemPowerStatus (mhammond#2010, [@CristiFati][CristiFati])
* Add CascadeWindows (mhammond#1999, [@CristiFati][CristiFati])
* Add win32gui.ResetDC
* Fix leak in win32pdh.GetFormattedCounterArray
* Fix IIS on later python versions (mhammond#2025)
* Fix for service registration code updated in build 305 (mhammond#1985)
* Support for Python 3.6 was dropped, support for later versions was improved.

Build 305, released 2022-11-06
------------------------------

* Installation .exe files were deprecated.
* [@kxrob][kxrob] put a lot of work towards removing use of the deprecated Unicode
  API so we can build on Python 3.12. This should be largely invisible, but
  please report any unintended consequences.
* odbc: Handle `varchar(max)`/`nvarchar(max)` column sizes (mhammond#1954)
* win32api.GetTickCount() now returns an unsigned 64bit integer ([@kxrob][kxrob], mhammond#1946)
* win32pipe.GetNamedPipeHandleState() now takes a 3rd optional param
  indicating whether the username should be returned, and related constants
  added. ([@kxrob][kxrob], mhammond#1946)
* Add win32gui.GetTopWindow() and win32gui.GetAncestor() (mhammond#1928, [@CristiFati][CristiFati])
* Tweaks to how pywintypes searches for DLLs to better support virtualenvs
  created with --system-site-packages. ([@saaketp][saaketp], mhammond#1933)
* Added win32event.CreateWaitableTimerExW (mhammond#1945, [@zariiii9003][zariiii9003])
* Changes in PARAM handling. Some functions which returned a WPARAM or LPARAM
  allowed you to return a pointer to a Python buffer object or a PyUnicode.
  These functions now only accept a Python long to be returned. Note that
  this DOES NOT apply to functions with accept WPARAM or LPARAM as arguments,
  only when they are being returned. Impacted functions are `OnNotify`
  handler, LV_ITEM/TV_ITEM objects, PyIContextMenu3::HandleMenuMsg2, and the
  result of a WNDPROC/DLGPROC (mhammond#1927).
* service registration had an overhaul, avoiding a complicated, and ultimately
  unnecessary "single globally registered service runner" concept.
  Now, when registering a service, the host pythonservice.exe runner will be
  copied to `sys.exec_prefix`, along with possibly `pywintypesXX.dll` and run
  from there. (mhammond#1908)
* Dropped support for allowing a bytes object to be passed where a COM BSTR
  is expected - this support was accidental on the path from Python 2 -> 3.
* win32crypt's PyCERTSTORE.CertCloseStore()'s `Flags` argument has been
  deprecated as it is likely to crash the process if
  `CERT_CLOSE_STORE_FORCE_FLAG` is specified. The underlying function is now
  always called with `CERT_CLOSE_STORE_CHECK_FLAG`, and support for this
  param will be dropped at some point in the future.
* Fix a bug where win32crypt.CryptQueryObject() would return a PyCTL_CONTEXT
  object instead of a PyCERT_CONTEXT for base64 encoded certificates (mhammond#1859)
* win32crypt.CryptQueryObject() is now able to return PyCTL_CONTEXT objects.
  This is technically a breaking change as previously it would return the
  address in memory of the object, but this address wasn't practically usable,
  so it's very unlikely anyone relied on this behavior. (mhammond#1859)

Build 304, released 2022-05-02
------------------------------

* Fixed Unicode issues in the `dde` module (mhammond#1861, [@markuskimius][markuskimius] )
* Add `PRINTER_INFO_6` support for Set/GetPrinter (mhammond#1853, [@CristiFati][CristiFati])
* Fixed codepage/mojibake issues when non-ascii characters were included in
  COM exceptions raised by Python apps. This should be invisible, but might
  break any workarounds which were used, such as using specific encodings in
  these strings. (mhammond#1823, mhammond#1833)
* Fixed a bug triggering `win32print.SetJob` to fail due to data type
  (`char*` / `wchar_t*`) mismatch (mhammond#1849, [@CristiFati][CristiFati])
* Fix eventlog initialization (mhammond#1845, mhammond#1846, [@kxrob][kxrob])

Build 303, released 2021-12-20
------------------------------

* Tweaks to how DLLs are loaded and our installation found, which should
  improve virtualenv support and version mismatch issues (mhammond#1787, mhammond#1794)
* Fixed a bug where `win32clipboard.GetClipboardData()` may have returned extra
  data.
* Binary installers for 32-bit 3.10+ are no longer released (mhammond#1805)

Build 302, released 2021-10-11
------------------------------

* Fixed support for unicode as a `win32crypt.CREDENTIAL_ATTRIBUTE.Value`
* Support for Python 3.10, dropped support for Python 3.5 (3.5 security support
  ended 13 Sep 2020)
* Merged win2kras into win32ras. In the unlikely case that anyone is still
  using win2kras, there is a win2kras.py that imports all of win32ras. If you
  import win2kras and it fails with 'you must import win32ras first', then it
  means an old win2kras.pyd exists, which you should remove.
* GitHub branch 'master' was renamed to 'main'.

Build 301, released 2021-05-30
------------------------------

* Fix some confusion on how dynamic COM object properties work. The old
  code was confused, so there's a chance there will be some subtle
  regression here - please open a bug if you find anything, but this
  should fix mhammond#1427.
* COM objects are now registered with the full path to pythoncomXX.dll, fixes
  mhammond#1704.
* Creating a `win32crypt.CRYPT_ATTRIBUTE` object now correctly sets `cbData`.
* Add wrap and unwrap operations defined in the GSSAPI to the sspi module
  and enhance the examples given in this module.
  (mhammond#1692, Emmanuel Coirier)
* Fix a bug in `win32profile.GetEnvironmentStrings()` relating to environment
  variables with an equals sign ([@maxim-krikun][maxim-krikun] in mhammond#1661)
* Fixed a bug where certain COM dates would fail to be converted to a Python
  datetime object with `ValueError: microsecond must be in 0..999999`. Shoutout
  to [@hujiaxing][hujiaxing] for reporting and helping reproduce the issue (mhammond#1655)
* Added win32com.shell.SHGetKnownFolderPath() and related constants.
* CoClass objects should work better with special methods like `__len__` etc.
  (mhammond#1699)
* Shifted work in win32.lib.pywin32_bootstrap to Python's import system from
  manual path manipulations ([@wkschwartz][wkschwartz] in mhammond#1651)
* Fixed a bug where win32print.DeviceCapabilities would return strings
  containing the null character followed by junk characters.
  (mhammond#1654, mhammond#1660, Lincoln Puzey)

Build 300, released 2020-11-14
------------------------------

* Fixed a bug where win32com.client.VARIANT params were returned in the reverse
  order. This only happened when win32com.client.VARIANT was explicitly used
  (ie, not when normal params were passed) For example:

  ```python
      arg1 = VARIANT(pythoncom.VT_R4 | pythoncom.VT_BYREF, 2.0)
      arg2 = VARIANT(pythoncom.VT_BOOL | pythoncom.VT_BYREF, True)
      object.SomeFunction(arg1, arg2)
  ```

  after this call, `arg1.value` was actually the value for `arg2`, and
  vice-versa (mhammond#1303, mhammond#622).

* Fixed a bug that Pythonwin had an empty `sys.argv` ([@kxrob][kxrob] in mhammond#1607)

* Fixed a bug that prevented win32process.ReadProcessMemory() from working
  in all scenarios (mhammond#1599)

* Changed how Services implemented with win32serviceutil.ServiceFramework
  report that they have stopped. Now if the SvcRun() method (or the SvcDoRun()
  method, which is called by SvcRun() by default) raises on Exception,
  the Service will report a final SERVICE_STOPPED status with a non-zero error
  code. This will cause the Service's recovery actions to be triggered if the
  Service has the "Enable actions for stops with errors" option enabled.
  (mhammond#1563, Lincoln Puzey)

* adodbapi connect() method now accepts a "mode" keyword argument which is the
  "Mode" property to set on the ADO "Connection" object before opening the
  Connection. See "ConnectModeEnum" for valid values.
  (Lincoln Puzey)

* The Windows 10 SDK is now used to build the project. This shouldn't cause any
  visible changes, but should make it much easier to build the project yourself.

Python 2 is no longer supported - so long, Python 2, you served us well!

Notable changes in this transition:

* Python 3 builds used to erroneously turn "bytes" into a tuple of integers
  instead of a buffer type object. Because this special-casing is important for
  performance when using massive buffers, this has been fixed in Python 3 so
  it matches the old Python 2 behavior. If you use arrays of VT_UI1 and expect
  get back tuples of integers, your code may break.

* Pythonwin's default encoding is now utf-8 (mhammond#1559)

* The build environment has been greatly simplified - you just need Visual
  Studio and a Windows 10 SDK. (The free compilers probably work too, but
  haven't been tested - let me know your experiences!)

Previous
--------

Build 228 (2020-06-13) was the last to support Python 2.0.

Older entries are periodically removed - see the git history of this file
for them.

<!-- Create short-form markdown user links -->
[adamkbmc]: https://github.com/adamkbmc
[andreabravetti]: https://github.com/andreabravetti
[Avasam]: https://github.com/Avasam
[avivbrg]: https://github.com/avivbrg
[clin1234]: https://github.com/clin1234
[CristiFati]: https://github.com/CristiFati
[geppi]: https://github.com/geppi
[gesslerpd]: https://github.com/gesslerpd
[hujiaxing]: https://github.com/hujiaxing
[jmartens]: https://github.com/jmartens
[kxrob]: https://github.com/kxrob
[markuskimius]: https://github.com/markuskimius
[maxim-krikun]: https://github.com/maxim
[Mscht]: https://github.com/Mscht
[nbbeatty]: https://github.com/nbbeatty
[saaketp]: https://github.com/saaketp
[the-snork]: https://github.com/the-snork
[wkschwartz]: https://github.com/wkschwartz
[wxinix-2022]: https://github.com/wxinix
[zariiii9003]: https://github.com/zariiii9003
