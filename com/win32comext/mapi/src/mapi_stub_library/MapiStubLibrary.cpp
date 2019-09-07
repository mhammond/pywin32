#include <winsock2.h>
#include <Windows.h>

#include <MAPI.h>
#include <MAPIForm.h>
#include <MAPIUtil.h>
#include <MAPIVal.h>
#include <MAPISPI.h>
#include <IMessage.h>
#include <TNEF.h>

#include <strsafe.h>

// Check that we have the Outlook 2010 MAPI headers or higher
// We do this by checking for the presence of a macro not present in the older headers
#ifndef MAPIFORM_CPU_X64
#pragma message("Compilation requires Outlook 2010 MAPI headers or higher")
#pragma message("Go to the following URL")
#pragma message( \
    "    http://www.microsoft.com/downloads/en/details.aspx?FamilyID=f8d01fc8-f7b5-4228-baa3-817488a66db1&displaylang=en")
#pragma message("and follow the instructions to install the Outlook 2010 MAPI headers")
#pragma message("Then go to Tools\\Options\\Projects and Solutions\\VC++ Directories and ensure the headers include")
#pragma message("directory precedes the Visual Studio include directories.")
#pragma message(" ")
#error Outlook 2010 MAPI headers or higher must be installed
#endif

#if defined(_M_X64) || defined(_M_ARM)
#define ExpandFunction(fn, c) #fn
#elif defined(_M_IX86)
#define ExpandFunction(fn, c) #fn "@" #c
#else
#error "Unsupported Platform"
#endif

#if _MSC_VER < 1600
#define nullptr NULL
#endif

// Forward declarations for types not documented in MAPI headers

struct MAPIOFFLINE_CREATEINFO;
struct IMAPIOfflineMgr;
struct RTF_WCSINFO;
struct RTF_WCSRETINFO;

#define LINKAGE_EXTERN_C extern "C"
#define LINKAGE_NO_EXTERN_C /* */

// Forward declares from MapiStubUtil.cpp
HMODULE GetMAPIHandle();
HMODULE GetPrivateMAPI();
void UnloadPrivateMAPI();
extern volatile ULONG g_ulDllSequenceNum;

#define DEFINE_STUB_FUNCTION_V0(_linkage, _modifiers, _name, _lookup)                   \
                                                                                        \
    _linkage typedef void(_modifiers * _name##TYPE)(void);                              \
                                                                                        \
    _linkage void _modifiers _name(void)                                                \
    {                                                                                   \
        static _name##TYPE _name##VAR = nullptr;                                        \
        static UINT ulDllSequenceNum = 0;                                               \
                                                                                        \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) { \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);      \
            ulDllSequenceNum = g_ulDllSequenceNum;                                      \
        }                                                                               \
                                                                                        \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                  \
            _name##VAR();                                                               \
        }                                                                               \
    }

#define DEFINE_STUB_FUNCTION_ORD_V0(_linkage, _modifiers, _name, _ordinal)                   \
                                                                                             \
    _linkage typedef void(_modifiers * _name##TYPE)(void);                                   \
                                                                                             \
    _linkage void _modifiers _name(void)                                                     \
    {                                                                                        \
        static _name##TYPE _name##VAR = nullptr;                                             \
        static UINT ulDllSequenceNum = 0;                                                    \
                                                                                             \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {      \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal)); \
            ulDllSequenceNum = g_ulDllSequenceNum;                                           \
        }                                                                                    \
                                                                                             \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                       \
            _name##VAR();                                                                    \
        }                                                                                    \
    }

#define DEFINE_STUB_FUNCTION_0(_linkage, _ret_type, _modifiers, _name, _lookup, _default) \
                                                                                          \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(void);                            \
                                                                                          \
    _linkage _ret_type _modifiers _name(void)                                             \
    {                                                                                     \
        static _name##TYPE _name##VAR = nullptr;                                          \
        static UINT ulDllSequenceNum = 0;                                                 \
                                                                                          \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {   \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);        \
            ulDllSequenceNum = g_ulDllSequenceNum;                                        \
        }                                                                                 \
                                                                                          \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                    \
            return _name##VAR();                                                          \
        }                                                                                 \
        else {                                                                            \
            return _default;                                                              \
        }                                                                                 \
    }

#define DEFINE_STUB_FUNCTION_ORD_0(_linkage, _ret_type, _modifiers, _name, _ordinal, _default) \
                                                                                               \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(void);                                 \
                                                                                               \
    _linkage _ret_type _modifiers _name(void)                                                  \
    {                                                                                          \
        static _name##TYPE _name##VAR = nullptr;                                               \
        static UINT ulDllSequenceNum = 0;                                                      \
                                                                                               \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {        \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));   \
            ulDllSequenceNum = g_ulDllSequenceNum;                                             \
        }                                                                                      \
                                                                                               \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                         \
            return _name##VAR();                                                               \
        }                                                                                      \
        else {                                                                                 \
            return _default;                                                                   \
        }                                                                                      \
    }

#define DEFINE_STUB_FUNCTION_V1(_linkage, _modifiers, _name, _lookup, _param1_type)     \
                                                                                        \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type);                      \
                                                                                        \
    _linkage void _modifiers _name(_param1_type a)                                      \
    {                                                                                   \
        static _name##TYPE _name##VAR = nullptr;                                        \
        static UINT ulDllSequenceNum = 0;                                               \
                                                                                        \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) { \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);      \
            ulDllSequenceNum = g_ulDllSequenceNum;                                      \
        }                                                                               \
                                                                                        \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                  \
            _name##VAR(a);                                                              \
        }                                                                               \
    }

#define DEFINE_STUB_FUNCTION_ORD_V1(_linkage, _modifiers, _name, _ordinal, _param1_type)     \
                                                                                             \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type);                           \
                                                                                             \
    _linkage void _modifiers _name(_param1_type a)                                           \
    {                                                                                        \
        static _name##TYPE _name##VAR = nullptr;                                             \
        static UINT ulDllSequenceNum = 0;                                                    \
                                                                                             \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {      \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal)); \
            ulDllSequenceNum = g_ulDllSequenceNum;                                           \
        }                                                                                    \
                                                                                             \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                       \
            _name##VAR(a);                                                                   \
        }                                                                                    \
    }

