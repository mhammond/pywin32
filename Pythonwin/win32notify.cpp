/*

	win32 Notify Handler

	Created May 1995, Mark Hammond (MHammond@skippinet.com.au)

*/
#include "stdafx.h"
#include "win32win.h"

// Not very general purpose notify parser!
PyObject *PyNotifyMakeExtraTuple( NMHDR *ptr, char *fmt)
{
	char *pUse = (char *)(ptr+1);
	int argNo = 0;
	int tupleLen = 0;
	for (char *szTemp = fmt;*szTemp; ++szTemp) {
		if (*szTemp == '-')
			++szTemp; // skip next one.
		else if (isdigit(*szTemp))
			; // ignore it.
		else
			++tupleLen; // count it
	}
	PyObject *ret = PyTuple_New(tupleLen);
	PyObject *ob;
	BOOL bIgnore;
	while (*fmt) {
#ifdef _DEBUG
		ob = NULL;
#endif
		bIgnore = *fmt=='-';
		if (bIgnore) ++fmt;
		switch (*fmt) {
		case 'i':
			ob = bIgnore ? NULL : PyInt_FromLong( * ((int *)pUse) );
			pUse += (sizeof(int));
			break;
		case 'P': { // point
			LONG l1 = * ((LONG *)pUse);
			pUse += (sizeof(long));
			LONG l2 = * ((LONG *)pUse);
			pUse += (sizeof(long));
			ob = bIgnore ? NULL : Py_BuildValue("ll", l1, l2);
			break;
			}
		case 'z': // string pointer
		case 's': // string buffer - same for this parse
			{
			char *use = (*fmt=='z') ? * (char **) pUse : pUse;
			ob = bIgnore ? NULL : PyString_FromString(""); // HACK HACK - FIX ME FIX ME
			if (*fmt=='s') { // followed by buffer size;
				int val = 0;
				while (fmt[1] && isdigit(fmt[1])) {
					val = val * 10 + (fmt[1]-'0');
					fmt++;
				}
				pUse += sizeof(char) * val;
			} else {
				pUse += sizeof(char *);
			}
			break;
			}
		case 'Z': // Unicode string pointer
		case 'S': // Unicode buffer - same for this parse
			{
			char *use = (*fmt=='Z') ? * (char **) pUse : pUse;
			ob = bIgnore ? NULL : PyString_FromString(""); // HACK HACK - FIX ME FIX ME
			if (*fmt=='S') { // followed by buffer size;
				int val = 0;
				while (fmt[1] && isdigit(fmt[1])) {
					val = val * 10 + (fmt[1]-'0');
					fmt++;
				}
				pUse += sizeof(wchar_t) * val;
			} else {
				pUse += sizeof(wchar_t *);
			}
			break;
			}

		case 'O': // object with no reference count maintained
			ob = bIgnore ? NULL : (PyObject *)pUse;
			Py_INCREF(ob);
			pUse += (sizeof(PyObject *));
			break;
		case 'T': {// TV_ITEM structure
			TV_ITEM *ptv = (TV_ITEM *)pUse;
			ob = bIgnore ? NULL : MakeTV_ITEMTuple(ptv);
			pUse += (sizeof(TV_ITEM));
			break;
			}
		case 'L': {// LV_ITEM structure
			LV_ITEM *plv = (LV_ITEM *)pUse;
			ob = bIgnore ? NULL : MakeLV_ITEMTuple(plv);
			pUse += (sizeof(LV_ITEM));
			break;
			}
			/*
		case 'H': {// HD_ITEM structure
			HD_ITEM *phd = (HD_ITEM *)pUse;
			ob = bIgnore ? NULL : MakeHD_ITEMTuple(phd);
			pUse += (sizeof(HD_ITEM));
			break;
			}
			*/
		default:
			ASSERT(FALSE);
			Py_DECREF(ret);
			RETURN_ERR("Bad format char in internal WM_NOTIFY tuple conversion");
		}
		if (!bIgnore) {
			PyTuple_SET_ITEM(ret, argNo, ob);
			argNo++;
		}
		ASSERT(bIgnore==FALSE || ob==NULL); // check bIgnore logic
		fmt++;
	}
	return ret;
}

