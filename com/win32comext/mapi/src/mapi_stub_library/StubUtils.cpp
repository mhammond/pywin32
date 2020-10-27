#include <Windows.h>
#include <strsafe.h>
#include <Msi.h>
#include <winreg.h>
#include <stdlib.h>

#if _MSC_VER < 1600
#define nullptr NULL
#endif

/*
 *  MAPI Stub Utilities
 *
 *	Public Functions:
 *
 *		GetPrivateMAPI()
 *			Obtain a handle to the MAPI DLL.  This function will load the MAPI DLL
 *			if it hasn't already been loaded
 *
 *		UnloadPrivateMAPI()
 *			Forces the MAPI DLL to be unloaded.  This can cause problems if the code
 *			still has outstanding allocated MAPI memory, or unmatched calls to
 *			MAPIInitialize/MAPIUninitialize
 *
 *		ForceOutlookMAPI()
 *			Instructs the stub code to always try loading the Outlook version of MAPI
 *			on the system, instead of respecting the system MAPI registration
 *			(HKLM\Software\Clients\Mail). This call must be made prior to any MAPI
 *			function calls.
 */
HMODULE GetPrivateMAPI();
void UnloadPrivateMAPI();
void ForceOutlookMAPI();

const WCHAR WszKeyNameMailClient[] = L"Software\\Clients\\Mail";
const WCHAR WszValueNameDllPathEx[] = L"DllPathEx";
const WCHAR WszValueNameDllPath[] = L"DllPath";

const CHAR SzValueNameMSI[] = "MSIComponentID";
const CHAR SzValueNameLCID[] = "MSIApplicationLCID";

const WCHAR WszOutlookMapiClientName[] = L"Microsoft Outlook";

const WCHAR WszMAPISystemPath[] = L"%s\\%s";

static const WCHAR WszOlMAPI32DLL[] = L"olmapi32.dll";
static const WCHAR WszMSMAPI32DLL[] = L"msmapi32.dll";
static const WCHAR WszMapi32[] = L"mapi32.dll";
static const WCHAR WszMapiStub[] = L"mapistub.dll";

static const CHAR SzFGetComponentPath[] = "FGetComponentPath";

// Sequence number which is incremented every time we set our MAPI handle which will
//  cause a re-fetch of all stored function pointers
volatile ULONG g_ulDllSequenceNum = 1;

// Whether or not we should ignore the system MAPI registration and always try to find
//  Outlook and its MAPI DLLs
static bool s_fForceOutlookMAPI = false;

static volatile HMODULE g_hinstMAPI = nullptr;

HMODULE GetMAPIHandle() { return g_hinstMAPI; }

void SetMAPIHandle(HMODULE hinstMAPI)
{
    const HMODULE hinstNULL = nullptr;
    HMODULE hinstToFree = nullptr;

    if (hinstMAPI == nullptr) {
        hinstToFree = static_cast<HMODULE>(InterlockedExchangePointer(
            const_cast<PVOID *>(reinterpret_cast<PVOID volatile *>(&g_hinstMAPI)), static_cast<PVOID>(hinstNULL)));
    }
    else {
        // Set the value only if the global is nullptr
        const HMODULE hinstPrev = static_cast<HMODULE>(InterlockedExchangePointer(
            const_cast<PVOID *>(reinterpret_cast<PVOID volatile *>(&g_hinstMAPI)), static_cast<PVOID>(hinstMAPI)));
        if (nullptr != hinstPrev) {
            hinstToFree = hinstMAPI;
        }

        // If we've updated our MAPI handle, any previous addressed fetched via GetProcAddress are invalid, so we
        // have to increment a sequence number to signal that they need to be re-fetched
        InterlockedIncrement(reinterpret_cast<volatile LONG *>(&g_ulDllSequenceNum));
    }
    if (nullptr != hinstToFree) {
        FreeLibrary(hinstToFree);
    }
}

/*
 *  RegQueryWszExpand
 *		Wrapper for RegQueryValueExW which automatically expands REG_EXPAND_SZ values
 */
DWORD RegQueryWszExpand(HKEY hKey, LPCWSTR lpValueName, LPWSTR lpValue, DWORD cchValueLen)
{
    DWORD dwType = 0;

    WCHAR rgchValue[MAX_PATH] = {0};
    DWORD dwSize = sizeof rgchValue;

    DWORD dwErr = RegQueryValueExW(hKey, lpValueName, nullptr, &dwType, reinterpret_cast<LPBYTE>(&rgchValue), &dwSize);

    if (dwErr == ERROR_SUCCESS) {
        if (dwType == REG_EXPAND_SZ) {
            // Expand the strings
            DWORD cch = ExpandEnvironmentStringsW(rgchValue, lpValue, cchValueLen);
            if ((0 == cch) || (cch > cchValueLen)) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
        }
        else if (dwType == REG_SZ) {
            wcscpy_s(lpValue, cchValueLen, rgchValue);
        }
    }

    return dwErr;
}

