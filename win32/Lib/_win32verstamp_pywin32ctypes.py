"""
A pure-python re-implementation of methods used by win32verstamp.
This is to avoid a bootstraping problem where win32verstamp is used during build,
but requires an installation of pywin32 to be present.
We used to work around this by ignoring failure to verstamp, but that's easy to miss.

Implementations adapted, simplified and typed from:
- https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/core/ctypes/_util.py
- https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/core/cffi/_resource.py
- https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/pywin32/win32api.py

---

(C) Copyright 2014 Enthought, Inc., Austin, TX
All right reserved.

This file is open source software distributed according to the terms in
https://github.com/enthought/pywin32-ctypes/blob/main/LICENSE.txt
"""

from __future__ import annotations

from collections.abc import Callable, Iterable
from ctypes import FormatError, WinDLL, _SimpleCData, get_last_error
from ctypes.wintypes import (
    BOOL,
    DWORD,
    HANDLE,
    LPCWSTR,
    LPVOID,
    WORD,
)
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from ctypes import _NamedFuncPointer

    from _typeshed import ReadableBuffer
    from typing_extensions import Literal, SupportsBytes, SupportsIndex

kernel32 = WinDLL("kernel32", use_last_error=True)

###
# https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/core/ctypes/_util.py
###


def function_factory(
    function: _NamedFuncPointer,
    argument_types: list[type[_SimpleCData[Any]]],
    return_type: type[_SimpleCData[Any]],
    error_checking: Callable[..., Any],  # Simplified over errcheck's signature
) -> _NamedFuncPointer:
    function.argtypes = argument_types
    function.restype = return_type
    function.errcheck = error_checking
    return function


def make_error(function: _NamedFuncPointer) -> OSError:
    code = get_last_error()
    description = FormatError(code).strip()
    function_name = function.__name__
    exception = OSError()
    exception.winerror = code
    exception.function = function_name
    exception.strerror = description
    return exception


def check_null(result: int | None, function: _NamedFuncPointer, *_) -> int:
    if result is None:
        raise make_error(function)
    return result


def check_false(result: int | None, function: _NamedFuncPointer, *_) -> Literal[True]:
    if not bool(result):
        raise make_error(function)
    else:
        return True


###
# https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/core/cffi/_resource.py
###


def _UpdateResource(
    hUpdate: int,
    lpType: str | int,
    lpName: str | int,
    wLanguage: int,
    lpData: bytes,
    cbData: int,
):
    lp_type = LPCWSTR(lpType)
    lp_name = LPCWSTR(lpName)
    _BaseUpdateResource(hUpdate, lp_type, lp_name, wLanguage, lpData, cbData)


_BeginUpdateResource = function_factory(
    kernel32.BeginUpdateResourceW,
    [LPCWSTR, BOOL],
    HANDLE,
    check_null,
)


_EndUpdateResource = function_factory(
    kernel32.EndUpdateResourceW,
    [HANDLE, BOOL],
    BOOL,
    check_false,
)

_BaseUpdateResource = function_factory(
    kernel32.UpdateResourceW,
    [HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD],
    BOOL,
    check_false,
)


###
# https://github.com/enthought/pywin32-ctypes/blob/main/win32ctypes/pywin32/win32api.py
###

LANG_NEUTRAL = 0x00


def BeginUpdateResource(filename: str, delete: bool):
    """Get a handle that can be used by the :func:`UpdateResource`.

    Parameters
    ----------
    fileName : unicode
        The filename of the module to load.
    delete : bool
        When true all existing resources are deleted

    Returns
    -------
    result : hModule
        Handle of the resource.

    """
    return _BeginUpdateResource(filename, delete)


def EndUpdateResource(handle: int, discard: bool) -> None:
    """End the update resource of the handle.

    Parameters
    ----------
    handle : hModule
        The handle of the resource as it is returned
        by :func:`BeginUpdateResource`

    discard : bool
        When True all writes are discarded.

    """
    _EndUpdateResource(handle, discard)


def UpdateResource(
    handle: int,
    type: str | int,
    name: str | int,
    data: Iterable[SupportsIndex] | SupportsIndex | SupportsBytes | ReadableBuffer,
    language=LANG_NEUTRAL,
) -> None:
    """Update a resource.

    Parameters
    ----------
    handle : hModule
        The handle of the resource file as returned by
        :func:`BeginUpdateResource`.

    type : str : int
        The type of resource to update.

    name : str : int
        The name or Id of the resource to update.

    data : bytes
        A bytes like object is expected.

        .. note::
          PyWin32 version 219, on Python 2.7, can handle unicode inputs.
          However, the data are stored as bytes and it is not really
          possible to convert the information back into the original
          unicode string. To be consistent with the Python 3 behaviour
          of PyWin32, we raise an error if the input cannot be
          converted to `bytes`.

    language : int
        Language to use, default is LANG_NEUTRAL.

    """
    try:
        lp_data = bytes(data)
    except UnicodeEncodeError:
        raise TypeError("a bytes-like object is required, not a 'unicode'")
    _UpdateResource(handle, type, name, language, lp_data, len(lp_data))
