// gdiobject class
#pragma once

class PyCGdiObject : public ui_assoc_CObject {
friend static PyObject *ui_dc_select_object (PyObject *self, PyObject *args);
public:
  static ui_type_CObject type;
  static CGdiObject *GetGdiObject( PyObject *self, DWORD type=0 );
  CGdiObject *GetGdiObject( DWORD type=0 ) {return GetGdiObject( this, type );}
  static CFont *GetFont (PyObject *self) { return (CFont *)GetGdiObject(self, OBJ_FONT); }
  CFont *GetFont() { return GetFont (this); }
  static CPen *GetPen (PyObject *self) { return (CPen *)GetGdiObject (self, OBJ_PEN); }
  CPen *GetPen() { return GetPen (this); }
  static CBrush *GetBrush (PyObject *self) { return (CBrush *)GetGdiObject (self, OBJ_BRUSH); }
  CBrush *GetBrush() { return GetBrush (this); }
  static CBitmap *GetBitmap (PyObject *self) { return (CBitmap *)GetGdiObject(self, OBJ_BITMAP); }
  CBitmap *GetBitmap() { return GetBitmap (this); }
  BOOL m_deleteObject;

protected:
  PyCGdiObject()
	: m_deleteObject(TRUE)
	{ }
  ~PyCGdiObject();
  virtual void DoKillAssoc( BOOL bDestructing = FALSE );
  virtual bool CheckCppObject(ui_type *ui_type_check) const;

  // virtuals for the Python interface.
  virtual CString repr();
};