#define DEFINE_STUB_FUNCTION_1(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _default) \
                                                                                                        \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type);                                  \
                                                                                                        \
    _linkage _ret_type _modifiers _name(_param1_type a)                                                 \
    {                                                                                                   \
        static _name##TYPE _name##VAR = nullptr;                                                        \
        static UINT ulDllSequenceNum = 0;                                                               \
                                                                                                        \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                 \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                      \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                      \
        }                                                                                               \
                                                                                                        \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                  \
            return _name##VAR(a);                                                                       \
        }                                                                                               \
        else {                                                                                          \
            return _default;                                                                            \
        }                                                                                               \
    }

#define DEFINE_STUB_FUNCTION_ORD_1(_linkage, _ret_type, _modifiers, _name, _ordinal, _param1_type, _default) \
                                                                                                             \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type);                                       \
                                                                                                             \
    _linkage _ret_type _modifiers _name(_param1_type a)                                                      \
    {                                                                                                        \
        static _name##TYPE _name##VAR = nullptr;                                                             \
        static UINT ulDllSequenceNum = 0;                                                                    \
                                                                                                             \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                      \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                 \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                           \
        }                                                                                                    \
                                                                                                             \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                       \
            return _name##VAR(a);                                                                            \
        }                                                                                                    \
        else {                                                                                               \
            return _default;                                                                                 \
        }                                                                                                    \
    }

#define DEFINE_STUB_FUNCTION_V2(_linkage, _modifiers, _name, _lookup, _param1_type, _param2_type) \
                                                                                                  \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type);                  \
                                                                                                  \
    _linkage void _modifiers _name(_param1_type a, _param2_type b)                                \
    {                                                                                             \
        static _name##TYPE _name##VAR = nullptr;                                                  \
        static UINT ulDllSequenceNum = 0;                                                         \
                                                                                                  \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {           \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                \
        }                                                                                         \
                                                                                                  \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                            \
            _name##VAR(a, b);                                                                     \
        }                                                                                         \
    }

#define DEFINE_STUB_FUNCTION_ORD_V2(_linkage, _modifiers, _name, _ordinal, _param1_type, _param2_type) \
                                                                                                       \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type);                       \
                                                                                                       \
    _linkage void _modifiers _name(_param1_type a, _param2_type b)                                     \
    {                                                                                                  \
        static _name##TYPE _name##VAR = nullptr;                                                       \
        static UINT ulDllSequenceNum = 0;                                                              \
                                                                                                       \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));           \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                     \
        }                                                                                              \
                                                                                                       \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                 \
            _name##VAR(a, b);                                                                          \
        }                                                                                              \
    }

#define DEFINE_STUB_FUNCTION_2(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type, _default) \
                                                                                                                      \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type);                                  \
                                                                                                                      \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b)                                               \
    {                                                                                                                 \
        static _name##TYPE _name##VAR = nullptr;                                                                      \
        static UINT ulDllSequenceNum = 0;                                                                             \
                                                                                                                      \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                               \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                    \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                    \
        }                                                                                                             \
                                                                                                                      \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                                \
            return _name##VAR(a, b);                                                                                  \
        }                                                                                                             \
        else {                                                                                                        \
            return _default;                                                                                          \
        }                                                                                                             \
    }

#define DEFINE_STUB_FUNCTION_ORD_2(_linkage, _ret_type, _modifiers, _name, _ordinal, _param1_type, _param2_type, \
                                   _default)                                                                     \
                                                                                                                 \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type);                             \
                                                                                                                 \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b)                                          \
    {                                                                                                            \
        static _name##TYPE _name##VAR = nullptr;                                                                 \
        static UINT ulDllSequenceNum = 0;                                                                        \
                                                                                                                 \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                          \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                     \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                               \
        }                                                                                                        \
                                                                                                                 \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                           \
            return _name##VAR(a, b);                                                                             \
        }                                                                                                        \
        else {                                                                                                   \
            return _default;                                                                                     \
        }                                                                                                        \
    }

#define DEFINE_STUB_FUNCTION_V3(_linkage, _modifiers, _name, _lookup, _param1_type, _param2_type, _param3_type) \
                                                                                                                \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type);                  \
                                                                                                                \
    _linkage void _modifiers _name(_param1_type a, _param2_type b, _param3_type c)                              \
    {                                                                                                           \
        static _name##TYPE _name##VAR = nullptr;                                                                \
        static UINT ulDllSequenceNum = 0;                                                                       \
                                                                                                                \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                         \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                              \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                              \
        }                                                                                                       \
                                                                                                                \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                          \
            _name##VAR(a, b, c);                                                                                \
        }                                                                                                       \
    }

#define DEFINE_STUB_FUNCTION_ORD_V3(_linkage, _modifiers, _name, _ordinal, _param1_type, _param2_type, _param3_type) \
                                                                                                                     \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type);                       \
                                                                                                                     \
    _linkage void _modifiers _name(_param1_type a, _param2_type b, _param3_type c)                                   \
    {                                                                                                                \
        static _name##TYPE _name##VAR = nullptr;                                                                     \
        static UINT ulDllSequenceNum = 0;                                                                            \
                                                                                                                     \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                              \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                         \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                   \
        }                                                                                                            \
                                                                                                                     \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                               \
            _name##VAR(a, b, c);                                                                                     \
        }                                                                                                            \
    }

#define DEFINE_STUB_FUNCTION_3(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type, \
                               _param3_type, _default)                                                      \
                                                                                                            \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type);          \
                                                                                                            \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c)                     \
    {                                                                                                       \
        static _name##TYPE _name##VAR = nullptr;                                                            \
        static UINT ulDllSequenceNum = 0;                                                                   \
                                                                                                            \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                     \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                          \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                          \
        }                                                                                                   \
                                                                                                            \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                      \
            return _name##VAR(a, b, c);                                                                     \
        }                                                                                                   \
        else {                                                                                              \
            return _default;                                                                                \
        }                                                                                                   \
    }

#define DEFINE_STUB_FUNCTION_ORD_3(_linkage, _ret_type, _modifiers, _name, _ordinal, _param1_type, _param2_type, \
                                   _param3_type, _default)                                                       \
                                                                                                                 \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type);               \
                                                                                                                 \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c)                          \
    {                                                                                                            \
        static _name##TYPE _name##VAR = nullptr;                                                                 \
        static UINT ulDllSequenceNum = 0;                                                                        \
                                                                                                                 \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                          \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                     \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                               \
        }                                                                                                        \
                                                                                                                 \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                           \
            return _name##VAR(a, b, c);                                                                          \
        }                                                                                                        \
        else {                                                                                                   \
            return _default;                                                                                     \
        }                                                                                                        \
    }

