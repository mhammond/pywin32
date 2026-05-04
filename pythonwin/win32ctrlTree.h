/* win32ctrltree : header

    Tree control object.

    Created May 1996, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
///////////////////////////////////////////////////////////////////////
// Control objects.
//
// PyCTreeCtrl
//
class PyCTreeCtrl : public PyCWnd {
   public:
    static ui_type_CObject type;
    MAKE_PY_CTOR(PyCTreeCtrl)

   protected:
    PyCTreeCtrl();
    virtual ~PyCTreeCtrl();
};
