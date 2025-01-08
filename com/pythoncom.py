# Magic utility that "redirects" to pythoncomXX.dll
from typing import TYPE_CHECKING, Any

import pywintypes

pywintypes.__import_pywin32_system_module__("pythoncom", globals())

# This module dynamically re-exports from a C-Extension.
# Prevent mypy attr-defined and pyright reportAttributeAccessIssue errors locally
# External usage should still prefer typeshed stubs
if TYPE_CHECKING:

    def __getattr__(name: str) -> Any: ...
