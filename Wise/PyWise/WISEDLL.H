// WiseDLL.h: definitions for functions that are called by Wise Installation
//            System executables.

#ifdef WIN32

typedef struct {
   DWORD wStructLen;   // The length of the structure
   DWORD hWnd;         // Handle to main window
   DWORD wMaxReplaces; // Maximum number of replaces
   DWORD wRepNameWidth;// The width of a replace name
   DWORD wRepStrWidth; // The width of each replace string
   DWORD wCurrReps;    // Current number of replace strings
   char *lpszRepName;  // The actual replace names
   char *lpszRepStr;   // The actual replace values
   DWORD bRunMode;     // The installation mode
   DWORD fLogFile;     // A file handle to the log file
   char *lpszParam;    // String parameter from Wise Installation System
} DLLCALLPARAMS;

typedef DLLCALLPARAMS *LPDLLCALLPARAMS;

#else

typedef struct {
   WORD wStructLen;    // The length of the structure
   HWND hWnd;          // Handle to main window
   WORD wMaxReplaces;  // Maximum number of replaces
   WORD wRepNameWidth; // The width of a replace name
   WORD wRepStrWidth;  // The width of each replace string
   WORD wCurrReps;     // Current number of replace strings
   char *lpszRepName;  // The actual replace names
   char *lpszRepStr;   // The actual replace values
   BYTE bRunMode;      // The installation mode
                       // NOTE: For Borland C++ change to: WORD wRunMode;
   HFILE fLogFile;     // A file handle to the log file
   char *lpszParam;    // String parameter from Wise Installation System
} DLLCALLPARAMS;

typedef DLLCALLPARAMS far *LPDLLCALLPARAMS;

#endif

