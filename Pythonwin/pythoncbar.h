// Templated control bar based classes.
// @doc

#pragma once
template <class T>
class CPythonControlBarFramework : public CPythonWndFramework<T>
{
public:
	// ctor hacks!
	CPythonControlBarFramework() : CPythonWndFramework<T>() {;}
	// End of ctor hacks!!!!
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz ) {
		// @pyvirtual int|PyCControlBar|CalcFixedLayout|Override to augment control-bar size calculations.
		// @comm The base implementation is not called if a handler exists.  This can be
		// done via <om CPythonControlBar.CalcFixedLayout>.
		// @xref <om CPythonControlBar.CalcFixedLayout>
		PyObject *ob;
		CSize result;
		CVirtualHelper helper( "CalcFixedLayout", this );
		if (helper.HaveHandler() && helper.call(bStretch, bHorz)) {
			helper.retval(ob);
			PyArg_ParseTuple(ob, "ii", &result.cx, &result.cy);
		} else
			result = T::CalcFixedLayout(bStretch, bHorz);
		return result;
	}
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode ) {
		// @pyvirtual int|PyCControlBar|CalcDynamicLayout|Override to augment control-bar size calculations.
		// @comm The base implementation is not called if a handler exists.  This can be
		// done via <om CPythonControlBar.CalcDynamicLayout>.
		// @xref <om CPythonControlBar.CalcDynamicLayout>
		PyObject *ob;
		CSize result;
		CVirtualHelper helper( "CalcDynamicLayout", this );
		if (helper.HaveHandler() && helper.call(nLength, dwMode)) {
			helper.retval(ob);
			PyArg_ParseTuple(ob, "ii", &result.cx, &result.cy);
		} else
			result = T::CalcDynamicLayout(nLength, dwMode);
		return result;
	}
	virtual void OnBarStyleChange(DWORD oldStyle, DWORD newStyle) {
		// @pyvirtual int|PyCControlBar|OnBarStyleChange|Override to augment control-bar size calculations.
		// @comm The base implementation is not called if a handler exists.  This can be
		// done via <om CPythonControlBar.OnBarStyleChange>.
		CVirtualHelper helper( "OnBarStyleChange", this );
		if (helper.HaveHandler() && helper.call(oldStyle, newStyle)) {
			;
		} else {
			T::OnBarStyleChange(oldStyle, newStyle);
		}
	}
};

class CPythonControlBar : public CPythonControlBarFramework<CControlBar>
{
	// @pyvirtual int|PyCControlBar|OnUpdateCmdUI|
	// @pyparm <o PyCFrameWnd>|frame||
	// @pyparm int|bDisableIsNoHandler||
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) {
		CVirtualHelper helper( "OnUpdateCmdUI", this );
		helper.call(pTarget, bDisableIfNoHndler);
	}
};
typedef CPythonControlBarFramework<CToolBar> CPythonToolBar;
typedef CPythonControlBarFramework<CStatusBar> CPythonStatusBar;
