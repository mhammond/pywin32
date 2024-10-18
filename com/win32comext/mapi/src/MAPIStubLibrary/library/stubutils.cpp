#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <MAPIX.h>
#include <Msi.h>

// clang-format off
#pragma warning(disable : 26426) // Warning C26426 Global initializer calls a non-constexpr (i.22)
#pragma warning(disable : 26446) // Warning C26446 Prefer to use gsl::at() instead of unchecked subscript operator (bounds.4).
#pragma warning(disable : 26481) // Warning C26481 Don't use pointer arithmetic. Use span instead (bounds.1).
#pragma warning(disable : 26485) // Warning C26485 Expression '': No array to pointer decay (bounds.3).
#pragma warning(disable : 26487) // Warning C26487 Don't return a pointer '' that may be invalid (lifetime.4).
// clang-format on

namespace mapistub
{
	/*
	 * MAPI Stub Utilities
	 *
	 * Public Functions:
	 *
	 * GetPrivateMAPI()
	 * Obtain a handle to the MAPI DLL. This function will load the MAPI DLL
	 * if it hasn't already been loaded
	 *
	 * UnloadPrivateMAPI()
	 * Forces the MAPI DLL to be unloaded. This can cause problems if the code
	 * still has outstanding allocated MAPI memory, or unmatched calls to
	 * MAPIInitialize/MAPIUninitialize
	 *
	 * ForceOutlookMAPI()
	 * Instructs the stub code to always try loading the Outlook version of MAPI
	 * on the system, instead of respecting the system MAPI registration
	 * (HKLM\Software\Clients\Mail). This call must be made prior to any MAPI
	 * function calls.

	 * ForceSystemMAPI()
	 * Instructs the stub code to always try loading the MAPI32 from the
	 * system directory. This call must be made prior to any MAPI
	 * function calls.
	 */

	std::function<void(LPCWSTR szMsg, va_list argList)> logLoadMapiCallback;
	std::function<void(LPCWSTR szMsg, va_list argList)> logLoadLibraryCallback;

	void __cdecl logLoadMapi(LPCWSTR szMsg, ...)
	{
		if (logLoadMapiCallback)
		{
			va_list argList = nullptr;
			va_start(argList, szMsg);
			logLoadMapiCallback(szMsg, argList);
			va_end(argList);
		}
	}

	void __cdecl logLoadLibrary(LPCWSTR szMsg, ...)
	{
		if (logLoadLibraryCallback)
		{
			va_list argList = nullptr;
			va_start(argList, szMsg);
			logLoadLibraryCallback(szMsg, argList);
			va_end(argList);
		}
	}

	template <class T> void LogError(LPCWSTR function, T error)
	{
		if (error) logLoadMapi(L"%ws failed with 0x%08X\n", function, error);
	}

	std::wstring GetSystemDirectory()
	{
		logLoadMapi(L"Enter GetSystemDirectory\n");
		auto path = std::wstring();
		auto copied = DWORD();
		do
		{
			path.resize(path.size() + MAX_PATH);
			copied = ::GetSystemDirectoryW(const_cast<LPWSTR>(path.data()), static_cast<UINT>(path.size()));
			if (!copied)
			{
				const auto dwErr = GetLastError();
				logLoadMapi(L"GetSystemDirectory: GetSystemDirectoryW failed with 0x%08X\n", dwErr);
			}
		} while (copied >= path.size());

		path.resize(copied);

		logLoadMapi(L"Exit GetSystemDirectory: found %ws\n", path.c_str());
		return path;
	}

