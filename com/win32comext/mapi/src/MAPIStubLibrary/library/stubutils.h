#pragma once
#include <functional>
#include <vector>
#include <string>

// stubutils.h - Optional header to enable clients to reuse stubutils code
// Not reuqired to build the mapistub library

namespace mapistub
{
	// Assign callbacks to these to enable logging
	extern std::function<void(LPCWSTR szMsg, va_list argList)> logLoadMapiCallback;
	extern std::function<void(LPCWSTR szMsg, va_list argList)> logLoadLibraryCallback;

	std::wstring GetSystemDirectory();

	_Check_return_ HMODULE LoadFromSystemDir(_In_ const std::wstring& szDLLName);

	extern HMODULE hModKernel32;
	extern HMODULE hModMSI;
	// Loads szModule at the handle given by hModule, then looks for szEntryPoint.
	// Will not load a module or entry point twice
	void LoadProc(_In_ const std::wstring& szModule, HMODULE& hModule, LPCSTR szEntryPoint, FARPROC& lpfn);

	extern volatile ULONG g_ulDllSequenceNum;
	// Keep this in sync with g_pszOutlookQualifiedComponents
	enum officeComponent
	{
		oqcOffice16 = 0,
		oqcOffice15 = 1,
		oqcOffice14 = 2,
		oqcOffice12 = 3,
		oqcOffice11 = 4,
		oqcOffice11Debug = 5
	};

	std::wstring GetComponentPath(const std::wstring& szComponent, const std::wstring& szQualifier, bool fInstall);
	extern std::vector<std::wstring> g_pszOutlookQualifiedComponents;
	std::vector<std::wstring> GetMAPIPaths();
	// Looks up Outlook's path given its qualified component guid
	std::wstring GetOutlookPath(_In_ const std::wstring& szCategory, _Out_opt_ bool* lpb64);
	std::wstring GetInstalledOutlookMAPI(int iOutlook);
	std::wstring GetMAPISystemDir();

	HMODULE GetMAPIHandle() noexcept;
	void SetMAPIHandle(HMODULE hinstMAPI);

	HMODULE GetPrivateMAPI();
	void UnloadPrivateMAPI();
	void ForceOutlookMAPI(bool fForce);
	void ForceSystemMAPI(bool fForce);
} // namespace mapistub
