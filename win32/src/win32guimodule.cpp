// A source file which includes the SWIG generated code.
//
// SWIG is capable of generating a number of different versions
// so this source file controls which one is actually used! 

#ifndef DISTUTILS_BUILD /* Not needed for distutils based builds */
#ifdef UNDER_CE
#include "win32guimodule_wince.cpp"
#else
#include "win32guimodule_win32.cpp"
#endif
#endif /* DISTUTILS_BUILD */
