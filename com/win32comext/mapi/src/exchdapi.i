/* File : exchdapi.i */

%module exchdapi // An COM interface to Exchange's DAPI

//%{
//#define UNICODE
//%}


%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"

#include <mapiutil.h>
#include "EDKMDB.H"
#include "EDKMAPI.H"
#include "EDKCFG.H"
#include "EXCHINST.H"
#include "EDKUTILS.H"
// see exchange.i - the stand-alone exchange SDK seems to be
// missing lots of functions
#if defined(EXCHANGE_RE)
#	define DAPI_FUNCTIONS_MISSING
#endif

%}

// @pyswig |HrInstallService|
%{
HRESULT MyHrInstallService(
	TCHAR *lpszServer,
	TCHAR *lpszSiteDN,
	TCHAR *pszServiceDisplayName,
	TCHAR *lpszServiceName,
	TCHAR *lpszCommonName,
	TCHAR *lpszObjectGuid,
	TCHAR *lpszProductGuid,
	TCHAR *lpszExeName,
	TCHAR *lpszDependencies,
	TCHAR *lpszAccount,
	TCHAR *lpszPassword
)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrInstallService(lpszServer, lpszSiteDN, pszServiceDisplayName, 
	                        lpszServiceName, lpszCommonName, lpszObjectGuid,
	                        lpszProductGuid, lpszExeName, lpszDependencies,
	                        lpszAccount, lpszPassword);
#endif
}
%}

%name(HrInstallService) HRESULT MyHrInstallService(
	TCHAR *INPUT, // lpszServer
	TCHAR *INPUT, // lpszSiteDN
	TCHAR *INPUT, // lpszServiceDisplayName
	TCHAR *INPUT, // lpszServiceName
	TCHAR *INPUT, // lpszCommonName
	TCHAR *INPUT, // lpszObjectGuid
	TCHAR *INPUT, // lpszProductGuid
	TCHAR *INPUT, // lpszExeName
	TCHAR *INPUT_NULLOK, // lpszDependencies
	TCHAR *INPUT, // lpszAccount
	TCHAR *INPUT_NULLOK // lpszPassword
); 


// @pyswig |HrInstallMailboxAgent|
%{
HRESULT MyHrInstallMailboxAgent( 
	TCHAR *Server,
	TCHAR *SiteDN,
	TCHAR *DisplayName,
	TCHAR *RDN,
	TCHAR *lpszExtensionName,
	TCHAR *lpszExtensionData,
	TCHAR *lpszAccountName)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrInstallMailboxAgent(Server, SiteDN, DisplayName, RDN, 
	                               lpszExtensionName, lpszExtensionData, 
	                               lpszAccountName);
#endif
}
%}
%name(HrInstallMailboxAgent) HRESULT MyHrInstallMailboxAgent( 
	TCHAR *INPUT, // Server
	TCHAR *INPUT, // SiteDN
	TCHAR *INPUT, // DisplayName
	TCHAR *INPUT, // RDN
	TCHAR *INPUT_NULLOK, // lpszExtensionName
	TCHAR *INPUT_NULLOK, // lpszExtensionData
	TCHAR *INPUT_NULLOK // lpszAccountName
);

// @pyswig |HrCreateMailboxAgentProfile|
HRESULT HrCreateMailboxAgentProfile(
	char *INPUT, // @pyparm string|serviceName||The name of the service.
	char *INPUT // @pyparm string|profile||The profile.
);

// @pyswig |HrCreateGatewayProfile|
HRESULT HrCreateGatewayProfile(
	char *INPUT, // @pyparm string|serviceName||The name of the service.
	char *INPUT // @pyparm string|profile||The profile.
);


// @pyswig |HrMailboxAgentExists|
%{
HRESULT MyHrMailboxAgentExists(
	TCHAR *server,
	TCHAR *siteDN,
	TCHAR *rdn)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrMailboxAgentExists(server, siteDN, rdn);
#endif
}
%}

