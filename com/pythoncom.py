# Magic utility that "redirects" to pythoncomXX.dll
from typing import TYPE_CHECKING, Any

import pywintypes

pywintypes.__import_pywin32_system_module__("pythoncom", globals())

# This module dynamically re-exports from a C-Extension.
# Prevent attribute access issues with checkers and language servers (IDEs)
# External usage should still prefer typeshed stubs
if TYPE_CHECKING:

    def __getattr__(name: str) -> Any: ...
