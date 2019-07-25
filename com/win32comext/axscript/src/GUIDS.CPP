/*
 * FILENAME:
 *	guids.cpp
 *
 * DESCRIPTION:
 *	Define the guids used by the project. Both internally defined
 *	guids as well as guids from external sources.
 */
#include <objbase.h>
#pragma data_seg(".text")

#include "initguid.h"

// === ActiveX Guids ===
#if PMAC
#define INITGUID
#endif

// === Project Guids ===
#include "GUIDS.H"
// NOTE - The standard "activscp.h" header is not good enough -
// need to use IE4 SDK or MSVC6 etc
#include "activscp.h"
// #include "multinfo.h"

#if !WIN16
#pragma data_seg()
#endif  // !WIN16
