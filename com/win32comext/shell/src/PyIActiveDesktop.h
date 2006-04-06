
class PyIActiveDesktop : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIActiveDesktop);
	static IActiveDesktop *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *ApplyChanges(PyObject *self, PyObject *args);
	static PyObject *GetWallpaper(PyObject *self, PyObject *args);
	static PyObject *SetWallpaper(PyObject *self, PyObject *args);
	static PyObject *GetWallpaperOptions(PyObject *self, PyObject *args);
	static PyObject *SetWallpaperOptions(PyObject *self, PyObject *args);
	static PyObject *GetPattern(PyObject *self, PyObject *args);
	static PyObject *SetPattern(PyObject *self, PyObject *args);
	static PyObject *GetDesktopItemOptions(PyObject *self, PyObject *args);
	static PyObject *SetDesktopItemOptions(PyObject *self, PyObject *args);
	static PyObject *AddDesktopItem(PyObject *self, PyObject *args);
	static PyObject *AddDesktopItemWithUI(PyObject *self, PyObject *args);
	static PyObject *ModifyDesktopItem(PyObject *self, PyObject *args);
	static PyObject *RemoveDesktopItem(PyObject *self, PyObject *args);
	static PyObject *GetDesktopItemCount(PyObject *self, PyObject *args);
	static PyObject *GetDesktopItem(PyObject *self, PyObject *args);
	static PyObject *GetDesktopItemByID(PyObject *self, PyObject *args);
	static PyObject *GenerateDesktopItemHtml(PyObject *self, PyObject *args);
	static PyObject *AddUrl(PyObject *self, PyObject *args);
	static PyObject *GetDesktopItemBySource(PyObject *self, PyObject *args);

protected:
	PyIActiveDesktop(IUnknown *pdisp);
	~PyIActiveDesktop();
};


class PyIActiveDesktopP : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIActiveDesktopP);
	static IActiveDesktopP *GetI(PyObject *self);
	static PyComTypeObject type;
	static PyObject *SetSafeMode(PyObject *self, PyObject *args);
	/* shlobj.h includes these methods for IActiveDesktopP, but they're not documented
		EnsureUpdateHTML()
		SetScheme(LPCWSTR pwszSchemeName, DWORD dwFlags)
		GetScheme(LPWSTR pwszSchemeName, DWORD *lpdwcchBuffer, DWORD dwFlags)
	*/
protected:
	PyIActiveDesktopP(IUnknown *pdisp);
	~PyIActiveDesktopP();
};

class PyIADesktopP2 : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIADesktopP2);
	static IADesktopP2 *GetI(PyObject *self);
	static PyComTypeObject type;
	static PyObject *UpdateAllDesktopSubscriptions(PyObject *self, PyObject *args);
	/* These are part of interface definition, but also don't show up on MSDN
		ReReadWallpaper()
		GetADObjectFlags(DWORD *lpdwFlags, DWORD dwMask)
		MakeDynamicChanges(IOleObject *pOleObj)
	*/
protected:
	PyIADesktopP2(IUnknown *pdisp);
	~PyIADesktopP2();

};
