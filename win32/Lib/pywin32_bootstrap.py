# Imported by pywin32.pth to bootstrap the pywin32 environment in "portable"
# environments or any other case where the post-install script isn't run.
#
# In short, there's a directory installed by pywin32 named 'pywin32_system32'
# with some important DLLs which need to be found by Python when some pywin32
# modules are imported.
# If Python has `os.add_dll_directory()`, we need to call it with this path.
# Otherwise, we add this path to PATH.
import os


try:
    Exc = ModuleNotFoundError  # Introduced in Python 3.6
except NameError:
    Exc = ImportError

try:
    import pywin32_system32
except Exc:
    path_iterator = iter([])
else:
    # We're guaranteed only that __path__: Iterable[str]
    # https://docs.python.org/3/reference/import.html#__path__
    path_iterator = iter(pywin32_system32.__path__)

try:
    pywin32_system32 = next(path_iterator)
except StopIteration:
    pass
else:
    if os.path.isdir(pywin32_system32):
        if hasattr(os, "add_dll_directory"):
            os.add_dll_directory(pywin32_system32)
        # This is to ensure the pywin32 path is in the beginning to find the
        # pywin32 DLLs first and prevent other PATH entries to shadow them
        elif not os.environ["PATH"].startswith(pywin32_system32):
            os.environ["PATH"] = os.environ["PATH"].replace(os.pathsep + pywin32_system32, "")
            os.environ["PATH"] = pywin32_system32 + os.pathsep + os.environ["PATH"]
