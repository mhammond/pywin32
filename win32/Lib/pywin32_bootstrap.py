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
            # First try the preferred method
            if hasattr(os, "add_dll_directory"):
                os.add_dll_directory(path)
            # If `add_dll_directory` is missing, which can happen in Pylance early initialization,
            # try to modify PATH if it exists (just create it if it doesn't)
            elif "PATH" not in os.environ:
                os.environ["PATH"] = path
            else:
                # This is to ensure the pywin32 path is in the beginning to find the
                # pywin32 DLLs first and prevent other PATH entries to shadow them
                prepend_to_path = path + os.pathsep
                if not os.environ["PATH"].startswith(prepend_to_path):
                    os.environ["PATH"] = prepend_to_path + os.environ["PATH"].replace(
                        os.pathsep + path, ""
                    )
            break
