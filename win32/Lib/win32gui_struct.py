# This is a work in progress - see Demos/win32gui_menu.py

# win32gui_struct.py - helpers for working with various win32gui structures.
# As win32gui is "light-weight", it does not define objects for all possible
# win32 structures - in general, "buffer" objects are passed around - it is
# the callers responsibility to pack the buffer in the correct format.
#
# This module defines some helpers for the commonly used structures.
#
# In general, each structure has 3 functions:
#
# buffer, extras = PackSTRUCTURE(items, ...)
# item, ... = UnpackSTRUCTURE(buffer)
# buffer, extras = EmtpySTRUCTURE(...)
#
# 'extras' is always items that must be held along with the buffer, as the
# buffer refers to these object's memory.
# For structures that support a 'mask', this mask is hidden from the user - if
# 'None' is passed, the mask flag will not be set, or on return, None will
# be returned for the value if the mask is not set.
#
# NOTE: I considered making these structures look like real classes, and
# support 'attributes' etc - however, ctypes already has a good structure
# mechanism - I think it makes more sense to support ctype structures
# at the win32gui level, then there will be no need for this module at all.

import win32gui
import win32con
import struct
import array

# MENUITEMINFO struct
# http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/Resources/Menus/MenuReference/MenuStructures/MENUITEMINFO.asp
# We use the struct module to pack and unpack strings as MENUITEMINFO
# structures.  We also have special handling for the 'fMask' item in that
# structure to avoid the caller needing to explicitly check validity
# (None is used if the mask excludes/should exclude the value)
menuitem_fmt = '9IP2I'

def PackMENUITEMINFO(fType=None, fState=None, wID=None, hSubMenu=None,
                     hbmpChecked=None, hbmpUnchecked=None, dwTypeData=None,
                     text=None, hbmpItem=None):
    # 'extras' are objects the caller must keep a reference to (as their
    # memory is used) for the lifetime of the INFO item.
    extras = []
    fMask = 0
    if fType is None: fType = 0
    else: fMask |= win32con.MIIM_FTYPE
    if fState is None: fState = 0
    else: fMask |= win32con.MIIM_STATE
    if wID is None: wID = 0
    else: fMask |= win32con.MIIM_ID
    if hSubMenu is None: hSubMenu = 0
    else: fMask |= win32con.MIIM_SUBMENU
    if hbmpChecked is None:
        assert hbmpUnchecked is None, \
                "neither or both checkmark bmps must be given"
        hbmpChecked = hbmpUnchecked = 0
    else:
        assert hbmpUnchecked is not None, \
                "neither or both checkmark bmps must be given"
        fMask |= win32con.MMIM_CHECKMARKS
    if dwTypeData is None: dwTypeData = 0
    else: fMask |= win32con.MIIM_DATA
    if hbmpItem is None: hbmpItem = 0
    else: fMask |= win32con.MIIM_BITMAP
    if text is not None:
        fMask |= win32con.MIIM_STRING
        str_buf = array.array("c", text+'\0')
        cch = len(str_buf)
        # We are taking address of strbuf - it must not die until windows
        # has finished with our structure.
        lptext = str_buf.buffer_info()[0]
        extras.append(str_buf)
    else:
        lptext = 0
        cch = 0
    # Create the struct.
    dwItemData = 0
    item = struct.pack(
                menuitem_fmt,
                struct.calcsize(menuitem_fmt), # cbSize
                fMask,
                fType,
                fState,
                wID,
                hSubMenu,
                hbmpChecked,
                hbmpUnchecked,
                dwItemData,
                lptext,
                cch,
                hbmpItem
                )
    # Now copy the string to a writable buffer, so that the result
    # could be passed to a 'Get' function
    return array.array("c", item), extras

def UnpackMENUITEMINFO(s):
    cb,
    fMask,
    fType,
    fState,
    wID,
    hSubMenu,
    hbmpChecked,
    hbmpUnchecked,
    dwItemData,
    lptext,
    cch,
    hbmpItem = struct.unpack(menuitem_fmt, s)
    assert cb==len(s)
    if fMask & win32con.MIIM_FTYPE==0: fType = None
    if fMask & win32con.MIIM_STATE==0: fState = None
    if fMask & win32con.MIIM_ID==0: wID = None
    if fMask & win32con.MIIM_SUBMENU==0: hSubMenu = None
    if fMask & win32con.MIIM_CHECKMARKS==0: hbmpChecked = hbmpUnchecked = None
    if fMask & win32con.MIIM_DATA==0: dwItemData = None
    if fMask & win32con.MIIM_BITMAP==0: hbmpItem = None
    if fMask & win32con.MIIM_STRING:
        text = PyGetString(lptext, cch)
    else:
        text = None
    return fType, fState, wID, hSubMenu, bmpChecked, bmpUnchecked, \
           dwTypeData, text, hbmpItem

##########################################################################
#
# Tree View structure support - TVITEM, TVINSERTSTRUCT and TVDISPINFO
# 
##########################################################################