	std::vector<std::wstring> g_pszOutlookQualifiedComponents = {
		L"{5812C571-53F0-4467-BEFA-0A4F47A9437C}", // O16_CATEGORY_GUID_CORE_OFFICE (retail) // STRING_OK
		L"{E83B4360-C208-4325-9504-0D23003A74A5}", // O15_CATEGORY_GUID_CORE_OFFICE (retail) // STRING_OK
		L"{1E77DE88-BCAB-4C37-B9E5-073AF52DFD7A}", // O14_CATEGORY_GUID_CORE_OFFICE (retail) // STRING_OK
		L"{24AAE126-0911-478F-A019-07B875EB9996}", // O12_CATEGORY_GUID_CORE_OFFICE (retail) // STRING_OK
		L"{BC174BAD-2F53-4855-A1D5-0D575C19B1EA}", // O11_CATEGORY_GUID_CORE_OFFICE (retail) // STRING_OK
		L"{BC174BAD-2F53-4855-A1D5-1D575C19B1EA}", // O11_CATEGORY_GUID_CORE_OFFICE (debug) // STRING_OK
	};

	std::wstring GetInstalledOutlookMAPI(int iOutlook);
	std::wstring GetInstalledOutlookMAPI(const std::wstring component);

	_Check_return_ HMODULE LoadFromSystemDir(_In_ const std::wstring& szDLLName)
	{
		if (szDLLName.empty()) return nullptr;

		static auto szSystemDir = std::wstring();
		static auto bSystemDirLoaded = false;

		logLoadLibrary(L"LoadFromSystemDir - loading \"%ws\"\n", szDLLName.c_str());

		if (!bSystemDirLoaded)
		{
			szSystemDir = mapistub::GetSystemDirectory();
			bSystemDirLoaded = true;
		}

		const auto szDLLPath = szSystemDir + L"\\" + szDLLName;
		logLoadLibrary(L"LoadFromSystemDir - loading from \"%ws\"\n", szDLLPath.c_str());
		return LoadLibraryW(szDLLPath.c_str());
	}

	// Loads szModule at the handle given by hModule, then looks for szEntryPoint.
	// Will not load a module or entry point twice
	void LoadProc(_In_ const std::wstring& szModule, HMODULE& hModule, LPCSTR szEntryPoint, FARPROC& lpfn)
	{
		if (!szEntryPoint) return;
		if (!hModule && !szModule.empty())
		{
			hModule = LoadFromSystemDir(szModule);
		}

		if (!hModule) return;

		lpfn = GetProcAddress(hModule, szEntryPoint);
		if (!lpfn)
		{
			logLoadLibrary(L"LoadProc: failed to load \"%ws\" from \"%ws\"\n", szEntryPoint, szModule.c_str());
		}
	}

	_Check_return_ HMODULE LoadFromOLMAPIDir(_In_ const std::wstring& szDLLName)
	{
		HMODULE hModRet = nullptr;

		logLoadLibrary(L"LoadFromOLMAPIDir - loading \"%ws\"\n", szDLLName.c_str());

		for (const auto component : mapistub::g_pszOutlookQualifiedComponents)
		{
			auto szOutlookMAPIPath = mapistub::GetInstalledOutlookMAPI(component);
			if (!szOutlookMAPIPath.empty())
			{
				auto szDrive = std::wstring(_MAX_DRIVE, '\0');
				auto szMAPIPath = std::wstring(MAX_PATH, '\0');
				const auto errNo = _wsplitpath_s(
					szOutlookMAPIPath.c_str(),
					const_cast<LPWSTR>(szDrive.c_str()),
					szDrive.length(),
					const_cast<LPWSTR>(szMAPIPath.c_str()),
					szMAPIPath.length(),
					nullptr,
					NULL,
					nullptr,
					NULL);
				LogError(L"LoadFromOLMAPIDir: _wsplitpath_s", errNo);

				if (errNo == ERROR_SUCCESS)
				{
					auto szFullPath = szDrive + szMAPIPath + szDLLName;

					logLoadLibrary(L"LoadFromOLMAPIDir - loading from \"%ws\"\n", szFullPath.c_str());
					hModRet = LoadLibraryW(szFullPath.c_str());
				}
			}

			if (hModRet) break;
		}

		return hModRet;
	}