#define MY_RET_ERR(msg) { PyErr_SetString(ui_module_error, msg); return;}
// Not very general purpose notify parser!
void PyNotifyParseExtraTuple( NMHDR *ptr, PyObject *args,  char *fmt)
{
	char *pUse = (char *)(ptr+1);
	BOOL bIgnore;
	int argNum = 0;
	while (*fmt) {
		PyObject *ob = PyTuple_GetItem(args, argNum);
		if (ob==NULL) return;
		bIgnore = *fmt=='-';
		if (bIgnore) ++fmt;
		switch (*fmt) {
		case 'i':
			if (!bIgnore) {
				if (!PyInt_Check(ob)) MY_RET_ERR("Expected integer object")
				*((int *)pUse) = PyInt_AsLong(ob);
			}
			pUse += (sizeof(int));
			break;
		case 'P': { // point
			ASSERT(FALSE);
			break;
			}
		case 'T': { // TV_ITEM
			ASSERT(FALSE);
			break;
			}
		case 'z': // string pointer
			if (!bIgnore) {
				ASSERT(FALSE);
			}
			pUse += (sizeof(char *));
			break;
		case 'Z': // wide string pointer
			if (!bIgnore) {
				ASSERT(FALSE);
			}
			pUse += (sizeof(wchar_t *));
			break;
		case 's': // string buffer
			{
			int bufSize = 0;
			while (fmt[1] && isdigit(fmt[1])) {
				bufSize = bufSize * 10 + (fmt[1]-'0');
				fmt++;
			}
			ASSERT(bufSize);
			if (!bIgnore) {
				if (!PyString_Check(ob)) MY_RET_ERR("Expected string object")
				char *val = PyString_AsString(ob);
				int slen = strlen(val);
				int copylen = max(bufSize-1, slen);
				strncpy( pUse, val, copylen);
				pUse[copylen] = '\0';
			}
			pUse += bufSize;
			break;
			}
		case 'S': // string buffer
			{
			int bufSize = 0;
			while (fmt[1] && isdigit(fmt[1])) {
				bufSize = bufSize * 10 + (fmt[1]-'0');
				fmt++;
			}
			ASSERT(bufSize);
			if (!bIgnore) {
				if (!PyString_Check(ob)) MY_RET_ERR("Expected string object")
				char *szVal = PyString_AsString(ob);
				int slen = strlen(szVal);
				mbstowcs( (wchar_t *)pUse, szVal, bufSize );
			}
			pUse += bufSize + sizeof(wchar_t);
			break;
			}
		case 'O': // object with no reference count maintained
			ASSERT(FALSE);
			break;
		default:
			ASSERT(FALSE);
			MY_RET_ERR("Bad format char in internal WM_NOTIFY tuple conversion");
		}
		fmt++;
		if (!bIgnore)
			argNum ++;
	}
	return;
}

