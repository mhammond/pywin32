// pythonppage.h : header file
//
#ifndef __PYTHONPPAGE_H__
#define __PYTHONPPAGE_H__

// bit of a hack
#ifdef _DEBUG
BOOL AFXAPI _AfxCheckDialogTemplate(LPCTSTR lpszResource, BOOL bInvisibleChild);
#endif

/////////////////////////////////////////////////////////////////////////////
// CPythonPropertyPage dialog

class CPythonPropertyPage : public CPythonPropertyPageFramework<CPropertyPage> {
    DECLARE_DYNAMIC(CPythonPropertyPage)

   protected:
    // Support for indirect creation
    HGLOBAL hSaved;

    // Construction
   public:
    CPythonPropertyPage(UINT id, UINT caption = 0);
    CPythonPropertyPage(LPCTSTR id, UINT caption = 0);
    ~CPythonPropertyPage();
    virtual void PostNcDestroy();

    BOOL SetTemplate(HGLOBAL tpl);

   private:
    void CommonConstruct();
#ifdef _DEBUG
    virtual void Dump(CDumpContext &dc) const;
#endif
};
#endif  // __filename_h__
