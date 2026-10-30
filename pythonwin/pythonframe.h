// pythonframe.h : header file
//
#ifndef __PYTHONFRAME_H__
#define __PYTHONFRAME_H__

// With the new template mechanism, the Python frame classes
// become (nearly) 1 liners :-)

class CPythonFrameWnd : public CPythonWndFramework<CFrameWnd> {
    DECLARE_DYNAMIC(CPythonFrameWnd);

   public:
    // m_pFloatingFrameClass is protected so we can't access
    // it from the outside
    void SetFloatingFrameClass(CRuntimeClass *frameClass) { m_pFloatingFrameClass = frameClass; }
};

class CPythonMDIChildWnd : public CPythonFrameFramework<CMDIChildWnd> {
    DECLARE_DYNAMIC(CPythonMDIChildWnd);
};

class CPythonMDIFrameWnd : public CPythonFrameFramework<CMDIFrameWnd> {
    DECLARE_DYNAMIC(CPythonMDIFrameWnd);
};

/////////////////////////////////////////////////////////////////////////////
#endif  // __filename_h__
