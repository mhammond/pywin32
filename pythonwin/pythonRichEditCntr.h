// PythonRichEditCntr - Python container object.

//	PyCRichEditCntrItem
class CPythonCntrItem : public CRichEditCntrItem {
    DECLARE_SERIAL(CPythonCntrItem)

    // Constructors
   public:
    CPythonCntrItem(REOBJECT *preo = NULL, CRichEditDoc *pContainer = NULL);
    // Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
    //  IMPLEMENT_SERIALIZE requires the class have a constructor with
    //  zero arguments.  Normally, OLE items are constructed with a
    //  non-NULL document pointer.

    // Attributes
   public:
    CRichEditDoc *GetDocument() { return (CRichEditDoc *)COleClientItem::GetDocument(); }
    CRichEditView *GetActiveView() { return (CRichEditView *)COleClientItem::GetActiveView(); }

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CWordPadCntrItem)
   public:
   protected:
    //}}AFX_VIRTUAL

    // Implementation
   public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext &dc) const;
#endif
};
