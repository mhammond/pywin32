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
%}

%{

%}


// @pyswig |HrInstallService|
HRESULT HrInstallService(
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
HRESULT HrInstallMailboxAgent( 
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
	TCHAR *INPUT, // @pyparm string|serviceName||The name of the service.
	TCHAR *INPUT // @pyparm string|profile||The profile.
);

// @pyswig |HrCreateGatewayProfile|
HRESULT HrCreateGatewayProfile(
	TCHAR *INPUT, // @pyparm string|serviceName||The name of the service.
	TCHAR *INPUT // @pyparm string|profile||The profile.
);

// @pyswig |HrMailboxAgentExists|
HRESULT HrMailboxAgentExists(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Contains the distinguished name (DN) of the site.
	TCHAR *INPUT // @pyparm string|rdn||RDN of the site where the mailbox agent might exist.
);

// @pyswig |HrAdminProgramExists|
HRESULT HrAdminProgramExists();

// @pyswig |HrRemoveMailboxAgent|Removes a Mailbox Agent
HRESULT HrRemoveMailboxAgent(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Contains the distinguished name (DN) of the site.
	TCHAR *INPUT // @pyparm string|rdn||RDN of the site where the mailbox agent exists.
);

// @pyswig |HrRemoveProfile|Removes a profile
HRESULT HrRemoveProfile(
	TCHAR *INPUT // @pyparm string|profile||The profile to delete.
);

// @pyswig [string, ...]|HrEnumOrganizations|Lists the names of the organizations on the server.
HRESULT HrEnumOrganizations(
	TCHAR *INPUT_NULLOK, // @pyparm string|rootDN||Contains the distinguished name (DN) of the directory information tree (DIT) root.
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR **OUTPUT_ARRAY // lppszOrganizations 
); 

// @pyswig [string, ...]|HrEnumSites|Lists the names of the sites in an organization.
HRESULT HrEnumSites(
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR *INPUT_NULLOK, // @pyparm string|organizationDN||Contains the distinguished name (DN) of the organization.
	TCHAR **OUTPUT_ARRAY // lppszSites
); 

// @pyswig [string, ...]|HrEnumContainers|Lists the names of the containers on the server
HRESULT HrEnumContainers(
	TCHAR *INPUT_NULLOK, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Distinguished name (DN) of the site.
	BOOL fSubtree, // @pyparm int|fSubtree||
	TCHAR **OUTPUT_ARRAY // lppszContainers
);

// @pyswig [string, ...]|HrEnumSiteAdmins|Lists the administrators for a site.
HRESULT HrEnumSiteAdmins(
	TCHAR *INPUT, // @pyparm string|server||The name of the server
	TCHAR *INPUT, // @pyparm string|siteDN||Distinguished name (DN) of the site.
	TCHAR **OUTPUT_ARRAY // lppszAdmins
);

// @pyswig string|HrGetServiceAccountName|Obtains the account name for a service.
HRESULT HrGetServiceAccountName(
	TCHAR *INPUT, // @pyparm string|serviceName||The name of the service
	TCHAR **OUTPUT_MAPI // @pyparm string|serviceName||The name of the service
);