%name(HrMailboxAgentExists) HRESULT MyHrMailboxAgentExists(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Contains the distinguished name (DN) of the site.
	TCHAR *INPUT // @pyparm string|rdn||RDN of the site where the mailbox agent might exist.
);


// @pyswig |HrAdminProgramExists|
HRESULT HrAdminProgramExists();

// @pyswig |HrRemoveMailboxAgent|Removes a Mailbox Agent
%{
HRESULT MyHrRemoveMailboxAgent(
	TCHAR *server,
	TCHAR *siteDN,
	TCHAR *rdn)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrRemoveMailboxAgent(server, siteDN, rdn);
#endif
}
%}

%name(HrRemoveMailboxAgent) HRESULT MyHrRemoveMailboxAgent(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Contains the distinguished name (DN) of the site.
	TCHAR *INPUT // @pyparm string|rdn||RDN of the site where the mailbox agent exists.
);

// @pyswig |HrRemoveProfile|Removes a profile
HRESULT HrRemoveProfile(
	char *INPUT // @pyparm string|profile||The profile to delete.
);

// @pyswig [string, ...]|HrEnumOrganizations|Lists the names of the organizations on the server.
%{
HRESULT MyHrEnumOrganizations(
	TCHAR *p1,
	TCHAR *p2,
	TCHAR **out)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrEnumOrganizations(p1, p2, out);
#endif
}
%}
%name(HrEnumOrganizations) HRESULT MyHrEnumOrganizations(
	TCHAR *INPUT_NULLOK, // @pyparm string|rootDN||Contains the distinguished name (DN) of the directory information tree (DIT) root.
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR **OUTPUT_ARRAY // lppszOrganizations 
); 

// @pyswig [string, ...]|HrEnumSites|Lists the names of the sites in an organization.
%{
HRESULT MyHrEnumSites(
	TCHAR *p1,
	TCHAR *p2,
	TCHAR **out)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrEnumSites(p1, p2, out);
#endif
}
%}
%name(HrEnumSites) HRESULT MyHrEnumSites(
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR *INPUT_NULLOK, // @pyparm string|organizationDN||Contains the distinguished name (DN) of the organization.
	TCHAR **OUTPUT_ARRAY // lppszSites
); 

// @pyswig [string, ...]|HrEnumContainers|Lists the names of the containers on the server
%{
HRESULT MyHrEnumContainers(
	TCHAR *p1,
	TCHAR *p2,
	BOOL b,
	TCHAR **out)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrEnumContainers(p1, p2, b, out);
#endif
}
%}
%name(HrEnumContainers) HRESULT MyHrEnumContainers(
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Distinguished name (DN) of the site.
	BOOL fSubtree, // @pyparm int|fSubtree||
	TCHAR **OUTPUT_ARRAY // lppszContainers
);

// @pyswig [string, ...]|HrEnumSiteAdmins|Lists the administrators for a site.
%{
HRESULT MyHrEnumSiteAdmins(
	TCHAR *p1,
	TCHAR *p2,
	TCHAR **out)
{
#if defined(DAPI_FUNCTIONS_MISSING)
	PyErr_Warn(PyExc_RuntimeWarning, "Not available with this version of the Exchange SDK");
	return E_NOTIMPL;
#else
	return HrEnumSiteAdmins(p1, p2, out);
#endif
}
%}
%name(HrEnumSiteAdmins) HRESULT MyHrEnumSiteAdmins(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Distinguished name (DN) of the site.
	TCHAR **OUTPUT_ARRAY // lppszAdmins
);

// @pyswig string|HrGetServiceAccountName|Obtains the account name for a service.
HRESULT HrGetServiceAccountName(
	char *INPUT, // @pyparm string|serviceName||The name of the service
	char **OUTPUT_MAPI // @pyparm string|serviceName||The name of the service
);