///////////////////////////////////////////////////////
// General notify handler for Python.
BOOL 
Python_OnNotify (CWnd *pFrom, WPARAM, LPARAM lParam, LRESULT *pResult)
{
	CEnterLeavePython _celp;
	PyCCmdTarget *pPyWnd = (PyCCmdTarget *) ui_assoc_CObject::GetPyObject(pFrom);
	NMHDR *pHdr = (NMHDR *)lParam;
	if (pHdr==NULL) return FALSE; // bad data passed?
	UINT code = pHdr->code;
	if (pPyWnd==NULL) return FALSE; // no object.
	if (!pPyWnd->is_uiobject (&PyCWnd::type)) return FALSE; // unexpected object type.
	PyObject *method;

	if (!pPyWnd->pNotifyHookList || 
		!pPyWnd->pNotifyHookList->Lookup (code, (void *&)method))
		return FALSE; // no hook installed.

	// have method to call.  Build arguments.
	PyObject *ob1 = Py_BuildValue("iii", pHdr->hwndFrom, pHdr->idFrom, pHdr->code);
	char *fmt;
	/*
	if (code >= LVN_LAST && code <= LVN_FIRST) // These are negative, hence the reversal.
		fmt = "iiiiiPi";						//this is a NM_LISTVIEW
	else if (code >= TVN_LAST && code <= TVN_FIRST) // These are negative, hence the reversal.
		fmt = "iTTP";							//this is NM_TREEVIEW
	else if (code==TTN_NEEDTEXTA)
		fmt = "-zs80ii";
	else if (code==TTN_NEEDTEXTW)
		fmt = "-ZS80ii";
	else
		fmt = NULL;
	*/
	if (code >= UDN_LAST && code <= UDN_FIRST)
		fmt = "ii";		//NM_UPDOWN
	else if (code == TBN_GETBUTTONINFOW) 
		fmt = "iiibbiiiZ";		//TBNOTIFY
	else if (code == TBN_QUERYDELETE || code == TBN_QUERYINSERT || (code >= TBN_ENDDRAG && code <= TBN_FIRST ))
		fmt = "iiibbiiiz";
	else if (code == TBN_CUSTHELP || code == TBN_TOOLBARCHANGE || (code >= TBN_RESET && code <= TBN_BEGINADJUST))
		fmt = NULL;		//NMHDR only
	else if (code >= TCN_LAST && code <= TCN_SELCHANGE)
		fmt = "i";		//HWND
	else if (code == TCN_KEYDOWN)
		fmt = "ii";		//TC_KEYDOWN
	else if (code == TTN_NEEDTEXTW)
		fmt = "-ZS80ii";	//TOOLTIPTEXT
	else if (code == TTN_POP || code == TTN_SHOW)
		fmt = NULL;		//NMHDR only
	else if (code == TTN_NEEDTEXTA)
		fmt = "-zs80ii";	//TOOLTIPTEXT
	else if (code == TVN_ENDLABELEDITW || code == TVN_BEGINLABELEDITW || code == TVN_SETDISPINFOW
			|| code == TVN_GETDISPINFOW || code == TVN_ENDLABELEDITA || code == TVN_BEGINLABELEDITA
			|| code == TVN_SETDISPINFOA || code == TVN_GETDISPINFOA)
		fmt = "T";		//TV_DISPINFO
	else if (code == TVN_KEYDOWN)
		fmt = "ii";		//TV_KEYDOWN
	else if (code >= TVN_LAST && code <= TVN_FIRST)
		fmt = "iTTP";	//NM_TREEVIEW
	else if (code == HDN_ITEMDBLCLICKW || code == HDN_ITEMDBLCLICKA)
		fmt = NULL;		//NMHDR only
	else if (code >= HDN_LAST && code <= HDN_FIRST)
		fmt = "iiH";	//HD_NOTIFY
	else if (code == LVN_KEYDOWN)
		fmt = "ii";		//LV_KEYDOWN
	else if ((code >= LVN_LAST && code <= LVN_GETDISPINFOW) || code == LVN_ENDLABELEDITA || code == LVN_BEGINLABELEDITA)
		fmt = "L";		//LV_DISPINFO
	else if (code >= LVN_BEGINRDRAG && code <= LVN_FIRST)
		fmt = "iiiiiPi";		//NM_LISTVIEW
	else
		fmt = NULL;
	
	PyObject *ob2;
	if (fmt==NULL)
		ob2 = PyInt_FromLong((int)(pHdr + 1));
	else
		ob2 = PyNotifyMakeExtraTuple(pHdr, fmt);
	if (ob2==NULL) {
		gui_print_error();
		return FALSE;
	}

	// make the call with my params.
	PyObject *args = Py_BuildValue("OO", ob1, ob2);
	Py_DECREF(ob1);
	Py_DECREF(ob2);
	int rc = 0;
	BOOL bPassOn = FALSE;
	PyObject *obOther;
	PyObject *result = Python_do_callback(method, args);
	if (result==NULL) {
		PyErr_SetString(ui_module_error, "Exception in OnNotify() handler");
		gui_print_error();
	} else if (result==Py_None)	// allow for None "dont pass on", else result to windows
		bPassOn = TRUE;
	else if (PyInt_Check(result)) {
		// Simple integer return val
		rc = PyInt_AsLong(result);
	} else if (PyArg_ParseTuple(result, "iO", &rc, &obOther)) {
		// parse off obOther
		PyErr_Clear();
		PyNotifyParseExtraTuple( pHdr, obOther, fmt);
		if (PyErr_Occurred()) {
			gui_print_error();
			PyErr_SetString(ui_module_error, "Error parsing OnNotify() extra return info");
			gui_print_error();
		}
	} else {
		PyErr_SetString(ui_module_error, "Unable to parse result from OnNotify()");
		gui_print_error();
		rc = 0;
	}
	Py_XDECREF(result);
	*pResult = rc;
	return !bPassOn;
}
