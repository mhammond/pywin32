// @doc

/*
//////////////////////////////////////////////////////////////////////

  Changes made by Chris Tismer
  ----------------------------

  CT980913 (after a week of hacking)

  - Added Support for DDE_Poke and DDE_Request.
  - Changed semantics of DDE_Items.
  - Added support for multiple servers.

  Details:
  --------
  A DDE Topic is still a fixed property.
  For DDE Items, we now allow registering an empty string as a
  DDE_StringItem. 
  The semantic is this: Whenever an empty string item is registered,
  the server will respond to any item for this topic and ignore 
  the internal list. Instead, the item must be evaluated by the 
  Python server class.

  Reason: Netscape makes heavy usage of string items and does not use
    them in a fixed manner. Instead, the items are (ab)used as the
    parameter list in DDE_Poke and DDE_Request.

  Server issues (Server):
  -----------------------
  Poke and Request are now exposed to the Python server interface.
  Poke    - does not require a result
          - has an optional value parameter (not used by Netscape)
  Request - should return a string object

  Client issues (Conversation):
  -----------------------------
  Poke and Request are now available.
  Poke (item, value=None)
		  - sends a Poke to it's conversation's server
          - has an optional value parameter (not used by Netscape)
		  - does not return anything
  Request (item)
		  - sends a request to it's conversation's server
		  - returns a string object.
		  - This object may be binary, depending of the server.

  About multiple servers:
  -----------------------
  Multiple servers (and clients) can now be created in one session.
  Especially, DDE can now be used from within PythonWin.
  The implementation was a bit hairy (see #ifdef _CALLHACK_ entries
  in stddde.h/stddde.cpp) and can be switched off.
  Instead of a single pTheServer variable, we use a pTheServerList
  variable which holds a circular list of DDE servers. Each has its
  own callback function, implemented by a row of code bytes patched
  into a server variable for this.

  Things left to do:
  ------------------
  - Check if servers really get destroyed when you "del" them. I could
	not make this sure and had the impression that del does nothing,
	although refcount showed "2" in Python.
  - Maybe the "Destroy" method should be removed, and the shutdown
    mechanism should be improved. Anyway, it seems to work if you
	destroy a server and then "del" it.
  - For a future version, I would try to remove more of the internals
    and go towards an almost Python solution. It should also be 
	possible to go without the dependency of win32ui.pyd.

  - But after all this is now working quite well, let's keep it as it
	is and use it :-)

  Thanks to the author Mark Hammond and all other contributors to 
  PythonWin. I hope I didn't spoil his work with my hacking.

  ciao - pirx

//////////////////////////////////////////////////////////////////////
*/

#include "stdafxdde.h"
#include "ddemodule.h"

static char BASED_CODE szModName[] = "dde";
static char BASED_CODE errorName[] = "error";
PyObject * BASED_CODE dde_module_error = NULL;

PyObject *PyDDE_CreateServer(PyObject *s, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":CreateServer")) return NULL;
	GUI_BGN_SAVE;
	PythonDDEServer *pS = new PythonDDEServer;
	GUI_END_SAVE;
	if (pS==NULL)
		RETURN_MEM_ERR("allocating PythonDDEServer");
	return ui_assoc_object::make(PyDDEServer::type, pS);
}

PyObject *PyDDE_CreateTopic(PyObject *s, PyObject *args)
{
	TCHAR *name;
	PyObject *obname;
	if (!PyArg_ParseTuple(args,"O:CreateTopic", &obname)) return NULL;
	if (!PyWinObject_AsTCHAR(obname, &name, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	PythonDDETopic *pNew = new PythonDDETopic;
	pNew->Create(name);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(name);
	return ui_assoc_object::make(PyDDETopic::type, pNew);
}

PyObject *PyDDE_CreateServerSystemTopic(PyObject *s, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":CreateServerSystemTopic")) return NULL;
	GUI_BGN_SAVE;
	PythonDDEServerSystemTopic *pNew = new PythonDDEServerSystemTopic;
	GUI_END_SAVE;
	return ui_assoc_object::make(PyDDEServerSystemTopic::type, pNew);
}

PyObject *PyDDE_CreateConversation(PyObject *s, PyObject *args)
{
	PyObject *obServer;
	if (!PyArg_ParseTuple(args,"O:CreateConversation", &obServer)) return NULL;
	PythonDDEServer *pS = PyDDEServer::GetServer(obServer);
	if (pS==NULL) return NULL;
	GUI_BGN_SAVE;
	PythonDDEConv *pNew = new PythonDDEConv(pS);
	GUI_END_SAVE;
	return ui_assoc_object::make(PyDDEConv::type, pNew);
}