# XXX - Note that the following implementation of TreeView structures is ripped
# XXX - from the SpamBayes project.  It may not quite work correctly yet - I
# XXX - intend checking them later - but having them is better than not at all!

# Helpers for the ugly win32 structure packing/unpacking
def _GetMaskAndVal(val, default, mask, flag):
    if val is None:
        return mask, default
    else:
        mask |= flag
        return mask, val

def PackTVINSERTSTRUCT(parent, insertAfter, tvitem):
    tvitem_buf, extra = PackTVITEM(*tvitem)
    tvitem_buf = tvitem_buf.tostring()
    format = "ii%ds" % len(tvitem_buf)
    return struct.pack(format, parent, insertAfter, tvitem_buf), extra

def PackTVITEM(hitem, state, stateMask, text, image, selimage, citems, param):
    extra = [] # objects we must keep references to
    mask = 0
    mask, hitem = _GetMaskAndVal(hitem, 0, mask, commctrl.TVIF_HANDLE)
    mask, state = _GetMaskAndVal(state, 0, mask, commctrl.TVIF_STATE)
    if not mask & commctrl.TVIF_STATE:
        stateMask = 0
    mask, text = _GetMaskAndVal(text, None, mask, commctrl.TVIF_TEXT)
    mask, image = _GetMaskAndVal(image, 0, mask, commctrl.TVIF_IMAGE)
    mask, selimage = _GetMaskAndVal(selimage, 0, mask, commctrl.TVIF_SELECTEDIMAGE)
    mask, citems = _GetMaskAndVal(citems, 0, mask, commctrl.TVIF_CHILDREN)
    mask, param = _GetMaskAndVal(param, 0, mask, commctrl.TVIF_PARAM)
    if text is None:
        text_addr = text_len = 0
    else:
        text_buffer = array.array("c", text+"\0")
        extra.append(text_buffer)
        text_addr, text_len = text_buffer.buffer_info()
    format = "iiiiiiiiii"
    buf = struct.pack(format,
                      mask, hitem,
                      state, stateMask,
                      text_addr, text_len, # text
                      image, selimage,
                      citems, param)
    return array.array("c", buf), extra

# Make a new buffer suitable for querying hitem's attributes.
def EmptyTVITEM(hitem, mask = None, text_buf_size=512):
    extra = [] # objects we must keep references to
    if mask is None:
        mask = commctrl.TVIF_HANDLE | commctrl.TVIF_STATE | commctrl.TVIF_TEXT | \
               commctrl.TVIF_IMAGE | commctrl.TVIF_SELECTEDIMAGE | \
               commctrl.TVIF_CHILDREN | commctrl.TVIF_PARAM
    if mask & commctrl.TVIF_TEXT:
        text_buffer = array.array("c", "\0" * text_buf_size)
        extra.append(text_buffer)
        text_addr, text_len = text_buffer.buffer_info()
    else:
        text_addr = text_len = 0
    format = "iiiiiiiiii"
    buf = struct.pack(format,
                      mask, hitem,
                      0, 0,
                      text_addr, text_len, # text
                      0, 0,
                      0, 0)
    return array.array("c", buf), extra
    
def UnpackTVItem(buffer):
    item_mask, item_hItem, item_state, item_stateMask, \
        item_textptr, item_cchText, item_image, item_selimage, \
        item_cChildren, item_param = struct.unpack("10i", buffer)
    # ensure only items listed by the mask are valid (except we assume the
    # handle is always valid - some notifications (eg, TVN_ENDLABELEDIT) set a
    # mask that doesn't include the handle, but the docs explicity say it is.)
    if not (item_mask & commctrl.TVIF_TEXT): item_textptr = item_cchText = None
    if not (item_mask & commctrl.TVIF_CHILDREN): item_cChildren = None
    if not (item_mask & commctrl.TVIF_IMAGE): item_image = None
    if not (item_mask & commctrl.TVIF_PARAM): item_param = None
    if not (item_mask & commctrl.TVIF_SELECTEDIMAGE): item_selimage = None
    if not (item_mask & commctrl.TVIF_STATE): item_state = item_stateMask = None
    
    if item_textptr:
        text = win32gui.PyGetString(item_textptr)
    else:
        text = None
    return item_hItem, item_state, item_stateMask, \
        text, item_image, item_selimage, \
        item_cChildren, item_param

# Unpack the lparm from a "TVNOTIFY" message
def UnpackTVNOTIFY(lparam):
    format = "iiii40s40s"
    buf = win32gui.PyMakeBuffer(struct.calcsize(format), lparam)
    hwndFrom, id, code, action, buf_old, buf_new \
          = struct.unpack(format, buf)
    item_old = UnpackTVItem(buf_old)
    item_new = UnpackTVItem(buf_new)
    return hwndFrom, id, code, action, item_old, item_new

def UnpackTVDISPINFO(lparam):
    format = "iii40s"
    buf = win32gui.PyMakeBuffer(struct.calcsize(format), lparam)
    hwndFrom, id, code, buf_item = struct.unpack(format, buf)
    item = UnpackTVItem(buf_item)
    return hwndFrom, id, code, item