/*
 *  GetComponentPath
 *		Wrapper around mapi32.dll->FGetComponentPath which maps an MSI component ID to
 *		a DLL location from the default MAPI client registration values
 *
 */
bool GetComponentPath(LPCSTR szComponent, LPSTR szQualifier, LPSTR szDllPath, DWORD cchBufferSize, bool fInstall)
{
    bool fReturn = false;

    typedef bool(STDAPICALLTYPE * FGetComponentPathType)(LPCSTR, LPSTR, LPSTR, DWORD, bool);

    HMODULE hMapiStub = LoadLibraryW(WszMapi32);
    if (!hMapiStub)
        hMapiStub = LoadLibraryW(WszMapiStub);

    if (hMapiStub) {
        const FGetComponentPathType pFGetCompPath =
            reinterpret_cast<FGetComponentPathType>(GetProcAddress(hMapiStub, SzFGetComponentPath));

        fReturn = pFGetCompPath(szComponent, szQualifier, szDllPath, cchBufferSize, fInstall);

        FreeLibrary(hMapiStub);
    }

    return fReturn;
}

/*
 *  LoadMailClientFromMSIData
 *		Attempt to locate the MAPI provider DLL via HKLM\Software\Clients\Mail\(provider)\MSIComponentID
 */
HMODULE LoadMailClientFromMSIData(HKEY hkeyMapiClient)
{
    HMODULE hinstMapi = nullptr;
    CHAR rgchMSIComponentID[MAX_PATH] = {0};
    CHAR rgchMSIApplicationLCID[MAX_PATH] = {0};
    CHAR rgchComponentPath[MAX_PATH] = {0};
    DWORD dwType = 0;

    DWORD dwSizeComponentID = sizeof rgchMSIComponentID;
    DWORD dwSizeLCID = sizeof rgchMSIApplicationLCID;

    if (ERROR_SUCCESS == RegQueryValueExA(hkeyMapiClient, SzValueNameMSI, nullptr, &dwType,
                                          reinterpret_cast<LPBYTE>(&rgchMSIComponentID), &dwSizeComponentID) &&
        ERROR_SUCCESS == RegQueryValueExA(hkeyMapiClient, SzValueNameLCID, nullptr, &dwType,
                                          reinterpret_cast<LPBYTE>(&rgchMSIApplicationLCID), &dwSizeLCID)) {
        if (GetComponentPath(rgchMSIComponentID, rgchMSIApplicationLCID, rgchComponentPath, _countof(rgchComponentPath),
                             false)) {
            hinstMapi = LoadLibraryA(rgchComponentPath);
        }
    }
    return hinstMapi;
}

/*
 *  LoadMAPIFromSystemDir
 *		Fall back for loading System32\Mapi32.dll if all else fails
 */
HMODULE LoadMAPIFromSystemDir()
{
    WCHAR szSystemDir[MAX_PATH] = {0};

    if (GetSystemDirectoryW(szSystemDir, MAX_PATH)) {
        WCHAR szDLLPath[MAX_PATH] = {0};
        swprintf_s(szDLLPath, _countof(szDLLPath), WszMAPISystemPath, szSystemDir, WszMapi32);
        return LoadLibraryW(szDLLPath);
    }

    return nullptr;
}

/*
 *  LoadMailClientFromDllPath
 *		Attempt to locate the MAPI provider DLL via HKLM\Software\Clients\Mail\(provider)\DllPathEx
 */
HMODULE LoadMailClientFromDllPath(HKEY hkeyMapiClient)
{
    HMODULE hinstMapi = nullptr;
    WCHAR rgchDllPath[MAX_PATH];

    DWORD dwSizeDllPath = _countof(rgchDllPath);

    if (ERROR_SUCCESS == RegQueryWszExpand(hkeyMapiClient, WszValueNameDllPathEx, rgchDllPath, dwSizeDllPath)) {
        hinstMapi = LoadLibraryW(rgchDllPath);
    }

    if (!hinstMapi) {
        dwSizeDllPath = _countof(rgchDllPath);
        if (ERROR_SUCCESS == RegQueryWszExpand(hkeyMapiClient, WszValueNameDllPath, rgchDllPath, dwSizeDllPath)) {
            hinstMapi = LoadLibraryW(rgchDllPath);
        }
    }
    return hinstMapi;
}

