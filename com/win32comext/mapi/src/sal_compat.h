/*
 * SAL (Source Annotation Language) annotations are MSVC-only.
 * On other compilers (GCC/Clang) they are no-ops; define them here so the
 * MAPI headers compile without MSVC's sal.h providing them.
 *
 * MinGW does have its own sal.h, but there's 2 outstanding issues:
 * 1. Missing `__deref_out_ecount_full` https://github.com/mingw-w64/mingw-w64/issues/173
 * 2. `__in`/`__out` macros conflict with  argument names in libstdc++
 * https://github.com/mingw-w64/mingw-w64/blob/0636d42e1a9ee944e51170bd84701ef114d418ff/mingw-w64-headers/include/sal.h#L523-L529
 */
#ifndef __in
#define __in
#endif
#ifndef __in_opt
#define __in_opt
#endif
#ifndef __out
#define __out
#endif
#ifndef __deref_out_ecount_full
#define __deref_out_ecount_full(x)
#endif
