From https://github.com/devdanzin/ft-review-toolkit:

```

  Free-Threading Analysis Report

  Extension: pywin32 (win32, win32com, pythonwin, isapi — ~225 C/C++ files)

  Migration Status

  Status: In Progress — active on feature branches, minimal on main

  The PYWIN_MODULE_INIT_PREPARE macro in PyWinTypes.h centrally emits
  PyUnstable_Module_SetGIL(module, Py_MOD_GIL_NOT_USED) under #ifdef Py_GIL_DISABLED —
  this means all ~30 modules get the GIL-opt-out declaration once the feature branch
  lands. That makes the unprotected shared state below active bugs on the feature
  branch today, not theoretical future concerns.

  Corrections from prior Group A analysis: hasInitialized, g_dwCoInitThread, and
  g_bCoInitThreadHasInit in dllmain.cpp were initially flagged as RACE/CRITICAL, but
  CEnterLeaveFramework (backed by g_csMain) covers all access sites — these are safe.
  bTrapAccessViolations is read-only after init and exists only in #ifdef _DEBUG. The
  true dllmain.cpp vulnerability is dwQuitThreadId, where the read site's lock is
  explicitly commented out.

  ---
  Executive Summary

  - Readiness: Moderate — foundational Win32 callback and COM registry infrastructure
  has active data races; blocking APIs have incomplete GIL-release migration
  - RACE findings: 8 — confirmed or likely data races
  - UNSAFE findings: 1 — deprecated thread-unsafe API in hot path
  - PROTECT findings: 9 — shared state needing synchronization
  - MIGRATE findings: 4 — structural changes needed

  Agents that confirmed: shared-state-auditor, ft-history-analyzer,
  lock-discipline-checker, atomic-candidate-finder, unsafe-api-detector,
  stop-the-world-advisor

  No _PyEval_StopTheWorld is needed — all findings are per-object or per-module scope.
  Per-object critical sections and PyMutex guarded init are the right tools throughout.

  ---
  Findings by Priority

  RACE Findings (fix immediately) — 8

  #: 1
  Finding: Py_MOD_GIL_NOT_USED declared; g_obRegisteredVTables dict written by
    RegisterVTable and read via borrowed ref across PY_INTERFACE_PRECALL window in
    CreateRegisteredTearOff
  File:Line: univgw.cpp:14, 605, 667, 756
  Severity: CRITICAL
  Agents: shared-state, lock-discipline, unsafe-api, stw-advisor
  ────────────────────────────────────────
  #: 2
  Finding: timer_id_callback_map borrowed ref (line 25) used after PyErr_Warn call
  (line
    28) that can execute arbitrary Python — invalidating the ref before Py_INCREF at
    line 35; concurrent kill_timer races with timer callback
  File:Line: timermodule.cpp:14, 25–35, 88, 109
  Severity: CRITICAL
  Agents: shared-state, lock-discipline, unsafe-api, stw-advisor
  ────────────────────────────────────────
  #: 3
  Finding: consoleControlHandlers PyList iterated by Win32 ctrl callback while
    PySetConsoleCtrlHandler appends/splices; comment explicitly says "thread-safety
    provided by GIL"
  File:Line: win32apimodule.cpp:5575–5685
  Severity: CRITICAL
  Agents: shared-state, lock-discipline, unsafe-api, stw-advisor
  ────────────────────────────────────────
  #: 4
  Finding: g_callbackMap (ISAPI) and obHandleMap (win32ras) accessed concurrently from
    native thread pool / RAS callback and Python API; comment "As we hold the thread
    lock, assume noone else can mod this dict" is invalid under free-threading
  File:Line: isapi/PyExtensionObjects.cpp:42–127, win32rasmodule.cpp:19, 444–571
  Severity: CRITICAL
  Agents: shared-state, unsafe-api, stw-advisor
  ────────────────────────────────────────
  #: 5
  Finding: Five COM registration dicts (g_obPyCom_Map*) written without lock from
    concurrent COM extension loading; additionally pythoncom_IsGatewayRegistered:232
    calls Py_DECREF(v) on a borrowed reference from PyDict_GetItem — a refcount
    corruption bug
  File:Line: Register.cpp:63–139, 232
  Severity: HIGH
  Agents: shared-state, lock-discipline, unsafe-api, stw-advisor
  ────────────────────────────────────────
  #: 6
  Finding: g_HWNDMap/g_DLGMap window proc callback dicts: PyWndProcHWND and
    PyDlgProcHDLG read/delete from these dicts on whichever thread owns each window's
    message queue — multiple window-owning threads access the shared dicts concurrently
  File:Line: win32gui.i:18–20, 641–714
  Severity: HIGH
  Agents: unsafe-api
  ────────────────────────────────────────
  #: 7
  Finding: dwQuitThreadId (static DWORD): write in PyCom_EnableQuitMessage has zero
    synchronization; read in PyCom_DLLReleaseRef has its CEnterLeaveFramework
  explicitly
     commented out
  File:Line: dllmain.cpp:35–36, 95–96
  Severity: HIGH
  Agents: lock-discipline, atomic-candidate
  ────────────────────────────────────────
  #: 8
  Finding: Blocking Win32 APIs holding GIL: CredEnumerate, CredRead, CredDelete,
    CredRename and 7 other Cred* functions; LookupPrivilegeName,
    LookupPrivilegeDisplayName, LookupPrivilegeValue; NetUserChangePassword —
  incomplete
     migration from commit b5f7e74f
  File:Line: win32credmodule.cpp:436–1102, win32security.i:1446–1521,
    win32netuser.cpp:438
  Severity: HIGH
  Agents: ft-history

  ---
  UNSAFE Findings (fix before declaring free-threading support) — 1

  #: 9
  Finding: PyErr_Fetch/PyErr_Restore deprecated since Python 3.12; not thread-safe
  under
    free-threading; used in the high-concurrency COM dispatch path
  File:Line: ErrorUtils.cpp:36, 364, 389, 409+, PyHANDLE.cpp:352, 355,
    PythonService.cpp:1263, pythonwin/win32virt.cpp:42, 55, win32gui.i:35
  Severity: MEDIUM
  Agents: unsafe-api

  ---
  PROTECT Findings (add synchronization) — 9

  #: 10
  Finding: PyCom_RegisterCoreSupport check-then-act race: two threads both see
    g_obPyCom_MapIIDToType == NULL, both proceed, second call corrupts/leaks the
  first's
     dicts
  File:Line: Register.cpp:316–317
  Severity: HIGH
  Agents: shared-state, stw-advisor
  ────────────────────────────────────────
  #: 11
  Finding: Lazy-init singletons with explicit "relies on GIL" / "assumes we have the
    GIL" comments: Decimal_class (3 call sites), PyVariant_Type, got/GetTZUTC
  File:Line: PyComHelpers.cpp:37, 57–100, oleargs.cpp:15, 38–46, PyTime.cpp:28–46
  Severity: MEDIUM
  Agents: shared-state, atomic-candidate, stw-advisor
  ────────────────────────────────────────
  #: 12
  Finding: addedCtrlHandler BOOL flag: check-then-act TOCTOU; two threads both call
    SetConsoleCtrlHandler to register the C handler
  File:Line: win32apimodule.cpp:5659–5678
  Severity: MEDIUM
  Agents: atomic-candidate
  ────────────────────────────────────────
  #: 13
  Finding: num_message_modules non-atomic += 1 counter; concurrent module loads can
  lose
    increments and write to the same slot in the error_message_modules array
  File:Line: PyWinTypesmodule.cpp:273
  Severity: MEDIUM
  Agents: shared-state, atomic-candidate
  ────────────────────────────────────────
  #: 14
  Finding: PyWinExc_ApiError / PyWinExc_COMError non-static externs written by
    PyWinGlobals_Ensure(); concurrent first-import from two threads can race on the
    pointer write
  File:Line: PyWinTypesmodule.cpp:21–22, 139
  Severity: MEDIUM
  Agents: shared-state
  ────────────────────────────────────────
  #: 15
  Finding: pCallbackCaller global: read by Win32 window message callbacks, set by
  Python
    API — no synchronization
  File:Line: pythonwin/win32uimodule.cpp:683–698
  Severity: MEDIUM
  Agents: shared-state
  ────────────────────────────────────────
  #: 16
  Finding: bInError function-local static recursion guard in gui_print_error: two
    threads both pass the check, both enter the error handler concurrently
  File:Line: pythonwin/win32uimodule.cpp:608–620
  Severity: MEDIUM
  Agents: atomic-candidate
  ────────────────────────────────────────
  #: 17
  Finding: m_reload_exception written under CSLock(m_initLock) at line 85 but read at
    line 291 (dispatchRequest) without any lock
  File:Line: isapi/PythonEng.cpp:85, 291
  Severity: MEDIUM
  Agents: lock-discipline
  ────────────────────────────────────────
  #: 18
  Finding: CoCreateInstanceEx GIL release: original fix reverted due to deadlock; 2nd
    attempt on origin/CoCreateInstance_gil not yet landed; production CoCreateInstance
    call sites unaudited
  File:Line: com/TestSources/PyCOMTest/PyCOMImpl.cpp, PythonCOM.cpp
  Severity: MEDIUM
  Agents: ft-history

  ---
  MIGRATE Findings (structural changes needed) — 4

  #: 19
  Finding: ~30+ static PyTypeObject instances registered via PyType_Ready() — shared
    across all threads and interpreters; need migration to heap types via
    PyType_FromSpec
  File:Line: odbc.cpp, mmapfilemodule.cpp, MiscTypes.cpp, win32trace.cpp,
    PyWinTypesmodule.cpp and ~25 more
  Severity: MEDIUM
  Agents: shared-state
  ────────────────────────────────────────
  #: 20
  Finding: CEnterLeavePython callers assume "GIL acquisition = sufficient mutual
    exclusion" — systematically incorrect under free-threading; per-object protection
  is
     also required at each site
  File:Line: win32/src/PyWinTypes.h:695–717 (all callers)
  Severity: MEDIUM
  Agents: lock-discipline
  ────────────────────────────────────────
  #: 21
  Finding: timermodule and ctrl-handler code in win32apimodule store callback maps as
    module-global statics — must migrate to per-module state with multi-phase init so
    Win32 callbacks can target the correct interpreter
  File:Line: timermodule.cpp:14, win32apimodule.cpp:5575–5576
  Severity: MEDIUM
  Agents: lock-discipline
  ────────────────────────────────────────
  #: 22
  Finding: _win32sysloader.cpp calls PyUnstable_Module_SetGIL unconditionally (line 79)

    without #ifdef Py_GIL_DISABLED guard; univgw.cpp wraps it correctly — consistency
    needed
  File:Line: _win32sysloader.cpp:79
  Severity: LOW
  Agents: ft-history

  ---
  SAFE Patterns (confirmed safe)

  - g_cLockCount / PyGatewayBase::m_cRef: Uses
  InterlockedIncrement/InterlockedDecrement — the correct reference atomic pattern;
  serves as the model for all other flag fixes
  - hasInitialized, g_dwCoInitThread, g_bCoInitThreadHasInit in dllmain.cpp: All access
   sites are inside CEnterLeaveFramework / g_csMain — correctly protected
  - bTrapAccessViolations (win32uimodule.cpp): Read-only after static zero-init; #ifdef
   _DEBUG only; no write sites found
  - _win32sysloader.cpp: Py_MOD_GIL_NOT_USED declared with genuinely zero mutable state
   — the correct model for simple modules
  - Win32 HANDLEs in win32trace.cpp: Named mutex/event handles provide inter-process
  synchronization independent of the GIL
  - PYCOM_USE_FREE_THREAD / #ifdef WITH_FREE_THREAD in PyWinTypes.h: Correctly compiles
   out PyWin_InterpreterState TLS machinery on free-threaded builds
  - CEnterLeaveFramework RAII: Correctly wraps all access to g_csMain-protected state;
  no raw EnterCriticalSection calls outside this class
  - PyW32_BEGIN/END_ALLOW_THREADS pairing in win32apimodule.cpp: All 103 usage
  instances correctly paired
  - TlsSetValue/TlsGetValue in dllmain.cpp: Inherently per-thread — safe
  - univgw_AddRef/univgw_Release: Use InterlockedIncrement/InterlockedDecrement on cRef
   — correct for COM ref counting

  ---
  Recommendations

  Immediate (RACE — Findings 1–8)

  1. Finding 1 (univgw.cpp): Before any PY_INTERFACE_PRECALL, do Py_INCREF(obVTable) to
   own the reference; release after use. Wrap all g_obRegisteredVTables mutations in
  Py_BEGIN_CRITICAL_SECTION. Either remove Py_MOD_GIL_NOT_USED from univgw.cpp until
  dict is protected, or treat it as the blocker for that declaration.
  2. Finding 2 (timermodule.cpp): Replace the PyDict_GetItem + delayed Py_INCREF
  pattern with PyDict_GetItemRef (Python 3.13+) or a
  Py_BEGIN_CRITICAL_SECTION(timer_id_callback_map) block that does lookup + incref
  atomically. Protect all dict mutations with the same critical section.
  3. Finding 3 (win32apimodule.cpp): Add a PyMutex g_ctrlHandlerMutex protecting both
  consoleControlHandlers and addedCtrlHandler. In PyCtrlHandler, snapshot the list
  under the mutex then iterate the snapshot. Remove the "thread-safety provided by GIL"
   comments.
  4. Finding 4 (isapi/PyExtensionObjects.cpp, win32rasmodule.cpp): Same pattern as
  Finding 2 — one CRITICAL_SECTION or Py_BEGIN_CRITICAL_SECTION per module protecting
  each callback dict. Guard obHandleMap lazy creation with a PyMutex.
  5. Finding 5 (Register.cpp:232): Immediately remove the Py_DECREF(v) on the borrowed
  reference at line 232 — this is a refcount corruption bug present in all builds
  today. Then add a shared PyMutex protecting all five g_obPyCom_Map* dicts.
  6. Finding 6 (win32gui.i): Wrap all g_HWNDMap/g_DLGMap access with
  Py_BEGIN_CRITICAL_SECTION. Replace PyDict_GetItem with PyDict_GetItemRef to take
  owned references before executing callbacks.
  7. Finding 7 (dllmain.cpp): Change dwQuitThreadId to volatile LONG and use
  InterlockedExchange/InterlockedCompareExchange — matching the existing g_cLockCount
  pattern.
  8. Finding 8 (win32credmodule.cpp, win32security.i, win32netuser.cpp): Apply the
  Py_BEGIN_ALLOW_THREADS + GetLastError() capture pattern to the ~10 remaining
  unguarded blocking API call sites. This is mechanical and low-risk.

  Short-term (UNSAFE + PROTECT — Findings 9–18)

  9. Finding 9 (ErrorUtils.cpp etc.): Replace PyErr_Fetch/PyErr_Restore with
  PyErr_GetRaisedException/PyErr_SetRaisedException throughout.
  10. Finding 10 (Register.cpp:316): Guard PyCom_RegisterCoreSupport with a static
  PyMutex using double-checked locking, or use Win32 InitOnceExecuteOnce.
  11. Findings 11–12: Replace lazy-init NULL-check patterns with either eager
  initialization in the module exec function (preferred) or _Py_atomic_ptr
  compare-exchange. Remove all "relies on the GIL" comments.
  12. Finding 13: Protect num_message_modules += 1 with the existing g_csMain critical
  section; convert to _Py_atomic_int.
  13. Findings 14–17: Add InitOnceExecuteOnce for PyWinGlobals_Ensure; add
  _Py_atomic_store_ptr for pCallbackCaller writes; use _Py_atomic_compare_exchange_int
  for bInError; wrap m_reload_exception reads in CSLock.
  14. Finding 18: Verify the 2nd-attempt fix on origin/CoCreateInstance_gil resolves
  the test deadlock; land it. Audit production CoCreateInstance/CoCreateInstanceEx call
   sites in PythonCOM.cpp and univgw.cpp.

  Longer-term (MIGRATE — Findings 19–22)

  15. Finding 19: Migrate the ~30+ static PyTypeObject instances to heap types using
  PyType_FromSpec. Prioritize win32api, win32com, and pywintypes types first. Required
  for correct sub-interpreter support independent of free-threading.
  16. Finding 20: Audit all CEnterLeavePython call sites — document that GIL
  acquisition is no longer sufficient and each shared Python object access requires
  per-object critical section coverage.
  17. Finding 21: Migrate timermodule and the ctrl-handler section of win32apimodule to
   per-module state with multi-phase init (Py_mod_create + Py_mod_exec). Store the
  interpreter reference at callback registration time.
  18. Finding 22: Add #ifdef Py_GIL_DISABLED guard around PyUnstable_Module_SetGIL in
  _win32sysloader.cpp:79.
```