PyObject *PyDDE_CreateStringItem(PyObject *s, PyObject *args)
{
	TCHAR *name;
	PyObject *obname;
	if (!PyArg_ParseTuple(args,"O:CreateStringItem", &obname)) return NULL;
	if (!PyWinObject_AsTCHAR(obname, &name, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	PythonDDEStringItem *pNew = new PythonDDEStringItem;
	pNew->Create(name);
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(name);
	return ui_assoc_object::make(PyDDEStringItem::type, pNew);
}

// @module dde|A module for Dynamic Data Exchange support
static struct PyMethodDef dde_functions[] =
{
	{"CreateConversation", PyDDE_CreateConversation, 1},
	{"CreateServer", PyDDE_CreateServer, 1},
	{"CreateServerSystemTopic", PyDDE_CreateServerSystemTopic, 1},
	{"CreateTopic", PyDDE_CreateTopic, 1},
	{"CreateStringItem", PyDDE_CreateStringItem, 1},
	{NULL,  NULL}
};


#define ADD_CONSTANT(tok) if (PyModule_AddIntConstant(module, #tok, tok)) PYWIN_MODULE_INIT_RETURN_ERROR;

PYWIN_MODULE_INIT_FUNC(dde)
{
	if (AfxGetApp()==NULL) {
		PyErr_SetString(PyExc_ImportError, "This must be an MFC application - try 'import win32ui' first");
		PYWIN_MODULE_INIT_RETURN_ERROR;
	}
	PYWIN_MODULE_INIT_PREPARE(dde, dde_functions,
	                          "A module for Dynamic Data Exchange support");

	dde_module_error = PyErr_NewException("dde.error", NULL, NULL);
	PyDict_SetItemString(dict, "error", dde_module_error);

	ADD_CONSTANT(APPCLASS_MONITOR); // Makes it possible for the application to monitor DDE activity in the system. This flag is for use by DDE monitoring applications. The application specifies the types of DDE activity to monitor by combining one or more monitor flags with the APPCLASS_MONITOR flag. For details, see the following Remarks section. 
	ADD_CONSTANT(APPCLASS_STANDARD); // Registers the application as a standard (nonmonitoring) DDEML application. 
	ADD_CONSTANT(APPCMD_CLIENTONLY); // Prevents the application from becoming a server in a DDE conversation. The application can only be a client. This flag reduces consumption of resources by the DDEML. It includes the functionality of the CBF_FAIL_ALLSVRXACTIONS flag. 
	ADD_CONSTANT(APPCMD_FILTERINITS); // Prevents the DDEML from sending XTYP_CONNECT and XTYP_WILDCONNECT transactions to the application until the application has created its string handles and registered its service names or has turned off filtering by a subsequent call to the DdeNameService or DdeInitialize function. This flag is always in effect when an application calls DdeInitialize for the first time, regardless of whether the application specifies the flag. On subsequent calls to DdeInitialize, not specifying this flag turns off the application's service-name filters, but specifying it turns on the application's service name filters. 
	ADD_CONSTANT(CBF_FAIL_ALLSVRXACTIONS); // Prevents the callback function from receiving server transactions. The system returns DDE_FNOTPROCESSED to each client that sends a transaction to this application. This flag is equivalent to combining all CBF_FAIL_ flags. 
	ADD_CONSTANT(CBF_FAIL_ADVISES); // Prevents the callback function from receiving XTYP_ADVSTART and XTYP_ADVSTOP transactions. The system returns DDE_FNOTPROCESSED to each client that sends an XTYP_ADVSTART or XTYP_ADVSTOP transaction to the server. 
	ADD_CONSTANT(CBF_FAIL_CONNECTIONS); // Prevents the callback function from receiving XTYP_CONNECT and XTYP_WILDCONNECT transactions. 
	ADD_CONSTANT(CBF_FAIL_EXECUTES); // Prevents the callback function from receiving XTYP_EXECUTE transactions. The system returns DDE_FNOTPROCESSED to a client that sends an XTYP_EXECUTE transaction to the server. 
	ADD_CONSTANT(CBF_FAIL_POKES); // Prevents the callback function from receiving XTYP_POKE transactions. The system returns DDE_FNOTPROCESSED to a client that sends an XTYP_POKE transaction to the server. 
	ADD_CONSTANT(CBF_FAIL_REQUESTS); // Prevents the callback function from receiving XTYP_REQUEST transactions. The system returns DDE_FNOTPROCESSED to a client that sends an XTYP_REQUEST transaction to the server. 
	ADD_CONSTANT(CBF_FAIL_SELFCONNECTIONS); // Prevents the callback function from receiving XTYP_CONNECT transactions from the application's own instance. This flag prevents an application from establishing a DDE conversation with its own instance. An application should use this flag if it needs to communicate with other instances of itself but not with itself. 
	ADD_CONSTANT(CBF_SKIP_ALLNOTIFICATIONS); // Prevents the callback function from receiving any notifications. This flag is equivalent to combining all CBF_SKIP_ flags. 
	ADD_CONSTANT(CBF_SKIP_CONNECT_CONFIRMS); // Prevents the callback function from receiving XTYP_CONNECT_CONFIRM notifications. 
	ADD_CONSTANT(CBF_SKIP_DISCONNECTS); // Prevents the callback function from receiving XTYP_DISCONNECT notifications. 
	ADD_CONSTANT(CBF_SKIP_REGISTRATIONS); // Prevents the callback function from receiving XTYP_REGISTER notifications. 
//	ADD_CONSTANT(MF_SKIP_UNREGISTRATIONS); // Prevents the callback function from receiving XTYP_UNREGISTER notifications. 
	ADD_CONSTANT(MF_CALLBACKS); // Notifies the callback function whenever a transaction is sent to any DDE callback function in the system. 
	ADD_CONSTANT(MF_CONV); // Notifies the callback function whenever a conversation is established or terminated. 
	ADD_CONSTANT(MF_ERRORS); // Notifies the callback function whenever a DDE error occurs. 
	ADD_CONSTANT(MF_HSZ_INFO); // Notifies the callback function whenever a DDE application creates, frees, or increments the usage count of a string handle or whenever a string handle is freed as a result of a call to the DdeUninitialize function. 
	ADD_CONSTANT(MF_LINKS); // Notifies the callback function whenever an advise loop is started or ended. 
	ADD_CONSTANT(MF_POSTMSGS); // Notifies the callback function whenever the system or an application posts a DDE message. 
	ADD_CONSTANT(MF_SENDMSGS); // Notifies the callback function whenever the system or an application sends a DDE message. 

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
