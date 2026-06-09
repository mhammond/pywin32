# Imported by pywin32.pth to bootstrap the pywin32 environment in "portable"
# environments or any other case where the post-install script isn't run.
#
# In short, there's a directory installed by pywin32 named 'pywin32_system32'
# with some important DLLs which need to be found by Python when some pywin32
# modules are imported.
import sys

try:
    import pywin32_system32
except ModuleNotFoundError:
    pass
else:
    import os

    # We're guaranteed only that __path__: Iterable[str]
    # https://docs.python.org/3/reference/import.html#path-attributes-on-modules
    for path in pywin32_system32.__path__:
        if os.path.isdir(path):
            os.add_dll_directory(path)
            break

# HACK to avoid having to add `__lazy_modules__` absolutely everywhere. This works for
# pywin32 because we already hack `import pywin32_bootstrap` in `pywin32.pth`.
#
# This is advanced usage, not recommended for libraries,
# so let's make sure we don't get in the way of end users' lazy import filters.
if (
    sys.version_info >= (3, 15)
    # If user set lazy imports to "all", this is unnecessary
    and sys.get_lazy_imports() == "normal"
    # If there's an existing filter, don't override it
    and not sys.get_lazy_imports_filter()
):
    # Modules that must remain eagerly imported due to import-time side effects
    _PYWIN32_LAZY_EXCLUSIONS = frozenset(
        {
            "pywin32_bootstrap",
            # These are the same listed in ruff.toml
            "coloreditor",
            "IDLEenvironment",
            "pythoncom",
            "pywintypes",
            "win32com",
            "win32traceutil",
            "win32ui",
        }
    )

    def _collect_pywin32_roots() -> frozenset[str]:
        import os

        # Known pywin32 roots outside win32
        roots = {
            # root
            "adodbapi",
            "isapi",
            "win32",
            # com
            "win32com",
            "win32comext",
            "pythoncom",
            # pythonwin
            "pythonwin",
            "pywin",
            "win32ui",
            "win32uiole",
        }
        # scanning win32 root-exposed modules as there's too many to keep track
        win32_lib_dir = os.path.dirname(__file__)
        win32_dir = os.path.dirname(win32_lib_dir)
        try:
            win32_roots = {
                entry.name.partition(".")[0]
                for scan_dir in (win32_dir, win32_lib_dir)
                for entry in os.scandir(scan_dir)
                if entry.is_file() and entry.name.endswith((".py", ".pyd"))
            }
            roots |= win32_roots
        except OSError:
            pass
        return frozenset(roots)

    _PYWIN32_ROOTS = _collect_pywin32_roots()

    def _pywin32_lazy_imports_filter(
        importer: str, name: str, fromlist: "tuple[str, ...] | None"
    ) -> bool:
        # Only auto-lazy import in pywin32's own code
        return importer.partition(".")[0] in _PYWIN32_ROOTS and not any(
            part in _PYWIN32_LAZY_EXCLUSIONS for part in name.split(".")
        )

    sys.set_lazy_imports_filter(_pywin32_lazy_imports_filter)