#define DEFINE_STUB_FUNCTION_V4(_linkage, _modifiers, _name, _lookup, _param1_type, _param2_type, _param3_type, \
                                _param4_type)                                                                   \
                                                                                                                \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type)     \
                                                                                                                \
        _linkage void _modifiers                                                                                \
        _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d)                                   \
    {                                                                                                           \
        static _name##TYPE _name##VAR = nullptr;                                                                \
        static UINT ulDllSequenceNum = 0;                                                                       \
                                                                                                                \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                         \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                              \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                              \
        }                                                                                                       \
                                                                                                                \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                          \
            _name##VAR(a, b, c, d);                                                                             \
        }                                                                                                       \
    }

#define DEFINE_STUB_FUNCTION_4(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,      \
                               _param3_type, _param4_type, _default)                                             \
                                                                                                                 \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type); \
                                                                                                                 \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d)          \
    {                                                                                                            \
        static _name##TYPE _name##VAR = nullptr;                                                                 \
        static UINT ulDllSequenceNum = 0;                                                                        \
                                                                                                                 \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                          \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                               \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                               \
        }                                                                                                        \
                                                                                                                 \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                           \
            return _name##VAR(a, b, c, d);                                                                       \
        }                                                                                                        \
        else {                                                                                                   \
            return _default;                                                                                     \
        }                                                                                                        \
    }

#define DEFINE_STUB_FUNCTION_ORD_4(_linkage, _ret_type, _modifiers, _name, _ordinal, _param1_type, _param2_type, \
                                   _param3_type, _param4_type, _default)                                         \
                                                                                                                 \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type); \
                                                                                                                 \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d)          \
    {                                                                                                            \
        static _name##TYPE _name##VAR = nullptr;                                                                 \
        static UINT ulDllSequenceNum = 0;                                                                        \
                                                                                                                 \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                          \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                     \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                               \
        }                                                                                                        \
                                                                                                                 \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                           \
            return _name##VAR(a, b, c, d);                                                                       \
        }                                                                                                        \
        else {                                                                                                   \
            return _default;                                                                                     \
        }                                                                                                        \
    }

#define DEFINE_STUB_FUNCTION_5(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,     \
                               _param3_type, _param4_type, _param5_type, _default)                              \
                                                                                                                \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type, \
                                                        _param5_type);                                          \
                                                                                                                \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,         \
                                        _param5_type e)                                                         \
    {                                                                                                           \
        static _name##TYPE _name##VAR = nullptr;                                                                \
        static UINT ulDllSequenceNum = 0;                                                                       \
                                                                                                                \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                         \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                              \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                              \
        }                                                                                                       \
                                                                                                                \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                          \
            return _name##VAR(a, b, c, d, e);                                                                   \
        }                                                                                                       \
        else {                                                                                                  \
            return _default;                                                                                    \
        }                                                                                                       \
    }

#define DEFINE_STUB_FUNCTION_ORD_5(_linkage, _ret_type, _modifiers, _name, _ordinal, _param1_type, _param2_type, \
                                   _param3_type, _param4_type, _param5_type, _default)                           \
                                                                                                                 \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,  \
                                                        _param5_type);                                           \
                                                                                                                 \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,          \
                                        _param5_type e)                                                          \
    {                                                                                                            \
        static _name##TYPE _name##VAR = nullptr;                                                                 \
        static UINT ulDllSequenceNum = 0;                                                                        \
                                                                                                                 \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                          \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                     \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                               \
        }                                                                                                        \
                                                                                                                 \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                           \
            return _name##VAR(a, b, c, d, e);                                                                    \
        }                                                                                                        \
        else {                                                                                                   \
            return _default;                                                                                     \
        }                                                                                                        \
    }

#define DEFINE_STUB_FUNCTION_6(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,     \
                               _param3_type, _param4_type, _param5_type, _param6_type, _default)                \
                                                                                                                \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type, \
                                                        _param5_type, _param6_type);                            \
                                                                                                                \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,         \
                                        _param5_type e, _param6_type f)                                         \
    {                                                                                                           \
        static _name##TYPE _name##VAR = nullptr;                                                                \
        static UINT ulDllSequenceNum = 0;                                                                       \
                                                                                                                \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                         \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                              \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                              \
        }                                                                                                       \
                                                                                                                \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                          \
            return _name##VAR(a, b, c, d, e, f);                                                                \
        }                                                                                                       \
        else {                                                                                                  \
            return _default;                                                                                    \
        }                                                                                                       \
    }

#define DEFINE_STUB_FUNCTION_V7(_linkage, _modifiers, _name, _lookup, _param1_type, _param2_type, _param3_type,    \
                                _param4_type, _param5_type, _param6_type, _param7_type)                            \
                                                                                                                   \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,        \
                                                    _param5_type, _param6_type, _param7_type);                     \
                                                                                                                   \
    _linkage void _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d, _param5_type e, \
                                   _param6_type f, _param7_type g)                                                 \
    {                                                                                                              \
        static _name##TYPE _name##VAR = nullptr;                                                                   \
        static UINT ulDllSequenceNum = 0;                                                                          \
                                                                                                                   \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                            \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                 \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                 \
        }                                                                                                          \
                                                                                                                   \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                             \
            _name##VAR(a, b, c, d, e, f, g);                                                                       \
        }                                                                                                          \
    }

#define DEFINE_STUB_FUNCTION_7(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,     \
                               _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _default)  \
                                                                                                                \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type, \
                                                        _param5_type, _param6_type, _param7_type);              \
                                                                                                                \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,         \
                                        _param5_type e, _param6_type f, _param7_type g)                         \
    {                                                                                                           \
        static _name##TYPE _name##VAR = nullptr;                                                                \
        static UINT ulDllSequenceNum = 0;                                                                       \
                                                                                                                \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                         \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                              \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                              \
        }                                                                                                       \
                                                                                                                \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                          \
            return _name##VAR(a, b, c, d, e, f, g);                                                             \
        }                                                                                                       \
        else {                                                                                                  \
            return _default;                                                                                    \
        }                                                                                                       \
    }

