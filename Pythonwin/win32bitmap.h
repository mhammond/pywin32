// win32bitmap.h : header file
//
//
/////////////////////////////////////////////////////////
//
//	ui_bitmap
class ui_bitmap : public PyCGdiObject {
public:
	static ui_type_CObject type;
	MAKE_PY_CTOR(ui_bitmap)
	static PyObject *create (PyObject *self, PyObject *args);
	static PyObject *create_from_handle (PyObject *self, PyObject *args);
	
	void ClearSupportData();

	CPalette *pPal;
	CSize sizeBitmap;
protected:
	ui_bitmap();
	~ui_bitmap();
private:
};

