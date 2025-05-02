# Imported by pywin32.pth to bootstrap the pywin32 environment in "portable"
# environments or any other case where the post-install script isn't run.
#
# In short, there's a directory installed by pywin32 named 'pywin32_system32'
# with some important DLLs which need to be found by Python when some pywin32
# modules are imported.


try:
    import pywin32_system32
except ImportError:
    pass
else:
    import os

    # We're guaranteed only that __path__: Iterable[str]
    # https://docs.python.org/3/reference/import.html#path-attributes-on-modules
    for path in pywin32_system32.__path__:
        if os.path.isdir(path):
            try:
                # First try the preferred method
                os.add_dll_directory(path)
            except Exception:
                # If anything fails, try to modify PATH if it exists
                try:
                    if "PATH" in os.environ:
                        os.environ["PATH"] = path + os.pathsep + os.environ["PATH"]
                    else:
                        # If PATH doesn't exist, just create it
                        os.environ["PATH"] = path
                except Exception:
                    # Last resort - if nothing works, just pass silently
                    pass
            break