#define DEFINE_STUB_FUNCTION_8(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,        \
                               _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, \
                               _default)                                                                           \
                                                                                                                   \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,    \
                                                        _param5_type, _param6_type, _param7_type, _param8_type);   \
                                                                                                                   \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,            \
                                        _param5_type e, _param6_type f, _param7_type g, _param8_type h)            \
    {                                                                                                              \
        static _name##TYPE _name##VAR = nullptr;                                                                   \
        static UINT ulDllSequenceNum = 0;                                                                          \
                                                                                                                   \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                            \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                 \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                 \
        }                                                                                                          \
                                                                                                                   \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                             \
            return _name##VAR(a, b, c, d, e, f, g, h);                                                             \
        }                                                                                                          \
        else {                                                                                                     \
            return _default;                                                                                       \
        }                                                                                                          \
    }

#define DEFINE_STUB_FUNCTION_9(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,        \
                               _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, \
                               _param9_type, _default)                                                             \
                                                                                                                   \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,    \
                                                        _param5_type, _param6_type, _param7_type, _param8_type,    \
                                                        _param9_type);                                             \
                                                                                                                   \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,            \
                                        _param5_type e, _param6_type f, _param7_type g, _param8_type h,            \
                                        _param9_type i)                                                            \
    {                                                                                                              \
        static _name##TYPE _name##VAR = nullptr;                                                                   \
        static UINT ulDllSequenceNum = 0;                                                                          \
                                                                                                                   \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                            \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                 \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                 \
        }                                                                                                          \
                                                                                                                   \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                             \
            return _name##VAR(a, b, c, d, e, f, g, h, i);                                                          \
        }                                                                                                          \
        else {                                                                                                     \
            return _default;                                                                                       \
        }                                                                                                          \
    }

#define DEFINE_STUB_FUNCTION_10(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,        \
                                _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, \
                                _param9_type, _param10_type, _default)                                              \
                                                                                                                    \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,     \
                                                        _param5_type, _param6_type, _param7_type, _param8_type,     \
                                                        _param9_type, _param10_type);                               \
                                                                                                                    \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,             \
                                        _param5_type e, _param6_type f, _param7_type g, _param8_type h,             \
                                        _param9_type i, _param10_type j)                                            \
    {                                                                                                               \
        static _name##TYPE _name##VAR = nullptr;                                                                    \
        static UINT ulDllSequenceNum = 0;                                                                           \
                                                                                                                    \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                             \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                  \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                  \
        }                                                                                                           \
                                                                                                                    \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                              \
            return _name##VAR(a, b, c, d, e, f, g, h, i, j);                                                        \
        }                                                                                                           \
        else {                                                                                                      \
            return _default;                                                                                        \
        }                                                                                                           \
    }

#define DEFINE_STUB_FUNCTION_11(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,        \
                                _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, \
                                _param9_type, _param10_type, _param11Type, _default)                                \
                                                                                                                    \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,     \
                                                        _param5_type, _param6_type, _param7_type, _param8_type,     \
                                                        _param9_type, _param10_type, _param11Type);                 \
                                                                                                                    \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,             \
                                        _param5_type e, _param6_type f, _param7_type g, _param8_type h,             \
                                        _param9_type i, _param10_type j, _param11Type k)                            \
    {                                                                                                               \
        static _name##TYPE _name##VAR = nullptr;                                                                    \
        static UINT ulDllSequenceNum = 0;                                                                           \
                                                                                                                    \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                             \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                  \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                  \
        }                                                                                                           \
                                                                                                                    \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                              \
            return _name##VAR(a, b, c, d, e, f, g, h, i, j, k);                                                     \
        }                                                                                                           \
        else {                                                                                                      \
            return _default;                                                                                        \
        }                                                                                                           \
    }

#define DEFINE_STUB_FUNCTION_12(_linkage, _ret_type, _modifiers, _name, _lookup, _param1_type, _param2_type,        \
                                _param3_type, _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, \
                                _param9_type, _param10_type, _param11Type, _param12Type, _default)                  \
                                                                                                                    \
    _linkage typedef _ret_type(_modifiers *_name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,     \
                                                        _param5_type, _param6_type, _param7_type, _param8_type,     \
                                                        _param9_type, _param10_type, _param11Type, _param12Type);   \
                                                                                                                    \
    _linkage _ret_type _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d,             \
                                        _param5_type e, _param6_type f, _param7_type g, _param8_type h,             \
                                        _param9_type i, _param10_type j, _param11Type k, _param12Type l)            \
    {                                                                                                               \
        static _name##TYPE _name##VAR = nullptr;                                                                    \
        static UINT ulDllSequenceNum = 0;                                                                           \
                                                                                                                    \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                             \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                  \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                  \
        }                                                                                                           \
                                                                                                                    \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                              \
            return _name##VAR(a, b, c, d, e, f, g, h, i, j, k, l);                                                  \
        }                                                                                                           \
        else {                                                                                                      \
            return _default;                                                                                        \
        }                                                                                                           \
    }

#define DEFINE_STUB_FUNCTION_V12(_linkage, _modifiers, _name, _lookup, _param1_type, _param2_type, _param3_type,     \
                                 _param4_type, _param5_type, _param6_type, _param7_type, _param8_type, _param9_type, \
                                 _param10_type, _param11Type, _param12Type)                                          \
                                                                                                                     \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,          \
                                                    _param5_type, _param6_type, _param7_type, _param8_type,          \
                                                    _param9_type, _param10_type, _param11Type, _param12Type);        \
                                                                                                                     \
    _linkage void _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d, _param5_type e,   \
                                   _param6_type f, _param7_type g, _param8_type h, _param9_type i, _param10_type j,  \
                                   _param11Type k, _param11Type l)                                                   \
    {                                                                                                                \
        static _name##TYPE _name##VAR = nullptr;                                                                     \
        static UINT ulDllSequenceNum = 0;                                                                            \
                                                                                                                     \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                              \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), _lookup);                                   \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                   \
        }                                                                                                            \
                                                                                                                     \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                               \
            _name##VAR(a, b, c, d, e, f, g, h, i, j, k, l);                                                          \
        }                                                                                                            \
    }