/*
 *  LoadRegisteredMapiClient
 *		Read the registry to discover the registered MAPI client and attempt to load its MAPI DLL.
 *
 *		If wzOverrideProvider is specified, this function will load that MAPI Provider instead of the
 *		currently registered provider
 */
HMODULE LoadRegisteredMapiClient(LPCWSTR pwzProviderOverride)
{
    HMODULE hinstMapi = nullptr;
    DWORD dwType;
    HKEY hkey = nullptr, hkeyMapiClient = nullptr;
    WCHAR rgchMailClient[MAX_PATH];
    LPCWSTR pwzProvider = pwzProviderOverride;

    // Open HKLM\Software\Clients\Mail
    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, WszKeyNameMailClient, 0, KEY_READ, &hkey)) {
        // If a specific provider wasn't specified, load the name of the default MAPI provider
        if (!pwzProvider) {
            // Get Outlook application path registry value
            DWORD dwSize = sizeof(rgchMailClient);
            if
                SUCCEEDED(RegQueryValueExW(hkey, nullptr, 0, &dwType, (LPBYTE)&rgchMailClient, &dwSize))

            if (dwType != REG_SZ)
                goto Error;

            pwzProvider = rgchMailClient;
        }

        if (pwzProvider) {
            if
                SUCCEEDED(RegOpenKeyExW(hkey, pwzProvider, 0, KEY_READ, &hkeyMapiClient))
                {
                    hinstMapi = LoadMailClientFromMSIData(hkeyMapiClient);

                    if (!hinstMapi)
                        hinstMapi = LoadMailClientFromDllPath(hkeyMapiClient);
                }
        }
    }

Error:
    return hinstMapi;
}

HMODULE GetDefaultMapiHandle()
{
    HMODULE hinstMapi = nullptr;

    // Try to respect the machine's default MAPI client settings.  If the active MAPI provider
    //  is Outlook, don't load and instead run the logic below
    if (s_fForceOutlookMAPI)
        hinstMapi = LoadRegisteredMapiClient(WszOutlookMapiClientName);
    else
        hinstMapi = LoadRegisteredMapiClient(nullptr);

    // If MAPI still isn't loaded, load the stub from the system directory
    if (!hinstMapi && !s_fForceOutlookMAPI) {
        hinstMapi = LoadMAPIFromSystemDir();
    }

    return hinstMapi;
}

/*------------------------------------------------------------------------------
    Attach to wzMapiDll(olmapi32.dll/msmapi32.dll) if it is already loaded in the
    current process.
------------------------------------------------------------------------------*/
HMODULE AttachToMAPIDll(const WCHAR *wzMapiDll)
{
    HMODULE hinstPrivateMAPI = nullptr;
    GetModuleHandleExW(0UL, wzMapiDll, &hinstPrivateMAPI);
    return hinstPrivateMAPI;
}

void UnloadPrivateMAPI()
{
    const HMODULE hinstPrivateMAPI = GetMAPIHandle();
    if (nullptr != hinstPrivateMAPI) {
        SetMAPIHandle(nullptr);
    }
}

void ForceOutlookMAPI(bool fForce) { s_fForceOutlookMAPI = fForce; }

HMODULE GetPrivateMAPI()
{
    HMODULE hinstPrivateMAPI = GetMAPIHandle();

    if (nullptr == hinstPrivateMAPI) {
        // First, try to attach to olmapi32.dll if it's loaded in the process
        hinstPrivateMAPI = AttachToMAPIDll(WszOlMAPI32DLL);

        // If that fails try msmapi32.dll, for Outlook 11 and below
        //  Only try this in the static lib, otherwise msmapi32.dll will attach to itself.
        if (nullptr == hinstPrivateMAPI) {
            hinstPrivateMAPI = AttachToMAPIDll(WszMSMAPI32DLL);
        }

        // If MAPI isn't loaded in the process yet, then find the path to the DLL and
        // load it manually.
        if (nullptr == hinstPrivateMAPI) {
            hinstPrivateMAPI = GetDefaultMapiHandle();
        }

        if (nullptr != hinstPrivateMAPI) {
            SetMAPIHandle(hinstPrivateMAPI);
        }

        // Reason - if for any reason there is an instance already loaded, SetMAPIHandle()
        // will free the new one and reuse the old one
        // So we fetch the instance from the global again
        return GetMAPIHandle();
    }

    return hinstPrivateMAPI;
}
