// pythondoc.cpp : implementation of the CPythonDoc class
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"

#include "pythonwin.h"
#include "pythondoc.h"
#include "win32ui.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPythonDoc, CDocument);

BEGIN_MESSAGE_MAP(CPythonDoc, CDocument)
//{{AFX_MSG_MAP(CPythonDoc)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPythonDoc::OnUpdateFileSave(CCmdUI *pCmdUI) { pCmdUI->Enable(IsModified()); }