#define DEFINE_STUB_FUNCTION_ORD_V12(_linkage, _modifiers, _name, _ordinal, _param1_type, _param2_type, _param3_type, \
                                     _param4_type, _param5_type, _param6_type, _param7_type, _param8_type,            \
                                     _param9_type, _param10_type, _param11Type, _param12Type)                         \
                                                                                                                      \
    _linkage typedef void(_modifiers * _name##TYPE)(_param1_type, _param2_type, _param3_type, _param4_type,           \
                                                    _param5_type, _param6_type, _param7_type, _param8_type,           \
                                                    _param9_type, _param10_type, _param11Type, _param12Type);         \
                                                                                                                      \
    _linkage void _modifiers _name(_param1_type a, _param2_type b, _param3_type c, _param4_type d, _param5_type e,    \
                                   _param6_type f, _param7_type g, _param8_type h, _param9_type i, _param10_type j,   \
                                   _param11Type k, _param11Type l)                                                    \
    {                                                                                                                 \
        static _name##TYPE _name##VAR = nullptr;                                                                      \
        static UINT ulDllSequenceNum = 0;                                                                             \
                                                                                                                      \
        if ((ulDllSequenceNum != g_ulDllSequenceNum) || (nullptr == GetMAPIHandle())) {                               \
            _name##VAR = (_name##TYPE)::GetProcAddress(GetPrivateMAPI(), (LPSTR)(_ordinal));                          \
            ulDllSequenceNum = g_ulDllSequenceNum;                                                                    \
        }                                                                                                             \
                                                                                                                      \
        if ((nullptr != _name##VAR) && (nullptr != GetMAPIHandle())) {                                                \
            _name##VAR(a, b, c, d, e, f, g, h, i, j, k, l);                                                           \
        }                                                                                                             \
    }

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, MAPILogonEx, ExpandFunction(MAPILogonEx, 20),
                       ULONG_PTR, LPTSTR, LPTSTR, ULONG, LPMAPISESSION *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(LINKAGE_EXTERN_C, STDAPICALLTYPE, MAPIUninitialize, ExpandFunction(MAPIUninitialize, 0))

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, SCODE, STDMETHODCALLTYPE, MAPIAllocateBuffer,
                       ExpandFunction(MAPIAllocateBuffer, 8), ULONG, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDMETHODCALLTYPE, MAPIAllocateMore,
                       ExpandFunction(MAPIAllocateMore, 12), ULONG, LPVOID, LPVOID FAR *, (SCODE)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, MAPIReallocateBuffer,
                       ExpandFunction(MAPIReallocateBuffer, 12), LPVOID, ULONG, LPVOID *, (SCODE)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, MAPIAdminProfiles,
                       ExpandFunction(MAPIAdminProfiles, 8), ULONG, LPPROFADMIN FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, MAPIInitialize, ExpandFunction(MAPIInitialize, 4),
                       LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, LaunchWizard, ExpandFunction(LaunchWizard, 20), HWND,
                       ULONG, LPCSTR FAR *, ULONG, LPSTR, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, MAPIOpenFormMgr, ExpandFunction(MAPIOpenFormMgr, 8),
                       LPMAPISESSION, LPMAPIFORMMGR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, MAPIOpenLocalFormContainer,
                       ExpandFunction(MAPIOpenLocalFormContainer, 4), LPMAPIFORMCONTAINER FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScInitMapiUtil, ExpandFunction(ScInitMapiUtil, 4),
                       ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(LINKAGE_EXTERN_C, STDAPICALLTYPE, DeinitMapiUtil, ExpandFunction(DeinitMapiUtil, 0))

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrAllocAdviseSink,
                       ExpandFunction(HrAllocAdviseSink, 12), LPNOTIFCALLBACK, LPVOID, LPMAPIADVISESINK FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrThisThreadAdviseSink,
                       ExpandFunction(HrThisThreadAdviseSink, 8), LPMAPIADVISESINK, LPMAPIADVISESINK FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrDispatchNotifications,
                       ExpandFunction(HrDispatchNotifications, 4), ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScBinFromHexBounded,
                       ExpandFunction(ScBinFromHexBounded, 12), LPTSTR, LPBYTE, ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FBinFromHex, ExpandFunction(FBinFromHex, 8), LPTSTR,
                       LPBYTE, FALSE)

DEFINE_STUB_FUNCTION_V3(LINKAGE_EXTERN_C, STDAPICALLTYPE, HexFromBin, ExpandFunction(HexFromBin, 12), LPBYTE, int,
                        LPTSTR)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrGetAutoDiscoverXML,
                       ExpandFunction(HrGetAutoDiscoverXML, 20), LPCWSTR, LPCWSTR, HANDLE, ULONG, IStream **,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_10(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, BuildDisplayTable,
                        ExpandFunction(BuildDisplayTable, 40), LPALLOCATEBUFFER, LPALLOCATEMORE, LPFREEBUFFER, LPMALLOC,
                        HINSTANCE, UINT, LPDTPAGE, ULONG, LPMAPITABLE *, LPTABLEDATA *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, MAPIInitIdle, ExpandFunction(MAPIInitIdle, 4), LPVOID,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(LINKAGE_EXTERN_C, STDAPICALLTYPE, MAPIDeinitIdle, ExpandFunction(MAPIDeinitIdle, 0))

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, FTG, STDAPICALLTYPE, FtgRegisterIdleRoutine,
                       ExpandFunction(FtgRegisterIdleRoutine, 20), PFNIDLE, LPVOID, short, ULONG, USHORT, nullptr)

DEFINE_STUB_FUNCTION_V2(LINKAGE_EXTERN_C, STDAPICALLTYPE, EnableIdleRoutine, ExpandFunction(EnableIdleRoutine, 8), FTG,
                        BOOL)

DEFINE_STUB_FUNCTION_V1(LINKAGE_EXTERN_C, STDAPICALLTYPE, DeregisterIdleRoutine,
                        ExpandFunction(DeregisterIdleRoutine, 4), FTG)

