// PythonRichEditDoc.cpp
#include "stdafx.h"
#include "pythondoc.h"
#include "pythonRichEditCntr.h"
#include "pythonRichEditDoc.h"
// @doc

IMPLEMENT_DYNCREATE(CPythonRichEditDoc, CRichEditDoc );

BEGIN_MESSAGE_MAP(CPythonRichEditDoc, CRichEditDoc)
	//{{AFX_MSG_MAP(CPythonDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPythonRichEditDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

CRichEditCntrItem* CPythonRichEditDoc::CreateClientItem( REOBJECT* preo ) const
{
	return new CPythonCntrItem( preo, (CPythonRichEditDoc*)this);
}