	// From kernel32.dll
	HMODULE hModKernel32 = nullptr;
	typedef bool(WINAPI GETMODULEHANDLEEXW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule);
	GETMODULEHANDLEEXW* pfnGetModuleHandleExW = nullptr;
	BOOL WINAPI MyGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule)
	{
		if (!pfnGetModuleHandleExW)
		{
			FARPROC lpfnFP = {};
			LoadProc(L"kernel32.dll", hModKernel32, "GetModuleHandleExW", lpfnFP); // STRING_OK;
			pfnGetModuleHandleExW = reinterpret_cast<GETMODULEHANDLEEXW*>(lpfnFP);
		}

		if (pfnGetModuleHandleExW) return pfnGetModuleHandleExW(dwFlags, lpModuleName, phModule);
		*phModule = GetModuleHandleW(lpModuleName);
		return *phModule != nullptr;
	}

	// From MSI.dll
	HMODULE hModMSI = nullptr;
	typedef HRESULT(STDMETHODCALLTYPE MSIPROVIDEQUALIFIEDCOMPONENT)(
		LPCWSTR szCategory,
		LPCWSTR szQualifier,
		DWORD dwInstallMode,
		LPWSTR lpPathBuf,
		LPDWORD pcchPathBuf);
	MSIPROVIDEQUALIFIEDCOMPONENT* pfnMsiProvideQualifiedComponent = nullptr;
	HRESULT MyMsiProvideQualifiedComponent(
		LPCWSTR szCategory,
		LPCWSTR szQualifier,
		DWORD dwInstallMode,
		LPWSTR lpPathBuf,
		LPDWORD pcchPathBuf)
	{
		if (!pfnMsiProvideQualifiedComponent)
		{
			FARPROC lpfnFP = {};
			LoadProc(L"msi.dll", hModMSI, "MsiProvideQualifiedComponentW", lpfnFP); // STRING_OK;
			pfnMsiProvideQualifiedComponent = reinterpret_cast<MSIPROVIDEQUALIFIEDCOMPONENT*>(lpfnFP);
		}

		if (pfnMsiProvideQualifiedComponent)
			return pfnMsiProvideQualifiedComponent(szCategory, szQualifier, dwInstallMode, lpPathBuf, pcchPathBuf);
		return MAPI_E_CALL_FAILED;
	}

	const WCHAR WszKeyNameMailClient[] = L"Software\\Clients\\Mail";
	const WCHAR WszValueNameDllPathEx[] = L"DllPathEx";
	const WCHAR WszValueNameDllPath[] = L"DllPath";

	const WCHAR WszValueNameMSI[] = L"MSIComponentID";
	const WCHAR WszValueNameLCID[] = L"MSIApplicationLCID";

	const WCHAR WszOutlookMapiClientName[] = L"Microsoft Outlook";

	const WCHAR WszMAPISystemPath[] = L"%s\\%s";
	const WCHAR WszMAPISystemDrivePath[] = L"%s%s%s";
	const WCHAR szMAPISystemDrivePath[] = L"%hs%hs%ws";

	static const WCHAR WszOlMAPI32DLL[] = L"olmapi32.dll";
	static const WCHAR WszMSMAPI32DLL[] = L"msmapi32.dll";
	static const WCHAR WszMapi32[] = L"mapi32.dll";
	static const WCHAR WszMapiStub[] = L"mapistub.dll";

	static const CHAR SzFGetComponentPath[] = "FGetComponentPath";

	// Sequence number which is incremented every time we set our MAPI handle which will
	// cause a re-fetch of all stored function pointers
	volatile ULONG g_ulDllSequenceNum = 1;

	// Whether or not we should ignore the system MAPI registration and always try to find
	// Outlook and its MAPI DLLs
	static bool s_fForceOutlookMAPI = false;

	// Whether or not we should ignore the registry and load MAPI from the system directory
	static bool s_fForceSystemMAPI = false;

	static volatile HMODULE g_hinstMAPI = nullptr;
	HMODULE g_hModPstPrx32 = nullptr;

	/*
	 * RegQueryWszExpand
	 * Wrapper for RegQueryValueExW which automatically expands REG_EXPAND_SZ values
	 */
	std::wstring RegQueryWszExpand(HKEY hKey, const std::wstring& lpValueName)
	{
		logLoadMapi(L"Enter RegQueryWszExpand: hKey = %p, lpValueName = %ws\n", hKey, lpValueName.c_str());
		DWORD dwType = 0;

		std::wstring ret;
		auto rgchValue = std::wstring(MAX_PATH, '\0');
		auto dwSize = static_cast<DWORD>(rgchValue.length());

		const auto dwErr = RegQueryValueExW(
			hKey,
			lpValueName.c_str(),
			nullptr,
			&dwType,
			reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(rgchValue.data())),
			&dwSize);

		if (dwErr == ERROR_SUCCESS)
		{
			logLoadMapi(L"RegQueryWszExpand: rgchValue = %ws\n", rgchValue.c_str());
			if (dwType == REG_EXPAND_SZ)
			{
				const auto szPath = std::wstring(MAX_PATH, '\0');
				// Expand the strings
				const auto cch = ExpandEnvironmentStringsW(
					rgchValue.c_str(), const_cast<wchar_t*>(szPath.c_str()), static_cast<DWORD>(szPath.length()));
				if (0 != cch && cch < MAX_PATH)
				{
					logLoadMapi(L"RegQueryWszExpand: rgchValue(expanded) = %ws\n", szPath.c_str());
					ret = szPath;
				}
			}
			else if (dwType == REG_SZ)
			{
				ret = rgchValue;
			}
		}

		logLoadMapi(L"Exit RegQueryWszExpand: dwErr = 0x%08X\n", dwErr);
		return ret;
	}

	std::string wstringTostring(const std::wstring& src)
	{
		std::string dst;
		dst.reserve(src.length());
		std::transform(src.begin(), src.end(), std::back_inserter(dst), [](auto c) { return static_cast<char>(c); });
		return dst;
	}

	/*
	 * GetComponentPath
	 * Wrapper around mapi32.dll->FGetComponentPath which maps an MSI component ID to
	 * a DLL location from the default MAPI client registration values
	 */
	std::wstring GetComponentPath(const std::wstring& szComponent, const std::wstring& szQualifier, bool fInstall)
	{
		logLoadMapi(
			L"Enter GetComponentPath: szComponent = %ws, szQualifier = %ws, fInstall = 0x%08X\n",
			szComponent.c_str(),
			szQualifier.c_str(),
			fInstall);
		auto fReturn = false;
		std::wstring path;

		typedef bool(STDAPICALLTYPE * FGetComponentPathType)(LPCSTR, LPSTR, LPSTR, DWORD, bool);

		auto hMapiStub = LoadLibraryW(WszMapi32);
		if (!hMapiStub) hMapiStub = LoadLibraryW(WszMapiStub);

		if (hMapiStub)
		{
			const auto pFGetCompPath =
				reinterpret_cast<FGetComponentPathType>(GetProcAddress(hMapiStub, SzFGetComponentPath));

			if (pFGetCompPath)
			{
				CHAR lpszPath[MAX_PATH] = {0};
				const constexpr ULONG cchPath = _countof(lpszPath);

				auto szComponentA = wstringTostring(szComponent);
				auto szQualifierA = wstringTostring(szQualifier);
				fReturn = pFGetCompPath(
					szComponentA.c_str(), const_cast<LPSTR>(szQualifierA.c_str()), lpszPath, cchPath, fInstall);
				auto pathA = std::string(lpszPath);
				if (fReturn) path = {pathA.begin(), pathA.end()};
				logLoadMapi(L"GetComponentPath: path = %ws\n", path.c_str());
			}

			FreeLibrary(hMapiStub);
		}

		logLoadMapi(L"Exit GetComponentPath: fReturn = 0x%08X\n", fReturn);
		return path;
	}

	/*
	 * GetMailClientFromMSIData
	 * Attempt to locate the MAPI provider DLL via HKLM\Software\Clients\Mail\(provider)\MSIComponentID
	 */
	std::wstring GetMailClientFromMSIData(HKEY hkeyMapiClient)
	{
		logLoadMapi(L"Enter GetMailClientFromMSIData\n");
		if (!hkeyMapiClient) return L"";
		WCHAR rgchMSIComponentID[MAX_PATH] = {0};
		WCHAR rgchMSIApplicationLCID[MAX_PATH] = {0};
		DWORD dwType = 0;
		std::wstring szPath;

		DWORD dwSizeComponentID = _countof(rgchMSIComponentID);
		DWORD dwSizeLCID = _countof(rgchMSIApplicationLCID);

		if (ERROR_SUCCESS == RegQueryValueExW(
								 hkeyMapiClient,
								 WszValueNameMSI,
								 nullptr,
								 &dwType,
								 reinterpret_cast<LPBYTE>(&rgchMSIComponentID),
								 &dwSizeComponentID) &&
			ERROR_SUCCESS == RegQueryValueExW(
								 hkeyMapiClient,
								 WszValueNameLCID,
								 nullptr,
								 &dwType,
								 reinterpret_cast<LPBYTE>(&rgchMSIApplicationLCID),
								 &dwSizeLCID))
		{
			const auto componentID = std::wstring(rgchMSIComponentID, dwSizeComponentID);
			const auto applicationID = std::wstring(rgchMSIApplicationLCID, dwSizeLCID);
			szPath = GetComponentPath(componentID, applicationID, false);
		}

		logLoadMapi(L"Exit GetMailClientFromMSIData: szPath = %ws\n", szPath.c_str());
		return szPath;
	}

	/*
	* GetMAPISystemDir
	* Fall back for loading System32\Mapi32.dll if all else fails
	*/
	std::wstring GetMAPISystemDir() { return GetSystemDirectory() + L"\\" + std::wstring(WszMapi32); }

	HKEY GetHKeyMapiClient(const std::wstring& pwzProviderOverride)
	{
		logLoadMapi(L"Enter GetHKeyMapiClient (%ws)\n", pwzProviderOverride.c_str());
		HKEY hMailKey = nullptr;

		// Open HKLM\Software\Clients\Mail
		auto status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, WszKeyNameMailClient, 0, KEY_READ, &hMailKey);
		LogError(L"GetHKeyMapiClient: RegOpenKeyExW(HKLM)", status);
		if (status != ERROR_SUCCESS)
		{
			hMailKey = nullptr;
		}

		// If a specific provider wasn't specified, load the name of the default MAPI provider
		std::wstring defaultClient;
		auto pwzProvider = pwzProviderOverride;
		if (hMailKey && pwzProvider.empty())
		{
			const auto rgchMailClient = std::wstring(MAX_PATH, '\0');
			// Get Outlook application path registry value
			DWORD dwSize = MAX_PATH;
			DWORD dwType = 0;
			status = RegQueryValueExW(
				hMailKey,
				nullptr,
				nullptr,
				&dwType,
				reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(rgchMailClient.c_str())),
				&dwSize);
			LogError(L"GetHKeyMapiClient: RegQueryValueExW(hMailKey)", status);
			if (status == ERROR_SUCCESS)
			{
				defaultClient = rgchMailClient;
				logLoadMapi(L"GetHKeyMapiClient: HKLM\\%ws = %ws\n", WszKeyNameMailClient, defaultClient.c_str());
			}
		}

		if (pwzProvider.empty()) pwzProvider = defaultClient;

		HKEY hkeyMapiClient = nullptr;
		if (hMailKey && !pwzProvider.empty())
		{
			logLoadMapi(L"GetHKeyMapiClient: pwzProvider = %ws\n", pwzProvider.c_str());
			status = RegOpenKeyExW(hMailKey, pwzProvider.c_str(), 0, KEY_READ, &hkeyMapiClient);
			LogError(L"GetHKeyMapiClient: RegOpenKeyExW", status);
			if (status != ERROR_SUCCESS)
			{
				hkeyMapiClient = nullptr;
			}
		}

		logLoadMapi(L"Exit GetHKeyMapiClient.hkeyMapiClient found (%ws)\n", hkeyMapiClient ? L"true" : L"false");

		if (hMailKey) RegCloseKey(hMailKey);
		return hkeyMapiClient;
	}

	// Looks up Outlook's path given its qualified component guid
	std::wstring GetOutlookPath(_In_ const std::wstring& szCategory, _Out_opt_ bool* lpb64)
	{
		logLoadMapi(L"Enter GetOutlookPath: szCategory = %ws\n", szCategory.c_str());
		DWORD dwValueBuf = 0;
		std::wstring path;

		if (lpb64) *lpb64 = false;

		auto hRes = MyMsiProvideQualifiedComponent(
			szCategory.c_str(),
			L"outlook.x64.exe", // STRING_OK
			static_cast<DWORD>(INSTALLMODE_DEFAULT),
			nullptr,
			&dwValueBuf);
		LogError(L"GetOutlookPath: MsiProvideQualifiedComponent(x64)", hRes);
		if (hRes == S_OK)
		{
			if (lpb64) *lpb64 = true;
		}
		else
		{
			hRes = MyMsiProvideQualifiedComponent(
				szCategory.c_str(),
				L"outlook.exe", // STRING_OK
				static_cast<DWORD>(INSTALLMODE_DEFAULT),
				nullptr,
				&dwValueBuf);
			LogError(L"GetOutlookPath: MsiProvideQualifiedComponent(x86)", hRes);
		}

		if (hRes == S_OK)
		{
			dwValueBuf += 1;
			const auto lpszTempPath = std::wstring(dwValueBuf, '\0');

			hRes = MyMsiProvideQualifiedComponent(
				szCategory.c_str(),
				L"outlook.x64.exe", // STRING_OK
				static_cast<DWORD>(INSTALLMODE_DEFAULT),
				const_cast<wchar_t*>(lpszTempPath.c_str()),
				&dwValueBuf);
			LogError(L"GetOutlookPath: MsiProvideQualifiedComponent(x64)", hRes);
			if (hRes != S_OK)
			{
				hRes = MyMsiProvideQualifiedComponent(
					szCategory.c_str(),
					L"outlook.exe", // STRING_OK
					static_cast<DWORD>(INSTALLMODE_DEFAULT),
					const_cast<wchar_t*>(lpszTempPath.c_str()),
					&dwValueBuf);
				LogError(L"GetOutlookPath: MsiProvideQualifiedComponent(x86)", hRes);
			}

			if (hRes == S_OK)
			{
				path = lpszTempPath;
				logLoadMapi(L"Exit GetOutlookPath: Path = %ws\n", path.c_str());
			}
		}

		if (path.empty())
		{
			logLoadMapi(L"Exit GetOutlookPath: nothing found\n");
		}

		return path;
	}

	std::wstring GetInstalledOutlookMAPI(int iOutlook)
	{
		logLoadMapi(L"Enter GetInstalledOutlookMAPI(%d)\n", iOutlook);

		auto szPath = GetInstalledOutlookMAPI(g_pszOutlookQualifiedComponents[iOutlook]);

		if (!szPath.empty())
		{
			logLoadMapi(L"GetInstalledOutlookMAPI: found %ws\n", szPath.c_str());
			return szPath;
		}

		logLoadMapi(L"Exit GetInstalledOutlookMAPI: found nothing\n");
		return L"";
	}

	std::wstring GetInstalledOutlookMAPI(const std::wstring component)
	{
		logLoadMapi(L"Enter GetInstalledOutlookMAPI(%s)\n", component.c_str());

		auto lpszTempPath = GetOutlookPath(component, nullptr);

		if (!lpszTempPath.empty())
		{
			WCHAR szDrive[_MAX_DRIVE] = {0};
			WCHAR szOutlookPath[MAX_PATH] = {0};
			const auto errNo = _wsplitpath_s(
				lpszTempPath.c_str(), szDrive, _MAX_DRIVE, szOutlookPath, MAX_PATH, nullptr, NULL, nullptr, NULL);
			LogError(L"GetOutlookPath: _wsplitpath_s", errNo);

			if (errNo == ERROR_SUCCESS)
			{
				const auto szPath = std::wstring(szDrive) + std::wstring(szOutlookPath) + WszOlMAPI32DLL;

				logLoadMapi(L"GetInstalledOutlookMAPI: found %ws\n", szPath.c_str());
				return szPath;
			}
		}

		logLoadMapi(L"Exit GetInstalledOutlookMAPI: found nothing\n");
		return L"";
	}

	std::vector<std::wstring> GetInstalledOutlookMAPI()
	{
		logLoadMapi(L"Enter GetInstalledOutlookMAPI\n");
		auto paths = std::vector<std::wstring>();

		for (const auto compontent : g_pszOutlookQualifiedComponents)
		{
			auto szPath = GetInstalledOutlookMAPI(compontent);
			if (!szPath.empty()) paths.push_back(szPath);
		}

		logLoadMapi(L"Exit GetInstalledOutlookMAPI: found %d paths\n", paths.size());
		return paths;
	}

	std::vector<std::wstring> GetMAPIPaths()
	{
		auto paths = std::vector<std::wstring>();
		std::wstring szPath;
		if (s_fForceSystemMAPI)
		{
			szPath = GetMAPISystemDir();
			if (!szPath.empty()) paths.push_back(szPath);
			return paths;
		}

		auto hkeyMapiClient = HKEY{};
		if (s_fForceOutlookMAPI)
			hkeyMapiClient = GetHKeyMapiClient(WszOutlookMapiClientName);
		else
			hkeyMapiClient = GetHKeyMapiClient(L"");

		szPath = RegQueryWszExpand(hkeyMapiClient, WszValueNameDllPathEx);
		if (!szPath.empty()) paths.push_back(szPath);

		auto outlookPaths = GetInstalledOutlookMAPI();
		paths.insert(end(paths), std::begin(outlookPaths), std::end(outlookPaths));

		szPath = RegQueryWszExpand(hkeyMapiClient, WszValueNameDllPath);
		if (!szPath.empty()) paths.push_back(szPath);

		szPath = GetMailClientFromMSIData(hkeyMapiClient);
		if (!szPath.empty()) paths.push_back(szPath);

		if (!s_fForceOutlookMAPI)
		{
			szPath = GetMAPISystemDir();
			if (!szPath.empty()) paths.push_back(szPath);
		}

		if (hkeyMapiClient) RegCloseKey(hkeyMapiClient);
		return paths;
	}

	HMODULE GetDefaultMapiHandle()
	{
		logLoadMapi(L"Enter GetDefaultMapiHandle\n");
		HMODULE hinstMapi = nullptr;

		auto paths = GetMAPIPaths();
		for (const auto& szPath : paths)
		{
			logLoadMapi(L"Trying %ws\n", szPath.c_str());
			hinstMapi = LoadLibraryW(szPath.c_str());
			if (hinstMapi) break;
		}

		logLoadMapi(L"Exit GetDefaultMapiHandle: hinstMapi = %p\n", hinstMapi);
		return hinstMapi;
	}

	/*------------------------------------------------------------------------------
	 Attach to wzMapiDll(olmapi32.dll/msmapi32.dll) if it is already loaded in the
	 current process.
	 ------------------------------------------------------------------------------*/
	HMODULE AttachToMAPIDll(const WCHAR* wzMapiDll)
	{
		logLoadMapi(L"Enter AttachToMAPIDll: wzMapiDll = %ws\n", wzMapiDll);
		HMODULE hinstPrivateMAPI = nullptr;
		MyGetModuleHandleExW(0UL, wzMapiDll, &hinstPrivateMAPI);
		logLoadMapi(L"Exit AttachToMAPIDll: hinstPrivateMAPI = %p\n", hinstPrivateMAPI);
		return hinstPrivateMAPI;
	}

	HMODULE GetMAPIHandle() noexcept { return g_hinstMAPI; }

	void SetMAPIHandle(HMODULE hinstMAPI)
	{
		logLoadMapi(L"Enter SetMAPIHandle: hinstMAPI = %p\n", hinstMAPI);
		HMODULE hinstToFree = nullptr;

		if (hinstMAPI == nullptr)
		{
			// If we've preloaded pstprx32.dll, unload it before MAPI is unloaded to prevent dependency problems
			if (g_hModPstPrx32)
			{
				FreeLibrary(g_hModPstPrx32);
				g_hModPstPrx32 = nullptr;
			}

			hinstToFree = static_cast<HMODULE>(
				InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&g_hinstMAPI), nullptr));
		}
		else
		{
			// Preload pstprx32 to prevent crash when using autodiscover to build a new profile
			if (!g_hModPstPrx32)
			{
				g_hModPstPrx32 = LoadFromOLMAPIDir(L"pstprx32.dll"); // STRING_OK
			}

			// Code Analysis gives us a C28112 error when we use InterlockedCompareExchangePointer, so we instead exchange, check and exchange back
			//hinstPrev = (HMODULE)InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&g_hinstMAPI), hinstMAPI, hinstNULL);
			const auto hinstPrev =
				InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&g_hinstMAPI), hinstMAPI);
			if (nullptr != hinstPrev)
			{
				static_cast<void>(InterlockedExchangePointer(
					reinterpret_cast<PVOID volatile*>(&g_hinstMAPI), static_cast<PVOID>(hinstPrev)));
				hinstToFree = hinstMAPI;
			}

			// If we've updated our MAPI handle, any previous addressed fetched via GetProcAddress are invalid, so we
			// have to increment a sequence number to signal that they need to be re-fetched
			InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_ulDllSequenceNum));
		}

		if (nullptr != hinstToFree)
		{
			FreeLibrary(hinstToFree);
		}

		logLoadMapi(L"Exit SetMAPIHandle\n");
	}

	HMODULE GetPrivateMAPI()
	{
		logLoadMapi(L"Enter GetPrivateMAPI\n");
		auto hinstPrivateMAPI = GetMAPIHandle();

		if (nullptr == hinstPrivateMAPI)
		{
			// First, try to attach to olmapi32.dll if it's loaded in the process
			hinstPrivateMAPI = AttachToMAPIDll(WszOlMAPI32DLL);

			// If that fails try msmapi32.dll, for Outlook 11 and below
			// Only try this in the static lib, otherwise msmapi32.dll will attach to itself.
			if (nullptr == hinstPrivateMAPI)
			{
				hinstPrivateMAPI = AttachToMAPIDll(WszMSMAPI32DLL);
			}

			// If MAPI isn't loaded in the process yet, then find the path to the DLL and
			// load it manually.
			if (nullptr == hinstPrivateMAPI)
			{
				hinstPrivateMAPI = GetDefaultMapiHandle();
			}

			if (nullptr != hinstPrivateMAPI)
			{
				SetMAPIHandle(hinstPrivateMAPI);
			}

			// Reason - if for any reason there is an instance already loaded, SetMAPIHandle()
			// will free the new one and reuse the old one
			// So we fetch the instance from the global again
			logLoadMapi(L"Exit GetPrivateMAPI: Returning GetMAPIHandle()\n");
			return GetMAPIHandle();
		}

		logLoadMapi(L"Exit GetPrivateMAPI, hinstPrivateMAPI = %p\n", hinstPrivateMAPI);
		return hinstPrivateMAPI;
	}

	void UnloadPrivateMAPI()
	{
		logLoadMapi(L"Enter UnloadPrivateMAPI\n");
		if (GetMAPIHandle() != nullptr)
		{
			SetMAPIHandle(nullptr);
		}

		logLoadMapi(L"Exit UnloadPrivateMAPI\n");
	}

	void ForceOutlookMAPI(bool fForce)
	{
		logLoadMapi(L"ForceOutlookMAPI: fForce = 0x%08X\n", fForce);
		s_fForceOutlookMAPI = fForce;
	}

	void ForceSystemMAPI(bool fForce)
	{
		logLoadMapi(L"ForceSystemMAPI: fForce = 0x%08X\n", fForce);
		s_fForceSystemMAPI = fForce;
	}
} // namespace mapistub