DEFINE_STUB_FUNCTION_V7(LINKAGE_EXTERN_C, STDAPICALLTYPE, ChangeIdleRoutine, ExpandFunction(ChangeIdleRoutine, 28), FTG,
                        PFNIDLE, LPVOID, short, ULONG, USHORT, USHORT)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, CreateIProp, ExpandFunction(CreateIProp, 24), LPCIID,
                       ALLOCATEBUFFER FAR *, ALLOCATEMORE FAR *, FREEBUFFER FAR *, LPVOID, LPPROPDATA FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_9(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, CreateTable, ExpandFunction(CreateTable, 36), LPCIID,
                       ALLOCATEBUFFER FAR *, ALLOCATEMORE FAR *, FREEBUFFER FAR *, LPVOID, ULONG, ULONG,
                       LPSPropTagArray, LPTABLEDATA FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, int, WINAPI, MNLS_lstrlenW, ExpandFunction(MNLS_lstrlenW, 4), LPCWSTR, 0)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, int, WINAPI, MNLS_lstrcmpW, ExpandFunction(MNLS_lstrcmpW, 8), LPCWSTR, LPCWSTR,
                       0)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, LPWSTR, WINAPI, MNLS_lstrcpyW, ExpandFunction(MNLS_lstrcpyW, 8), LPWSTR,
                       LPCWSTR, nullptr)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, int, WINAPI, MNLS_CompareStringW, ExpandFunction(MNLS_CompareStringW, 24),
                       LCID, DWORD, LPCWSTR, int, LPCWSTR, int, 0)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, int, WINAPI, MNLS_MultiByteToWideChar,
                       ExpandFunction(MNLS_MultiByteToWideChar, 24), UINT, DWORD, LPCSTR, int, LPWSTR, int,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, int, WINAPI, MNLS_WideCharToMultiByte,
                       ExpandFunction(MNLS_WideCharToMultiByte, 32), UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR,
                       BOOL FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, WINAPI, MNLS_IsBadStringPtrW, ExpandFunction(MNLS_IsBadStringPtrW, 8),
                       LPCWSTR, UINT, TRUE)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FEqualNames, ExpandFunction(FEqualNames, 8),
                       LPMAPINAMEID, LPMAPINAMEID, FALSE)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, WrapStoreEntryID,
                       ExpandFunction(WrapStoreEntryID, 24), ULONG, LPTSTR, ULONG, LPENTRYID, ULONG *, LPENTRYID *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, WINAPI, IsBadBoundedStringPtr, ExpandFunction(IsBadBoundedStringPtr, 8),
                       const void FAR *, UINT, FALSE)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrQueryAllRows, ExpandFunction(HrQueryAllRows, 24),
                       LPMAPITABLE, LPSPropTagArray, LPSRestriction, LPSSortOrderSet, LONG, LPSRowSet FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScCreateConversationIndex,
                       ExpandFunction(ScCreateConversationIndex, 16), ULONG, LPBYTE, ULONG FAR *, LPBYTE FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, PropCopyMore, ExpandFunction(PropCopyMore, 16),
                       LPSPropValue, LPSPropValue, ALLOCATEMORE *, LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, UlPropSize, ExpandFunction(UlPropSize, 4), LPSPropValue,
                       0)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FPropContainsProp, ExpandFunction(FPropContainsProp, 12),
                       LPSPropValue, LPSPropValue, ULONG, FALSE)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FPropCompareProp, ExpandFunction(FPropCompareProp, 12),
                       LPSPropValue, ULONG, LPSPropValue, FALSE)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, LONG, STDAPICALLTYPE, LPropCompareProp, ExpandFunction(LPropCompareProp, 8),
                       LPSPropValue, LPSPropValue, 0)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrAddColumns, ExpandFunction(HrAddColumns, 16),
                       LPMAPITABLE, LPSPropTagArray, LPALLOCATEBUFFER, LPFREEBUFFER, MAPI_E_CALL_FAILED)

typedef void(FAR *HrAddColumnsEx5ParamType)(LPSPropTagArray);

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrAddColumnsEx, ExpandFunction(HrAddColumnsEx, 20),
                       LPMAPITABLE, LPSPropTagArray, LPALLOCATEBUFFER, LPFREEBUFFER, HrAddColumnsEx5ParamType,
                       MAPI_E_CALL_FAILED)

const FILETIME ZERO_FILETIME = {0, 0};

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtMulDwDw, ExpandFunction(FtMulDwDw, 8), DWORD,
                       DWORD, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtAddFt, ExpandFunction(FtAddFt, 16), FILETIME,
                       FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtAdcFt, ExpandFunction(FtAdcFt, 20), FILETIME,
                       FILETIME, WORD FAR *, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtSubFt, ExpandFunction(FtSubFt, 16), FILETIME,
                       FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtMulDw, ExpandFunction(FtMulDw, 12), DWORD,
                       FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE, FtNegFt, ExpandFunction(FtNegFt, 8), FILETIME,
                       ZERO_FILETIME)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, UlAddRef, ExpandFunction(UlAddRef, 4), LPVOID, 1)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, UlRelease, ExpandFunction(UlRelease, 4), LPVOID, 1)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, LPTSTR, STDAPICALLTYPE, SzFindCh, ExpandFunction(SzFindCh, 8), LPCTSTR, USHORT,
                       nullptr)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, LPTSTR, STDAPICALLTYPE, SzFindLastCh, ExpandFunction(SzFindLastCh, 8), LPCTSTR,
                       USHORT, nullptr)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, LPTSTR, STDAPICALLTYPE, SzFindSz, ExpandFunction(SzFindSz, 8), LPCTSTR,
                       LPCTSTR, nullptr)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, unsigned int, STDAPICALLTYPE, UFromSz, ExpandFunction(UFromSz, 4), LPCTSTR, 0)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrGetOneProp, ExpandFunction(HrGetOneProp, 12),
                       LPMAPIPROP, ULONG, LPSPropValue FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrSetOneProp, ExpandFunction(HrSetOneProp, 8),
                       LPMAPIPROP, LPSPropValue, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FPropExists, ExpandFunction(FPropExists, 8), LPMAPIPROP,
                       ULONG, FALSE)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, LPSPropValue, STDAPICALLTYPE, PpropFindProp, ExpandFunction(PpropFindProp, 12),
                       LPSPropValue, ULONG, ULONG, nullptr)

DEFINE_STUB_FUNCTION_V1(LINKAGE_EXTERN_C, STDAPICALLTYPE, FreePadrlist, ExpandFunction(FreePadrlist, 4), LPADRLIST)