From https://github.com/devdanzin/cext-review-toolkit:

```
  C Extension Analysis Report

  Extension: pywin32

  Scope: Entire project (win32/, com/, isapi/, pythonwin/) — vendored Scintilla and
  MAPIStubLibrary excluded

  Agents Run: refcount-auditor, error-path-analyzer, null-safety-scanner,
  gil-discipline-checker, resource-lifecycle-checker, module-state-checker,
  type-slot-checker, pyerr-clear-auditor, stable-abi-checker, version-compat-scanner,
  git-history-analyzer (parity-checker and c-complexity-analyzer blocked by tool
  permissions — not assessed)

  Branch: nogil+swig4

  ---
  Executive Summary

  pywin32 is a large, mature Win32/COM binding library whose C extension layer contains
  a mix of long-standing correctness bugs and new technical debt introduced by the
  active free-threading migration. The highest-priority issue is Register.cpp:232 — a
  Py_DECREF on a borrowed reference from PyDict_GetItem that is a crash-severity
  refcount corruption bug on every Python build. A cluster of similarly severe
  all-builds correctness bugs follow: an always-FALSE COM gateway (PyIPropertyStorage
  ReadMultiple), a Win32 API function that returns success with an active exception
  set, and _Py_NewReference-pattern type constructors with Py_TPFLAGS_BASETYPE but no
  tp_new, causing allocator mismatch for Python subclasses. The free-threading
  migration (Py_MOD_GIL_NOT_USED declared for all ~30 modules) is structurally correct
  but the underlying C code retains numerous unprotected shared dicts, GIL-reliant
  lazy-init singletons, and callback-thread races that will trigger data corruption
  under free-threaded Python 3.13t+. The version compatibility posture is disciplined —
  no unguarded post-3.9 APIs — but 31 deprecated PyErr_Fetch/Restore call sites and
  ~50 files of deprecated T_* struct members need migration before Python deprecation
  removal. Recommended first action: fix finding 1 (Register.cpp:232) and finding 2
  (PyIPropertyStorage.cpp:451-458) immediately; they are one-line and four-line fixes
  respectively and affect every pywin32 user.

  ---
  Extension Profile

  ┌────────────────┬───────────────────────────────────────────────────────────────┐
  │     Field      │                             Value                             │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Modules        │ ~30 C extension modules                                       │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Source files   │ 214 C++ files + 11 SWIG .i files                              │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Init style     │ Single-phase (PyModule_Create) via PYWIN_MODULE_INIT_PREPARE  │
  │                │ macro                                                         │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Python targets │ ≥3.9 (CI: 3.9–3.15 + 3.13t/3.14t/3.15t free-threaded)         │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Limited API    │ No                                                            │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Types defined  │ ~21 static PyTypeObjects (win32/), ~50+ in com/               │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Code           │ Mixed: hand-written C++ + SWIG 1.1-generated wrappers         │
  │ generation     │                                                               │
  ├────────────────┼───────────────────────────────────────────────────────────────┤
  │ Free-threading │ Py_MOD_GIL_NOT_USED declared; C-level audit in progress       │
  └────────────────┴───────────────────────────────────────────────────────────────┘

  ---
  Key Metrics

  ┌───────────────┬────────┬─────┬──────────┬──────────────────────────────────────┐
  │   Dimension   │ Status │ FIX │ CONSIDER │             Top Finding              │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Refcount      │ 🔴     │ 9   │ 4        │ Py_DECREF on borrowed PyDict_GetItem │
  │ Safety        │        │     │          │  ref — Register.cpp:232              │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Error         │ 🔴     │ 7   │ 5        │ Always-FALSE ReadMultiple gateway —  │
  │ Handling      │        │     │          │ PyIPropertyStorage.cpp:451           │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ NULL Safety   │ 🟡     │ 5   │ 5        │ Unchecked PyTuple_New in COM typelib │
  │               │        │     │          │  conversion (OOM crash)              │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ GIL           │        │     │          │ PyWin_SetAPIError inside             │
  │ Discipline    │ 🔴     │ 1   │ 2        │ Py_BEGIN_ALLOW_THREADS —             │
  │               │        │     │          │ win32trace.cpp                       │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Resource      │ 🟡     │ 3   │ 2        │ GlobalAlloc leak on SetClipboardData │
  │ Lifecycle     │        │     │          │  failure                             │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Module State  │ 🟡     │ 5   │ 5        │ COM singletons (g_obEmpty etc.)      │
  │               │        │     │          │ overwritten on reimport              │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Type Slots    │ 🔴     │ 4   │ 6        │ PyHANDLE hash/equality inconsistency │
  │               │        │     │          │  breaks dict/set                     │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ PyErr_Clear   │ 🟡     │ 4   │ 7        │ COM gateway QueryInterface swallows  │
  │ Safety        │        │     │          │ MemoryError                          │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ ABI / Private │ 🟡     │ 0   │ 5        │ _PyLong_Sign deprecated in 3.14 (not │
  │  APIs         │        │     │          │  removed)                            │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Version       │ 🟡     │ 0   │ 3        │ 31 PyErr_Fetch/Restore deprecated    │
  │ Compat        │        │     │          │ sites across 8 files                 │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ C/Python      │ ⬜     │ —   │ —        │ Not assessed — agent blocked         │
  │ Parity        │        │     │          │                                      │
  ├───────────────┼────────┼─────┼──────────┼──────────────────────────────────────┤
  │ Complexity    │ ⬜     │ —   │ —        │ Not assessed — agent blocked         │
  └───────────────┴────────┴─────┴──────────┴──────────────────────────────────────┘

  🔴 = 4+ FIX findings | 🟡 = 1–3 FIX findings | ⬜ = not assessed

  ---
  Findings by Priority

  Must Fix (FIX) — 35

  Findings are globally numbered. All-builds / non-OOM bugs first; OOM-triggered;
  resource leaks; module state; adjacent-pattern confirmed bugs follow.

  Tier A — All-builds correctness; no OOM required

  #: 1
  Finding: Py_DECREF(v) on borrowed reference returned by PyDict_GetItem; v is live in
    the dict and this decrements its refcount, corrupting the COM type registry
  File:Line: Register.cpp:232
  Agents: refcount-auditor, git-history
  ────────────────────────────────────────
  #: 2
  Finding: Missing braces in PyObject_AsPreallocatedPROPVARIANTs; return FALSE is the
    for-loop body, not the error branch — ReadMultiple COM gateway always fails for
    non-empty arrays
  File:Line: PyIPropertyStorage.cpp:451-458
  Agents: error-path-analyzer
  ────────────────────────────────────────
  #: 3
  Finding: PyErr_Fetch transfers ownership of 3 objects; after Py_XDECREF(exc_info),
    exc_typ, exc_val, exc_tb are never released — 3 leaks + exception permanently lost
    per logging call
  File:Line: win32gui.i:30-59
  Agents: refcount-auditor
  ────────────────────────────────────────
  #: 4
  Finding: pPOT->Init(...) called unconditionally after new (PyPerfMonManager) that can

    return NULL — NULL pointer dereference crash
  File:Line: MappingManager.cpp:136-141
  Agents: error-path-analyzer
  ────────────────────────────────────────
  #: 5
  Finding: PyWin_SetAPIError (sets Python exception state) called inside an active
    Py_BEGIN_ALLOW_THREADS block; corrupts interpreter exception state, identified by
  an
     inline comment confirming the author knew
  File:Line: win32trace.cpp:277-295
  Agents: gil-discipline
  ────────────────────────────────────────
  #: 6
  Finding: ReturnAPIError("LoadCursor") result is discarded (no return); falls through
    to PyWinLong_FromHANDLE(NULL) returning 0 with exception already set — SystemError
    triggered on caller's next C API call
  File:Line: win32apimodule.cpp:1149-1155
  Agents: error-path-analyzer
  ────────────────────────────────────────
  #: 7
  Finding: Same discarded-error fall-through for CommandLineToArgvW; returns empty list

    with active exception
  File:Line: win32apimodule.cpp:1167-1187
  Agents: error-path-analyzer
  ────────────────────────────────────────
  #: 8
  Finding: PyHANDLE::hash() returns _Py_HashPointer(this) (object address) but
    richcompare() uses m_handle value — two distinct PyHANDLE objects wrapping the same

    HANDLE compare == but hash differently; silently wrong dict/set behavior
  File:Line: PyHANDLE.cpp (hashFunc + richcompare)
  Agents: type-slot
  ────────────────────────────────────────
  #: 9
  Finding: Same hash/equality inconsistency on PyOVERLAPPED — hashFunc uses pointer,
    richcompareFunc uses memcmp of struct contents
  File:Line: PyOVERLAPPED.cpp
  Agents: type-slot
  ────────────────────────────────────────
  #: 10
  Finding: Py_TPFLAGS_BASETYPE set (Python-subclassable) but tp_new = 0 and tp_dealloc
    calls delete instead of tp_free — Python subclasses are allocated via tp_alloc
    (Python heap) and freed via delete (MSVC heap); undefined behavior / crash under
    debug allocators or ASAN
  File:Line: PyHANDLE.cpp
  Agents: type-slot
  ────────────────────────────────────────
  #: 11
  Finding: Same Py_TPFLAGS_BASETYPE + delete-in-dealloc mismatch; tp_new calls new
    PyDEVMODEW rather than type->tp_alloc
  File:Line: PyDEVMODE.cpp
  Agents: type-slot
  ────────────────────────────────────────
  #: 12
  Finding: PyObject_RichCompare return value leaked on every loop iteration; NULL
  return
    from failed comparison not checked — one leak per iteration + silent wrong result
  on
     failure
  File:Line: PythonEng.cpp:147-153
  Agents: refcount-auditor
  ────────────────────────────────────────
  #: 13
  Finding: valueObject (refcount 1) leaked on PyDict_SetItem failure path
  File:Line: Register.cpp:97-106
  Agents: refcount-auditor
  ────────────────────────────────────────
  #: 14
  Finding: PyErr_Clear() in QueryInterface unconditionally swallows any Python
  exception
    including MemoryError; commented // ### what to do with exceptions?
  File:Line: PyGatewayBase.cpp:165,170
  Agents: pyerr-clear
  ────────────────────────────────────────
  #: 15
  Finding: PyErr_Clear() called before returning E_OUTOFMEMORY in getids_setup /
    invoke_setup — suppresses the very MemoryError that E_OUTOFMEMORY is intended to
    signal
  File:Line: PyGatewayBase.cpp:259-282
  Agents: pyerr-clear
  ────────────────────────────────────────
  #: 16
  Finding: PyErr_Clear() called before return E_OUTOFMEMORY in universal gateway vtable

    dispatch — allocation failure exception silently discarded
  File:Line: univgw.cpp:87
  Agents: pyerr-clear
  ────────────────────────────────────────
  #: 17
  Finding: VLogF_Logger in ErrorUtils.cpp: when the logger call fails, PyErr_Print() is

    called before PyErr_Restore() — PyErr_Print clobbers sys.last_* while the original
    exception is conceptually in flight; unsafe under free-threaded Python 3.13t+ where

    sys.last_* mutation is a shared-state race
  File:Line: ErrorUtils.cpp:389-409
  Agents: pyerr-clear

  Tier B — OOM-triggered NULL dereferences (crash on allocation failure)

  #: 18
  Finding: PyTuple_New(len) not checked for NULL; immediate crash in PyTuple_SetItem on

    OOM
  File:Line: PyFUNCDESC.cpp:15-16
  Agents: null-safety
  ────────────────────────────────────────
  #: 19
  Finding: PyTuple_New unchecked before immediate PyTuple_SetItem — 3 distinct sites
  File:Line: PyIType.cpp:38, 132, 964
  Agents: null-safety
  ────────────────────────────────────────
  #: 20
  Finding: Same pattern — 2 sites in PyObject_FromARRAYDESC and
    PyObject_FromELEMDESCArray
  File:Line: PyITypeObjects.cpp:83, 271
  Agents: null-safety
  ────────────────────────────────────────
  #: 21
  Finding: PyTuple_New(1) unchecked; crashes at PyTuple_SET_ITEM and leaks
    previously-built args tuple — in Windows Service startup path
  File:Line: PythonService.cpp:1169-1170
  Agents: null-safety
  ────────────────────────────────────────
  #: 22
  Finding: PyTuple_New(nbr_langs) unchecked; crash at PyTuple_SetItem in
    GetFileVersionInfo translation branch
  File:Line: win32apimodule.cpp:5492-5498
  Agents: null-safety

  Tier C — Reachable resource leaks (no OOM required)

  #: 23
  Finding: HANDLE + PyObject leaked on empty-file error path: if
    (!m_obj->mapping_size.QuadPart) returns NULL without Py_DECREF(m_obj), skipping
    mmapfile_object_dealloc which would call CloseHandle
  File:Line: mmapfilemodule.cpp:569
  Agents: resource-lifecycle
  ────────────────────────────────────────
  #: 24
  Finding: GlobalAlloc block leaked when SetClipboardData fails — Windows docs require
    caller to free on failure; author left // XXX - should we GlobalFree the mem?
    comment
  File:Line: win32clipboardmodule.cpp:868
  Agents: resource-lifecycle
  ────────────────────────────────────────
  #: 25
  Finding: Same GlobalAlloc leak in py_set_clipboard_text variant
  File:Line: win32clipboardmodule.cpp:945-946
  Agents: resource-lifecycle

  Tier D — Module state correctness (incorrect behavior on reimport /
  multi-interpreter)

  #: 26
  Finding: g_obEmpty, g_obMissing, g_obArgNotFound, g_obNothing, PyCom_InternalError
    created unconditionally every PyInit_pythoncom call with no NULL guard — previous
    value leaked on reimport; cross-interpreter aliasing of pythoncom.Empty singleton
  File:Line: PythonCOM.cpp:2146-2178
  Agents: module-state
  ────────────────────────────────────────
  #: 27
  Finding: PyModule_AddObject(module, "com_record", &PyRecord::Type) with no prior
    Py_INCREF — steals reference from statically-allocated type; attempted dealloc of
    static storage on module unload
  File:Line: PythonCOM.cpp:2192
  Agents: module-state, refcount
  ────────────────────────────────────────
  #: 28
  Finding: g_obRegisteredVTables = PyDict_New() in initunivgw() with no NULL guard —
    previous dict overwritten on reimport (all registered VTables silently lost);
    deferred crash at PyDict_GetItem if PyDict_New returns NULL
  File:Line: univgw.cpp:741
  Agents: module-state
  ────────────────────────────────────────
  #: 29
  Finding: timer_id_callback_map = PyDict_New() unconditionally overwrites on reimport
  —
    active timer registrations silently lost; Win32 timers continue to fire,
    PyDict_GetItem returns NULL, timers orphaned
  File:Line: timermodule.cpp:14
  Agents: module-state
  ────────────────────────────────────────
  #: 30
  Finding: servicemanager_startup_error = PyErr_NewException(...) unconditionally on
    every PyInit_servicemanager — memory leak on reimport;
    PyErr_ExceptionMatches(servicemanager_startup_error) fails if callers cached the
  old
     class
  File:Line: PythonService.cpp:553
  Agents: module-state

  Tier E — Adjacent pattern bugs confirmed by history analysis

  #: 31
  Finding: PyDict_GetItem(g_obPyCom_MapServerIIDToGateway, keyObject) result used then
    Py_DECREF'd — borrowed reference; same bug class as finding 1, different function
  in
     different file
  File:Line: PyComHelpers.cpp:274,381
  Agents: git-history
  ────────────────────────────────────────
  #: 32
  Finding: pStat->pwcsName = NULL without CoTaskMemFree — STATSTG.pwcsName is allocated

    by the COM storage layer; zeroing the pointer leaks COM task-allocator memory;
    self-documented with // XXX - need to fix this
  File:Line: PyComHelpers.cpp:465
  Agents: git-history
  ────────────────────────────────────────
  #: 33
  Finding: /// XXX - this leaks this variant :-( — developer-acknowledged unfixed
    VARIANT leak in type-object conversion loop, no subsequent commit has addressed it
  File:Line: PyITypeObjects.cpp:208
  Agents: git-history
  ────────────────────────────────────────
  #: 34
  Finding: PyTuple_New(cNames) (line 257) immediately precedes the confirmed
    PyErr_Clear-masks-OOM region (lines 259-282); if the tuple allocation fails, the
    clear at 259 silently drops the MemoryError that PyTuple_New set — fixing finding
  15
     must also guard line 257
  File:Line: PyGatewayBase.cpp:257
  Agents: git-history
  ────────────────────────────────────────
  #: 35
  Finding: 2021 NULL-guard fix (f4798e0a) covered PyCom_RegisterClientType but left
    PyCom_RegisterGatewayObject without a g_obPyCom_MapServerIIDToGateway == NULL guard

    — same crash class, incomplete fix
  File:Line: Register.cpp (PyCom_RegisterGatewayObject)
  Agents: git-history

  ---
  Should Consider (CONSIDER) — 28

  #: 36
  Finding: PyOVERLAPPED holds obState (arbitrary user-attached object) and obhEvent —
    both live PyObject* members — but has no tp_traverse/tp_clear and
  Py_TPFLAGS_HAVE_GC
     not set; reference cycles through obState invisible to GC; common in asyncio/IOCP
    code
  File:Line: PyOVERLAPPED.cpp
  ────────────────────────────────────────
  #: 37
  Finding: PyFUNCDESC, PyTYPEATTR, PyVARDESC each hold PyObject* members exposed as
    assignable T_OBJECT members; no GC flags — user-created cycles not collected
  File:Line: PyFUNCDESC.cpp, PyTYPEATTR.cpp, PyVARDESC.cpp
  ────────────────────────────────────────
  #: 38
  Finding: 5 ISAPI filter sub-objects (PyURL_MAP, PyPREPROC_HEADERS, etc.) hold strong
    PyHFC *m_parent reference; no tp_traverse/tp_clear; parent→child→parent cycles leak

    in long-running IIS processes
  File:Line: isapi/PyFilterObjects.cpp
  ────────────────────────────────────────
  #: 39
  Finding: PyType_Ready return values not checked for 9 ISAPI types in
    InitExtensionTypes / InitFilterTypes; partial-init types cause corrupted objects on

    later instantiation
  File:Line: PyExtensionObjects.cpp:1028, PyFilterObjects.cpp:1107-1113
  ────────────────────────────────────────
  #: 40
  Finding: All COM interface types define tp_richcompare but tp_hash = 0 — implicit
    PyObject_HashNotImplemented; COM interface objects cannot be used as dict keys or
    set members
  File:Line: MiscTypes.cpp type_template
  ────────────────────────────────────────
  #: 41
  Finding: PyRecord::tp_hash = 0 with tp_richcompare defined — com_record objects
    unhashable; value-type records are commonly used as dict keys
  File:Line: PyRecord.cpp:398,408
  ────────────────────────────────────────
  #: 42
  Finding: univgw.cpp:741 g_obRegisteredVTables = PyDict_New() not checked for NULL —
    deferred crash at first PyDict_GetItem (also listed as FIX #28; the crash path is
    the FIX part, the unchecked allocation is this CONSIDER)
  File:Line: univgw.cpp:741
  ────────────────────────────────────────
  #: 43
  Finding: oleargs.cpp:714 PyDict_New() not checked; passed to
    PyCom_CalculatePyObjectDimension; NULL dereference inside helper at first
    PyDict_GetItem call
  File:Line: oleargs.cpp:714
  ────────────────────────────────────────
  #: 44
  Finding: dllmain.cpp:66-69 PyList_New(0) not checked; PyList_Append on NULL crashes
  File:Line: dllmain.cpp:66
  ────────────────────────────────────────
  #: 45
  Finding: adsilib.i, mapilib.i SWIG typemaps call PyList_New(0) then immediately
    PyList_Append without NULL check; pattern multiplied across all generated function
    wrappers
  File:Line: adsilib.i:79,124, mapilib.i:393-461
  ────────────────────────────────────────
  #: 46
  Finding: CEnterLeavePython::acquire() calls PyGILState_Ensure without
    Py_IsInitialized() guard; used in CTRL_CLOSE_EVENT and service-stop callbacks that
    fire during/after interpreter shutdown — crash or deadlock; PythonEng.cpp:62
  already
     has this guard as the correct model
  File:Line: PyWinTypes.h:~702
  ────────────────────────────────────────
  #: 47
  Finding: win32trace.cpp:615-631 module init: hMutex leaked if subsequent CreateEvent
    fails; hEvent leaked if second CreateEvent fails — bounded (one-time init) but
  still
     incorrect
  File:Line: win32trace.cpp:615-631
  ────────────────────────────────────────
  #: 48
  Finding: PyGEnumVariant::Next error path calls PyErr_Clear() unconditionally — if
    PyObject_Length raised MemoryError, clear drops it; E_OUTOFMEMORY HRESULT not
    returned in that case
  File:Line: PyGEnumVariant.cpp:47
  ────────────────────────────────────────
  #: 49
  Finding: PyDlgProcHDLG calls PyDict_GetItem + unconditional PyErr_Clear() — should
  use
    PyDict_GetItemWithError; OOM from dict hash error silently dropped in dialog
  message
     loop
  File:Line: win32gui.i:702
  ────────────────────────────────────────
  #: 50
  Finding: win32evtlog.i EvtRender: PyWinObject_FromEVT_VARIANT failure cleared and
    replaced with Py_None — swallows MemoryError, caller receives incomplete list
    without indication
  File:Line: win32evtlog.i:1000-1004
  ────────────────────────────────────────
  #: 51
  Finding: odbc.cpp:773-774 ibindFloat returns NULL instead of 0 — this is an
    int-returning function; incorrect sentinel causes false-positive error detection on

    any subsequent check
  File:Line: odbc.cpp:773-774
  ────────────────────────────────────────
  #: 52
  Finding: odbc.cpp:1617-1620 PyModule_AddObject failure path leaks obtypes;
    PYWIN_MODULE_INIT_RETURN_ERROR does not release it
  File:Line: odbc.cpp:1617-1620
  ────────────────────────────────────────
  #: 53
  Finding: odbc.cpp:1174-1244 inputvars reference leaked on error paths in bulk-insert
    loop
  File:Line: odbc.cpp:1174-1244
  ────────────────────────────────────────
  #: 54
  Finding: PyWin_InterpreterState captures the first-importing interpreter and is never

    updated; PyWinThreadState_Ensure always binds COM callbacks to the first
    interpreter's thread state — silent cross-interpreter contamination in
    multi-interpreter scenarios
  File:Line: PyWinTypesmodule.cpp:139,143-165
  ────────────────────────────────────────
  #: 55
  Finding: PyWinExc_ApiError / PyWinExc_COMError created once with NULL guard but
  shared
    across all interpreters — second interpreter's pywintypes.error is an object owned
    by the first interpreter's heap
  File:Line: PyWinTypesmodule.cpp:797-860
  ────────────────────────────────────────
  #: 56
  Finding: timermodule.cpp:14 / win32assoc.cpp:99-190 ASSERT_GIL_HELD comments +
  comment
    "GIL prevents races" — these are concurrency correctness assumptions that are false

    under Py_GIL_DISABLED (active on this branch)
  File:Line: Multiple
  ────────────────────────────────────────
  #: 57
  Finding: dllmain.cpp:145,159 — two XXX markers: "regularly hangs" and "Needs more
    thought about threading implications"; COM DllMain + Python lock acquisition is a
    classic loader-lock deadlock; under free-threading the mask is removed
  File:Line: dllmain.cpp:145,159
  ────────────────────────────────────────
  #: 58
  Finding: win32notify.cpp:52,70 — HACK HACK - FIX ME FIX ME markers;
    PyBytes_FromString("") substituted where actual notification payload expected;
    callbacks receive wrong data
  File:Line: win32notify.cpp:52,70
  ────────────────────────────────────────
  #: 59
  Finding: _PyUnicode_AsString used via PYWIN_ATTR_CONVERT macro (51 callsites) — it's
    now defined as PyUnicode_AsUTF8 compat shim in Python 3.14 headers, but relying on
    the private name is fragile; replace with PyUnicode_AsUTF8 directly
  File:Line: PyWinTypes.h:74
  ────────────────────────────────────────
  #: 60
  Finding: _PyLong_Sign marked Py_DEPRECATED(3.14) in Python 3.14 headers;
    _PyLong_NumBits return type changed size_t→int64_t (functionally compatible but
    signals future removal); used unguarded in COM variant hot path
  File:Line: oleargs.cpp:141-142
  ────────────────────────────────────────
  #: 61
  Finding: ob_type = &SomeType direct field write pattern (~350 sites) — deprecated in
    3.9; Py_SET_TYPE is the correct form and is available at pywin32's 3.9 minimum
  File:Line: All C++ type constructors
  ────────────────────────────────────────
  #: 62
  Finding: structmember.h T_* macros deprecated in 3.12; ~50 files affected;
    PyWinTypes.h:18 includes structmember.h globally
  File:Line: PyWinTypes.h:18 + ~50 files
  ────────────────────────────────────────
  #: 63
  Finding: AXDebug.cpp frame struct access guard at >= 0x030b0000 (3.11) is too
    conservative — PyFrame_GetCode / PyFrame_GetBack have been public since 3.9; the
    #else struct-access branch can be dropped entirely
  File:Line: AXDebug.cpp:59-61, 189-230

  ---
  Tensions

  - Free-threading enablement vs. safety: PYWIN_MODULE_INIT_PREPARE declares
  Py_MOD_GIL_NOT_USED for all ~30 modules under Py_GIL_DISABLED, but the C code still
  contains ~10 unprotected shared-state patterns (console handlers, RAS callback map,
  timer dict, ISAPI g_callbackMap, pythonwin CAssocManager, COM helper singletons). The
  declaration is correct as an aspirational gate; the protection work is Phase 1–3 of
  the migration plan.
  - Single-phase init vs. subinterpreter correctness: The PYWIN_MODULE_INIT_PREPARE
  macro intentionally uses single-phase init (no Py_mod_multiple_interpreters). This is
  a valid architectural decision for a COM/Win32 binding library where IID maps are
  process-wide. However, findings 26–30 (missing NULL idempotency guards on globals)
  are not a consequence of this decision — they are mechanical correctness bugs within
  the single-phase model that leak memory on any reimport.
  - _Py_NewReference architectural approach: The C++ constructor pattern (placement new
  + _Py_NewReference) is used in ~48 files across 119 sites. It is presently
  functional — all private symbols verified present in Python 3.14 headers — but is a
  long-term fragility as CPython allocator internals evolve. Replacing it requires
  converting PyIBase and all ~50 COM subclasses to use tp_alloc/PyObject_Init. This is
  a multi-sprint effort and should be tracked as a separate initiative rather than
  treated as FIX-priority today.

  ---
  Policy Decisions (POLICY) — 12

  #: P1
  Finding: 31 PyErr_Fetch/PyErr_Restore/PyErr_NormalizeException call sites across 8
    files (ErrorUtils.cpp ×6, pythonpsheet.cpp, win32uimodule.cpp, win32virt.cpp,
    PyHANDLE.cpp, PythonService.cpp, win32consolemodule.cpp) — deprecated in Python
    3.12; migrate to PyErr_GetRaisedException/PyErr_SetRaisedException (requires #if
    PY_VERSION_HEX >= 0x030c0000 guard or pythoncapi-compat)
  ────────────────────────────────────────
  #: P2
  Finding: consoleControlHandlers PyList shared between Win32 ctrl-handler OS thread
  and
    Python mutator threads — comment "thread-safety provided by GIL" is invalid under
    Py_GIL_DISABLED; protect with PyMutex + Py_BEGIN_CRITICAL_SECTION
  ────────────────────────────────────────
  #: P3
  Finding: obHandleMap in win32rasmodule.cpp shared between RAS OS callback and
    PyWinDial — comment "As we hold the thread lock, assume noone else can mod this
    dict" invalid under free-threading
  ────────────────────────────────────────
  #: P4
  Finding: g_callbackMap in isapi/PyExtensionObjects.cpp protected only by GIL;
    concurrent IIS requests race on this dict under free-threading
  ────────────────────────────────────────
  #: P5
  Finding: PythonService.cpp:553 static have_init flag with // XXX - this assumes GIL
    held comment — races under free-threading; use std::call_once or
  InitOnceExecuteOnce
  ────────────────────────────────────────
  #: P6
  Finding: PyComHelpers.cpp:37 Decimal_class lazy-init singleton; oleargs.cpp:15
    PyVariant_Type; PyTime.cpp:28-46 GetTZUTC statics — all have "relies on GIL"
    comments; protect with PyMutex / _Py_once_flag
  ────────────────────────────────────────
  #: P7
  Finding: win32credmodule.cpp:436-788 — 9 Cred* functions missing
    Py_BEGIN_ALLOW_THREADS; incomplete migration from commit b5f7e74f; full list:
    CredMarshalCredential, CredUnmarshalCredential, CredEnumerate, CredGetTargetInfo,
    CredWriteDomainCredentials, CredReadDomainCredentials, CredDelete, CredRead,
    CredRename
  ────────────────────────────────────────
  #: P8
  Finding: win32security.i:1446-1521 — LookupPrivilegeName, LookupPrivilegeDisplayName,

    LookupPrivilegeValue missing Py_BEGIN_ALLOW_THREADS
  ────────────────────────────────────────
  #: P9
  Finding: WTSWaitSystemEvent (blocks indefinitely), ReadConsoleInput, ReadConsole,
    RegConnectRegistry (network), WTSSendMessage(Wait=TRUE) — all missing
    Py_BEGIN_ALLOW_THREADS; freeze all Python threads during wait
  ────────────────────────────────────────
  #: P10
  Finding: _Py_NewReference (119 sites), ob_type = field writes (~350 sites) —
    architectural migration to PyObject_Init + Py_SET_TYPE; track as multi-sprint
    initiative separate from individual bug fixes
  ────────────────────────────────────────
  #: P11
  Finding: pygcapi-compat not used; vendoring pythoncapi_compat.h would eliminate all
    manual #if PY_VERSION_HEX guards for PyWeakref_GetRef, PyErr_GetRaisedException,
    etc. and reduce future maintenance cost
  ────────────────────────────────────────
  #: P12
  Finding: g_obPyCom_MapRecordGUIDToRecordClass stores user-defined Python subclasses
  in
    a process-wide dict — unlike the IID maps (intentionally process-wide), this dict
  is
     interpreter-specific and should be separated once multi-phase init is adopted

  ---
  Strengths

  - CI matrix is excellent: 3.9–3.15 × x86/x64/ARM64 × GIL/free-threaded covers the
  widest supported range in pywin32's history. The free-threaded variants are already
  in CI.
  - Version guard discipline: Zero unguarded post-3.9 API calls found. Every
  PyWeakref_GetRef, PyUnstable_Module_SetGIL, and pre-3.11 frame access is correctly
  conditionalized.
  - PY_INTERFACE_PRECALL/POSTCALL macro pair: The GIL release/acquire pattern around
  COM calls is consistent and used correctly in most COM dispatch paths.
  - PYWIN_MODULE_INIT_PREPARE macro: Centralizes the PyUnstable_Module_SetGIL call
  under #ifdef Py_GIL_DISABLED in one place — if the API is promoted to a stable name,
  it's a one-file change.
  - CEnterLeavePython RAII: Correct pattern for non-Python-thread callbacks; the
  existing model is sound for GIL builds.
  - COM interface reference management: PyCom_PyObjectFromIUnknown correctly uses
  POFIU_RELEASE_ON_FAILURE to handle COM interface ownership on error paths — clean in
  all surveyed paths.
  - Recidivism awareness: The codebase has .git history showing 20+ years of the same
  bug class in Register.cpp being fixed. The authors have been diligent about fixing
  reported issues; the remaining bugs are gaps in coverage, not willful neglect.

  ---
  Code Removal Opportunities

  With minimum Python ≥3.9:

  - AXDebug.cpp pre-3.11 branch (#if PY_VERSION_HEX < 0x030b0000): PyFrame_GetCode and
  PyFrame_GetBack are available since 3.9. Delete the entire #else struct-access block
  (~15 lines) and the #include "compile.h" / #include "frameobject.h" guards. Net: ~20
  lines removed.
  - PyErr_Fetch/Restore migration (once pythoncapi-compat is vendored): Each of the 31
  call sites collapses from a 3-variable save/normalize/restore pattern to a 1-variable
  save/restore. Net: ~3 lines saved per site → ~60 lines removed.
  - structmember.h removal (once min Python ≥3.12): Remove the #include
  "structmember.h" from PyWinTypes.h:18; rename T_INT → Py_T_INT etc. in ~50 files.
  Net: clean removal of the deprecated header and ~41 macro rename sites.
  - _Py_NewReference → PyObject_Init (long-term): After the COM type hierarchy is
  refactored to use tp_alloc, the _Py_NewReference + ob_type = constructor pattern in
  48 files can be replaced with PyObject_Init(this, &type). Net: 119 + 350 = ~469 line
  edits, eliminates the private API entirely.

  ---
  Recommended Action Plan

  Immediate (FIX — all-builds correctness; safe to submit independently)

  1. Fix finding 1 (Register.cpp:232): Change Py_DECREF(v) to nothing — v is borrowed;
  the dict owns it. Confirm no Py_INCREF of v precedes this line.
  2. Fix finding 2 (PyIPropertyStorage.cpp:451-458): Add braces to the if
  (!PyObject_AsPROPVARIANT(...)) block so return FALSE is inside it, not the loop body.
  3. Fix finding 5 (win32trace.cpp:277-295): Propagate the raw Win32 error code out of
  the mutex helpers and call PyWin_SetAPIError only after Py_END_ALLOW_THREADS.
  4. Fix findings 6–7 (win32apimodule.cpp): Add return before each ReturnAPIError call
  in LoadCursor and CommandLineToArgvW.
  5. Fix finding 3 (win32gui.i:30-59): After calling Python logger, restore then clear
  all 3 Py_XDECREF-freed exception objects; or migrate to
  PyErr_GetRaisedException/PyErr_SetRaisedException pattern.
  6. Fix finding 4 (MappingManager.cpp:136-141): Add goto done after PyErr_SetString
  for the NULL allocation case.
  7. Fix findings 8–9 (PyHANDLE.cpp, PyOVERLAPPED.cpp): Make hashFunc derive the hash
  from m_handle / struct contents to agree with richcompare.
  8. Fix findings 10–11 (PyHANDLE.cpp, PyDEVMODE.cpp): Add tp_new that allocates via
  type->tp_alloc(type, 0) and call Py_TYPE(ob)->tp_free(ob) in tp_dealloc.
  9. Fix findings 14–16 (PyGatewayBase.cpp, univgw.cpp): Replace unconditional
  PyErr_Clear() with PyErr_ExceptionMatches(PyExc_MemoryError) check → return
  E_OUTOFMEMORY before clearing. Also add NULL check for PyTuple_New(cNames) at line
  257 (finding 34).
  10. Fix findings 23–25 (mmapfilemodule.cpp, win32clipboardmodule.cpp ×2): Add
  Py_DECREF(m_obj) and GlobalFree(handle) / GlobalFree(hMem) on failure paths.
  11. Fix findings 26–30 (module globals): Add if (x == NULL) { x = ...; } idempotency
  guards; add Py_INCREF(&PyRecord::Type) before PyModule_AddObject.
  12. Fix findings 31–33 (PyComHelpers.cpp:274,381,465): Remove Py_DECREF on borrowed
  dict-get results; replace pwcsName = NULL with CoTaskMemFree(pStat->pwcsName);
  pStat->pwcsName = NULL.

  Short-term (FIX Tier B + C + selected CONSIDER)

  13. Findings 18–22 (unchecked PyTuple_New): Add NULL checks + return NULL / DECREF
  cleanup after every PyTuple_New call.
  14. Finding 12 (PythonEng.cpp): Replace PyObject_RichCompare + pointer-compare with
  PyObject_RichCompareBool.
  15. Finding 13 (Register.cpp:97-106): Add Py_DECREF(valueObject); return E_FAIL; on
  PyDict_SetItem failure.
  16. Finding 35 (incomplete PyCom_RegisterGatewayObject guard): Add if
  (g_obPyCom_MapServerIIDToGateway == NULL) return E_FAIL;.
  17. Findings 39–41 (ISAPI PyType_Ready, GC traverse/clear for PyOVERLAPPED, COM
  hash): Low individual cost, high correctness value for IOCP-heavy code.

  Medium-term (POLICY — free-threading correctness)

  18. P2–P4 (console handlers, RAS map, ISAPI g_callbackMap): Add PyMutex +
  Py_BEGIN_CRITICAL_SECTION to protect each shared dict accessed from OS callback
  threads.
  19. P5–P6 (lazy-init singletons with "GIL prevents races" comments): Replace with
  PyMutex-guarded or _Py_once_flag initialization.
  20. P7–P8 (missing Py_BEGIN_ALLOW_THREADS in win32credmodule.cpp, win32security.i):
  Apply pattern from existing CredWrite fix to the 9 remaining Cred* functions and 3
  LookupPrivilege* functions.

  Longer-term (POLICY — migration)

  21. P1 (31 PyErr_Fetch/Restore sites): Vendor pythoncapi_compat.h; migrate all 8
  files to PyErr_GetRaisedException/PyErr_SetRaisedException.
  22. P10 (_Py_NewReference + ob_type = writes): Plan multi-sprint refactor of PyIBase
  and COM type hierarchy to tp_alloc/PyObject_Init.
  23. P9 (blocking Win32 calls without GIL release): Systematically audit remaining
  non-Py_BEGIN_ALLOW_THREADS Win32 waits.
  24. Remove the #if PY_VERSION_HEX < 0x030b0000 block in AXDebug.cpp (20-line cleanup,
  zero risk at 3.9 min).
```