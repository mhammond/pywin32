from typing import TYPE_CHECKING

if isinstance(__path__, str):
    # For freeze to work!
    import sys

    try:
        if TYPE_CHECKING:
            # Get the name from typeshed stubs
            from win32comext.mapi import mapi
        else:
            import mapi

        sys.modules["win32com.mapi.mapi"] = mapi
    except ImportError:
        pass
    try:
        if TYPE_CHECKING:
            # Get the name from typeshed stubs
            from win32comext.mapi import exchange
        else:
            import exchange

        sys.modules["win32com.mapi.exchange"] = exchange
    except ImportError:
        pass
else:
    import win32com

    # See if we have a special directory for the binaries (for developers)
    win32com.__PackageSupportBuildPath__(__path__)
