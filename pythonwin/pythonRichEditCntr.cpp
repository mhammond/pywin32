/* win32RichEditCntr : implementation file

    Created March 1996, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32doc.h"
#include "win32control.h"
#include "win32RichEdit.h"

#include "pythonRichEditCntr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonCntrItem implementation

IMPLEMENT_SERIAL(CPythonCntrItem, CRichEditCntrItem, 0)

CPythonCntrItem::CPythonCntrItem(REOBJECT *preo, CRichEditDoc *pContainer) : CRichEditCntrItem(preo, pContainer) {}

/////////////////////////////////////////////////////////////////////////////
// CPythonCntrItem diagnostics

#ifdef _DEBUG
void CPythonCntrItem::AssertValid() const { CRichEditCntrItem::AssertValid(); }

void CPythonCntrItem::Dump(CDumpContext &dc) const { CRichEditCntrItem::Dump(dc); }
#endif

/////////////////////////////////////////////////////////////////////////////