DEFINE_STUB_FUNCTION_V1(LINKAGE_EXTERN_C, STDAPICALLTYPE, FreeProws, ExpandFunction(FreeProws, 4), LPSRowSet)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrSzFromEntryID, ExpandFunction(HrSzFromEntryID, 12),
                       ULONG, LPENTRYID, LPTSTR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrEntryIDFromSz, ExpandFunction(HrEntryIDFromSz, 12),
                       LPTSTR, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_NO_EXTERN_C, HRESULT, STDAPICALLTYPE, HrComposeEID, ExpandFunction(HrComposeEID, 28),
                       LPMAPISESSION, ULONG, LPBYTE, ULONG, LPENTRYID, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrDecomposeEID, ExpandFunction(HrDecomposeEID, 28),
                       LPMAPISESSION, ULONG, LPENTRYID, ULONG FAR *, LPENTRYID FAR *, ULONG FAR *, LPENTRYID FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrComposeMsgID, ExpandFunction(HrComposeMsgID, 24),
                       LPMAPISESSION, ULONG, LPBYTE, ULONG, LPENTRYID, LPTSTR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrDecomposeMsgID,
                       ExpandFunction(HrDecomposeMsgID, 24), LPMAPISESSION, LPTSTR, ULONG FAR *, LPENTRYID FAR *,
                       ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, OpenStreamOnFile,
                       ExpandFunction(OpenStreamOnFile, 24), LPALLOCATEBUFFER, LPFREEBUFFER, ULONG, LPCTSTR, LPCTSTR,
                       LPSTREAM FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, OpenTnefStream, ExpandFunction(OpenTnefStream, 28),
                       LPVOID, LPSTREAM, LPTSTR, ULONG, LPMESSAGE, WORD, LPITNEF FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, OpenTnefStreamEx,
                       ExpandFunction(OpenTnefStreamEx, 32), LPVOID, LPSTREAM, LPTSTR, ULONG, LPMESSAGE, WORD,
                       LPADRBOOK, LPITNEF FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, GetTnefStreamCodepage,
                       ExpandFunction(GetTnefStreamCodepage, 12), LPSTREAM, ULONG FAR *, ULONG FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, UlFromSzHex, ExpandFunction(UlFromSzHex, 4), LPCTSTR, 0)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScCountNotifications,
                       ExpandFunction(ScCountNotifications, 12), int, LPNOTIFICATION, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScCopyNotifications,
                       ExpandFunction(ScCopyNotifications, 16), int, LPNOTIFICATION, LPVOID, ULONG FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScRelocNotifications,
                       ExpandFunction(ScRelocNotifications, 20), int, LPNOTIFICATION, LPVOID, LPVOID, ULONG FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScCountProps, ExpandFunction(ScCountProps, 12), int,
                       LPSPropValue, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScCopyProps, ExpandFunction(ScCopyProps, 16), int,
                       LPSPropValue, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScRelocProps, ExpandFunction(ScRelocProps, 20), int,
                       LPSPropValue, LPVOID, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, LPSPropValue, STDAPICALLTYPE, LpValFindProp, ExpandFunction(LpValFindProp, 12),
                       ULONG, ULONG, LPSPropValue, nullptr)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScDupPropset, ExpandFunction(ScDupPropset, 16), int,
                       LPSPropValue, LPALLOCATEBUFFER, LPSPropValue FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FBadRglpszW, ExpandFunction(FBadRglpszW, 8),
                       LPWSTR FAR *, ULONG, TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FBadRowSet, ExpandFunction(FBadRowSet, 4), LPSRowSet,
                       TRUE)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FBadRglpNameID, ExpandFunction(FBadRglpNameID, 8),
                       LPMAPINAMEID FAR *, ULONG, TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadPropTag, ExpandFunction(FBadPropTag, 4), ULONG,
                       TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadRow, ExpandFunction(FBadRow, 4), LPSRow, TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadProp, ExpandFunction(FBadProp, 4), LPSPropValue,
                       TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadColumnSet, ExpandFunction(FBadColumnSet, 4),
                       LPSPropTagArray, TRUE)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, RTFSync, ExpandFunction(RTFSync, 12), LPMESSAGE,
                       ULONG, BOOL FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, WrapCompressedRTFStream,
                       ExpandFunction(WrapCompressedRTFStream, 12), LPSTREAM, ULONG, LPSTREAM FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, __ValidateParameters,
                       ExpandFunction(__ValidateParameters, 8), METHODS, void *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, __CPPValidateParameters,
                       ExpandFunction(__CPPValidateParameters, 8), METHODS, const LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrValidateParameters,
                       ExpandFunction(HrValidateParameters, 8), METHODS, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadSortOrderSet, ExpandFunction(FBadSortOrderSet, 4),
                       LPSSortOrderSet, TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE, FBadEntryList, ExpandFunction(FBadEntryList, 4),
                       LPENTRYLIST, TRUE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, FBadRestriction, ExpandFunction(FBadRestriction, 4),
                       LPSRestriction, TRUE)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScUNCFromLocalPath,
                       ExpandFunction(ScUNCFromLocalPath, 12), LPSTR, LPSTR, UINT, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, ScLocalPathFromUNC,
                       ExpandFunction(ScLocalPathFromUNC, 12), LPSTR, LPSTR, UINT, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrIStorageFromStream,
                       ExpandFunction(HrIStorageFromStream, 16), LPUNKNOWN, LPCIID, ULONG, LPSTORAGE FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrValidateIPMSubtree,
                       ExpandFunction(HrValidateIPMSubtree, 20), LPMDB, ULONG, ULONG FAR *, LPSPropValue FAR *,
                       LPMAPIERROR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, OpenIMsgSession, ExpandFunction(OpenIMsgSession, 12),
                       LPMALLOC, ULONG, LPMSGSESS FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V1(LINKAGE_EXTERN_C, STDAPICALLTYPE, CloseIMsgSession, ExpandFunction(CloseIMsgSession, 4),
                        LPMSGSESS)

DEFINE_STUB_FUNCTION_11(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, OpenIMsgOnIStg, ExpandFunction(OpenIMsgOnIStg, 44),
                        LPMSGSESS, LPALLOCATEBUFFER, LPALLOCATEMORE, LPFREEBUFFER, LPMALLOC, LPVOID, LPSTORAGE,
                        MSGCALLRELEASE FAR *, ULONG, ULONG, LPMESSAGE FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, SetAttribIMsgOnIStg,
                       ExpandFunction(SetAttribIMsgOnIStg, 16), LPVOID, LPSPropTagArray, LPSPropAttrArray,
                       LPSPropProblemArray FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, GetAttribIMsgOnIStg,
                       ExpandFunction(GetAttribIMsgOnIStg, 12), LPVOID, LPSPropTagArray, LPSPropAttrArray FAR *,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE, MapStorageSCode, ExpandFunction(MapStorageSCode, 4),
                       SCODE, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, SCODE, STDMETHODCALLTYPE, ScMAPIXFromSMAPI, "ScMAPIXFromSMAPI", LHANDLE, ULONG,
                       LPCIID, LPMAPISESSION FAR *, MAPI_E_CALL_FAILED);

DEFINE_STUB_FUNCTION_11(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIAddress, "MAPIAddress", LHANDLE, ULONG_PTR, LPSTR,
                        ULONG, LPSTR, ULONG, lpMapiRecipDesc, FLAGS, ULONG, LPULONG, lpMapiRecipDesc FAR *,
                        (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIReadMail, "MAPIReadMail", LHANDLE, ULONG_PTR, LPSTR,
                       FLAGS, ULONG, lpMapiMessage FAR *, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIResolveName, "MAPIResolveName", LHANDLE, ULONG_PTR,
                       LPSTR, FLAGS, ULONG, lpMapiRecipDesc FAR *, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPISendDocuments, "MAPISendDocuments", ULONG_PTR, LPSTR,
                       LPSTR, LPSTR, ULONG, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPILogon, "MAPILogon", ULONG_PTR, LPSTR, LPSTR, FLAGS,
                       ULONG, LPLHANDLE, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPILogoff, "MAPILogoff", LHANDLE, ULONG_PTR, FLAGS, ULONG,
                       (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPISendMail, "MAPISendMail", LHANDLE, ULONG_PTR,
                       lpMapiMessage, FLAGS, ULONG, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPISaveMail, "MAPISaveMail", LHANDLE, ULONG_PTR,
                       lpMapiMessage, FLAGS, ULONG, LPSTR, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIFindNext, "MAPIFindNext", LHANDLE, ULONG_PTR, LPSTR,
                       LPSTR, FLAGS, ULONG, LPSTR, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIDeleteMail, "MAPIDeleteMail", LHANDLE, ULONG_PTR, LPSTR,
                       FLAGS, ULONG, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, ULONG, FAR PASCAL, MAPIDetails, "MAPIDetails", LHANDLE, ULONG_PTR,
                       lpMapiRecipDesc, FLAGS, ULONG, (ULONG)MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V1(LINKAGE_EXTERN_C, STDAPICALLTYPE, MAPICrashRecovery, ExpandFunction(MAPICrashRecovery, 4),
                        ULONG)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE, MAPIFreeBuffer, ExpandFunction(MAPIFreeBuffer, 4),
                       LPVOID, 0)

DEFINE_STUB_FUNCTION_0(LINKAGE_EXTERN_C, LPMALLOC, STDAPICALLTYPE, MAPIGetDefaultMalloc,
                       ExpandFunction(MAPIGetDefaultMalloc, 0), nullptr)

DEFINE_STUB_FUNCTION_6(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, OpenStreamOnFileW,
                       ExpandFunction(OpenStreamOnFileW, 24), LPALLOCATEBUFFER, LPFREEBUFFER, ULONG, LPWSTR, LPWSTR,
                       LPSTREAM FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrCreateNewWrappedObject,
                       ExpandFunction(HrCreateNewWrappedObject, 28), void *, ULONG, ULONG, const IID *, const ULONG *,
                       BOOL, void **, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrOpenOfflineObj,
                       ExpandFunction(HrOpenOfflineObj, 20), ULONG, LPCWSTR, const GUID *, const GUID *,
                       IMAPIOfflineMgr **, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE, HrCreateOfflineObj,
                       ExpandFunction(HrCreateOfflineObj, 12), ULONG, MAPIOFFLINE_CREATEINFO *, IMAPIOfflineMgr **,
                       MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE, WrapCompressedRTFStreamEx,
                       ExpandFunction(WrapCompressedRTFStreamEx, 16), LPSTREAM, CONST RTF_WCSINFO *, LPSTREAM *,
                       RTF_WCSRETINFO *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, BOOL, WINAPI, GetDefCachedMode, ExpandFunction(GetDefCachedMode, 4), BOOL *,
                       FALSE)

DEFINE_STUB_FUNCTION_1(LINKAGE_EXTERN_C, BOOL, WINAPI, GetDefCachedModeDownloadPubFoldFavs,
                       ExpandFunction(GetDefCachedModeDownloadPubFoldFavs, 4), BOOL *, FALSE)

DEFINE_STUB_FUNCTION_9(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryWithExchangeContext,
                       ExpandFunction(HrOpenABEntryWithExchangeContext, 36), LPMAPISESSION, LPMAPIUID, LPADRBOOK, ULONG,
                       LPENTRYID, LPCIID, ULONG, ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_12(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrDoABDetailsWithExchangeContext,
                        ExpandFunction(HrDoABDetailsWithExchangeContext, 48), LPMAPISESSION, LPMAPIUID, LPADRBOOK,
                        ULONG_PTR *, LPFNDISMISS, LPVOID, ULONG, LPENTRYID, LPFNBUTTON, LPVOID, LPSTR, ULONG,
                        MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_11(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrDoABDetailsWithProviderUID,
                        ExpandFunction(HrDoABDetailsWithProviderUID, 44), LPMAPIUID, LPADRBOOK, ULONG_PTR *,
                        LPFNDISMISS, LPVOID, ULONG, LPENTRYID, LPFNBUTTON, LPVOID, LPSTR, ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryUsingDefaultContext,
                       ExpandFunction(HrOpenABEntryUsingDefaultContext, 32), LPMAPISESSION, LPADRBOOK, ULONG, LPENTRYID,
                       LPCIID, ULONG, ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryWithProviderUID,
                       ExpandFunction(HrOpenABEntryWithProviderUID, 32), LPMAPIUID, LPADRBOOK, ULONG, LPENTRYID, LPCIID,
                       ULONG, ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryWithProviderUIDSupport,
                       ExpandFunction(HrOpenABEntryWithProviderUIDSupport, 32), LPMAPIUID, LPMAPISUP, ULONG, LPENTRYID,
                       LPCIID, ULONG, ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryWithResolvedRow,
                       ExpandFunction(HrOpenABEntryWithResolvedRow, 32), LPSRow, LPADRBOOK, ULONG, LPENTRYID, LPCIID,
                       ULONG, ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_9(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrCompareABEntryIDsWithExchangeContext,
                       ExpandFunction(HrCompareABEntryIDsWithExchangeContext, 36), LPMAPISESSION, LPMAPIUID, LPADRBOOK,
                       ULONG, LPENTRYID, ULONG, LPENTRYID, ULONG, ULONG *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrOpenABEntryWithSupport,
                       ExpandFunction(HrOpenABEntryWithSupport, 28), LPMAPISUP, ULONG, LPENTRYID, LPCIID, ULONG,
                       ULONG *, LPUNKNOWN *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(LINKAGE_EXTERN_C, HRESULT, WINAPI, HrGetGALFromEmsmdbUID,
                       ExpandFunction(HrGetGALFromEmsmdbUID, 20), LPMAPISESSION, LPADRBOOK, LPMAPIUID, ULONG *,
                       LPENTRYID *, MAPI_E_CALL_FAILED)