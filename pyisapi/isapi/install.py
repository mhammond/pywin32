# Installation utilities for Python ISAPI filters and extensions

# this code adapted from "Tomcat JK2 ISAPI redirector", part of Apache
# Created July 2004, Mark Hammond.
import sys, os, imp, shutil, stat
from win32com.client import GetObject, Dispatch
from win32com.client.gencache import EnsureModule, EnsureDispatch
import pythoncom
import winerror

_APP_INPROC  = 0;
_APP_OUTPROC = 1;
_APP_POOLED  = 2
_IIS_OBJECT  = "IIS://LocalHost/W3SVC"
_IIS_SERVER  = "IIsWebServer"
_IIS_WEBDIR  = "IIsWebDirectory"
_IIS_WEBVIRTUALDIR  = "IIsWebVirtualDir"
_IIS_FILTERS = "IIsFilters"
_IIS_FILTER  = "IIsFilter"

_DEFAULT_SERVER_NAME = "Default Web Site"
_DEFAULT_HEADERS     = "X-Powered-By: Python"
_DEFAULT_PROTECTION  = _APP_POOLED

# Default is for 'execute' only access - ie, only the extension
# can be used.  This can be overridden via your install script.
_DEFAULT_ACCESS_EXECUTE = True
_DEFAULT_ACCESS_READ = False
_DEFAULT_ACCESS_WRITE = False
_DEFAULT_ACCESS_SCRIPT = False
_DEFAULT_CONTENT_INDEXED = False
_DEFAULT_ENABLE_DIR_BROWSING = False
_DEFAULT_ENABLE_DEFAULT_DOC = False

is_debug_build = False
for ext, _, _ in imp.get_suffixes():
    if ext == "_d.pyd":
        is_debug_build = True
        break

this_dir = os.path.abspath(os.path.dirname(__file__))

class FilterParameters:
    Name = None
    Description = None
    Path = None
    def __init__(self, **kw):
        self.__dict__.update(kw)

class VirtualDirParameters:
    Name = None # Must be provided.
    Description = None # defaults to Name
    AppProtection = _DEFAULT_PROTECTION
    Headers       = _DEFAULT_HEADERS;
    Path          = None # defaults to WWW root.
    AccessExecute  = _DEFAULT_ACCESS_EXECUTE
    AccessRead     = _DEFAULT_ACCESS_READ
    AccessWrite    = _DEFAULT_ACCESS_WRITE
    AccessScript   = _DEFAULT_ACCESS_SCRIPT
    ContentIndexed = _DEFAULT_CONTENT_INDEXED
    EnableDirBrowsing = _DEFAULT_ENABLE_DIR_BROWSING
    EnableDefaultDoc  = _DEFAULT_ENABLE_DEFAULT_DOC
    ScriptMaps       = []
    ScriptMapUpdate = "end" # can be 'start', 'end', 'replace'
    def __init__(self, **kw):
        self.__dict__.update(kw)

class ScriptMapParams:
    Extension = None
    Module = None
    Flags = 5
    Verbs = ""
    def __init__(self, **kw):
        self.__dict__.update(kw)
    
class ISAPIParameters:
    ServerName     = _DEFAULT_SERVER_NAME;
    # Description = None
    Filters = []
    VirtualDirs = []

verbose = 1 # The level - 0 is quiet.
def log(level, what):
    if verbose >= level:
        print what

# Convert an ADSI COM exception to the Win32 error code embedded in it.
def _GetWin32ErrorCode(com_exc):
    hr, msg, exc, narg = com_exc
    # If we have more details in the 'exc' struct, use it.
    if exc:
        hr = exc[-1]
    if winerror.HRESULT_FACILITY(hr) != winerror.FACILITY_WIN32:
        raise
    return winerror.SCODE_CODE(hr)

class ItemNotFound(Exception): pass
class ConfigurationError(Exception): pass

def FindWebServiceObject(class_name = None, obj_name = None):
    #webService = adsi.ADsGetObject(_IIS_OBJECT, adsi.IID_IADsContainer)
    webService = GetObject(_IIS_OBJECT)
    if class_name is None and obj_name is None:
        return webService
    raise ItemNotFound, "WebService %s.%s" % (class_name, obj_name)

def FindWebServer(server_desc):
    webService = GetObject(_IIS_OBJECT)
    for ob in webService:
        if ob.Class == _IIS_SERVER and ob.ServerComment == server_desc:
            return ob
    raise ItemNotFound, "WebServer %s" % (server_desc,)
 
def FindADSIObject(adsiObject, clsName, objName):
    # IIS6 seems to have different case for the same objects compared to IIS5.
    # So do all ADSI searches case-insensitive.
    clsName = clsName.lower()
    objName = objName.lower()
    for child in adsiObject:
#        print "Checking", child.Class, child.Name
        if child.Class.lower() == clsName and child.Name.lower() == objName:
            return child
    raise ItemNotFound, "ADSI object %s.%s" % (clsName, objName)

def CreateVirtualDir(webRootDir, params):
    if not params.Name:
        raise ConfigurationError, "No Name param"
    try:
        newDir = webRootDir.Create(_IIS_WEBVIRTUALDIR, params.Name)
    except pythoncom.com_error, details:
        rc = _GetWin32ErrorCode(details)
        if rc != winerror.ERROR_ALREADY_EXISTS:
            raise
        # I don't understand this "WEBDIR" vs "WEBVIRTUALDIR"
        try:
            newDir = FindADSIObject(webRootDir, _IIS_WEBVIRTUALDIR, params.Name)
        except ItemNotFound:
            newDir = FindADSIObject(webRootDir, _IIS_WEBDIR, params.Name)
        log(2, "Updating existing directory '%s'..." % (params.Name,));
    else:
        log(2, "Creating new directory '%s'..." % (params.Name,))
        friendly = params.Description or params.Name
        newDir.AppFriendlyName = friendly
        path = params.Path or webRootDir.Path
        newDir.Path = path
        newDir.AppCreate2(params.AppProtection)
        newDir.HttpCustomHeaders = params.Headers
        
    log(2, "Setting directory options...");
    newDir.AccessExecute  = params.AccessExecute
    newDir.AccessRead     = params.AccessRead
    newDir.AccessWrite    = params.AccessWrite
    newDir.AccessScript   = params.AccessScript
    newDir.ContentIndexed = params.ContentIndexed
    newDir.EnableDirBrowsing = params.EnableDirBrowsing
    newDir.EnableDefaultDoc  = params.EnableDefaultDoc
    newDir.SetInfo()
    smp_items = []
    for smp in params.ScriptMaps:
        item = "%s,%s,%s" % (smp.Extension, smp.Module, smp.Flags)
        # IIS gets upset if there is a trailing verb comma, but no verbs
        if smp.Verbs:
            item += "," + smp.Verbs
        smp_items.append(item)
    if params.ScriptMapUpdate == "replace":
        newDir.ScriptMaps = smp_items
    elif params.ScriptMapUpdate == "end":
        for item in smp_items:
            if item not in newDir.ScriptMaps:
                newDir.ScriptMaps = newDir.ScriptMaps + (item,)
    elif params.ScriptMapUpdate == "start":
        for item in smp_items:
            if item not in newDir.ScriptMaps:
                newDir.ScriptMaps = (item,) + newDir.ScriptMaps
    else:
        raise ConfigurationError, \
              "Unknown ScriptMapUpdate option '%s'" % (params.ScriptMapUpdate,)
    newDir.SetInfo()
    log(1, "Configured Virtual Directory: %s" % (params.Name,))
    return newDir

def CreateISAPIFilter(webServer, filterParams):
    filters = FindADSIObject(webServer, _IIS_FILTERS, "Filters")
    try:
        newFilter = filters.Create(_IIS_FILTER, filterParams.Name)
        log(2, "Created new ISAPI filter...")
    except pythoncom.com_error, (hr, msg, exc, narg):
        if exc is None or exc[-1]!=-2147024713:
            raise
        log(2, "Updating existing filter '%s'..." % (filterParams.Name,))
        newFilter = FindADSIObject(filters, _IIS_FILTER, filterParams.Name)
    assert os.path.isfile(filterParams.Path)
    newFilter.FilterPath  = filterParams.Path
    newFilter.FilterDescription = filterParams.Description
    newFilter.SetInfo()
    load_order = [b.strip() for b in filters.FilterLoadOrder.split(",")]
    if filterParams.Name not in load_order:
        load_order.append(filterParams.Name)
        filters.FilterLoadOrder = ",".join(load_order)
        filters.SetInfo()
    log (1, "Configured Filter: %s" % (filterParams.Name,))
    return newFilter

def DeleteISAPIFilter(webServer, filterParams):
    filters = FindADSIObject(webServer, _IIS_FILTERS, "Filters")
    try:
        newFilter = filters.Delete(_IIS_FILTER, filterParams.Name)
        log(2, "Deleted ISAPI filter '%s'" % (filterParams.Name,))
    except pythoncom.com_error, details:
        rc = _GetWin32ErrorCode(details)
        if rc != winerror.ERROR_PATH_NOT_FOUND:
            raise
        log(2, "ISAPI filter '%s' did not exist." % (filterParams.Name,))
    if filterParams.Path:
        load_order = [b.strip() for b in filters.FilterLoadOrder.split(",")]
        if filterParams.Path in load_order:
            load_order.remove(filterParams.Path)
            filters.FilterLoadOrder = ",".join(load_order)
            filters.SetInfo()
    log (1, "Deleted Filter: %s" % (filterParams.Name,))

def CheckLoaderModule(dll_name):
    suffix = ""
    if is_debug_build: suffix = "_d"
    template = os.path.join(this_dir,
                            "PyISAPI_loader" + suffix + ".dll")
    if not os.path.isfile(template):
        raise isapi.install.ConfigurationError, \
              "Template loader '%s' does not exist"
    # We can't do a simple "is newer" check, as the DLL is specific to the 
    # Python version.  So we check the date-time and size are identical,
    # and skip the copy in that case.
    src_stat = os.stat(template)
    try:
        dest_stat = os.stat(dll_name)
    except os.error:
        same = 0
    else:
        same = src_stat[stat.ST_SIZE]==dest_stat[stat.ST_SIZE] and \
               src_stat[stat.ST_MTIME]==dest_stat[stat.ST_MTIME]
    if not same:
        log(2, "Updating %s->%s" % (template, dll_name))
        shutil.copyfile(template, dll_name)
        shutil.copystat(template, dll_name)
    else:
        log(2, "%s is up to date." % (dll_name,))

def Install(params):
    server_name = _DEFAULT_SERVER_NAME
    web_service = FindWebServiceObject()
    server = FindWebServer(server_name)
    root = FindADSIObject(server, _IIS_WEBVIRTUALDIR, "Root")
    for vd in params.VirtualDirs:
        CreateVirtualDir(root, vd)
    for filter_def in params.Filters:
        f = CreateISAPIFilter(server, filter_def)

def Uninstall(params):    
    server_name = _DEFAULT_SERVER_NAME
    web_service = FindWebServiceObject()
    server = FindWebServer(server_name)
    root = FindADSIObject(server, _IIS_WEBVIRTUALDIR, "Root")
    for vd in params.VirtualDirs:
        try:
            newDir = root.Delete(_IIS_WEBVIRTUALDIR, vd.Name)
        except pythoncom.com_error, details:
            rc = _GetWin32ErrorCode(details)
            if rc != winerror.ERROR_PATH_NOT_FOUND:
                raise
        log (1, "Deleted Virtual Directory: %s" % (vd.Name,))


    for filter_def in params.Filters:
        DeleteISAPIFilter(server, filter_def)

# Patch up any missing module names in the params, replacing them with
# the DLL name that hosts this extension/filter.
def _PatchParamsModule(params, dll_name, file_must_exist = True):
    if file_must_exist:
        if not os.path.isfile(dll_name):
            raise ConfigurationError, "%s does not exist" % (dll_name,)
    
    # Patch up all references to the DLL.
    for f in params.Filters:
        if f.Path is None: f.Path = dll_name
    for d in params.VirtualDirs:
        for sm in d.ScriptMaps:
            if sm.Module is None: sm.Module = dll_name

def GetLoaderModuleName(mod_name):
    # find the name of the DLL hosting us.
    # By default, this is "_{module_base_name}.dll"
    if hasattr(sys, "frozen"):
        # What to do?  The .dll knows its name, but this is likely to be
        # executed via a .exe, which does not know.
        # For now, we strip everything past the last underscore in the
        # executable name - this will work so long as you don't override
        # the dest_base for the DLL itself.
        base, ext = os.path.splitext(os.path.abspath(sys.argv[0]))
        path, base = os.path.split(base)
        try:
            base = base[:base.rindex("_")]
        except ValueError:
            pass
        dll_name = os.path.join(path, base + ".dll")
    else:
        base, ext = os.path.splitext(mod_name)
        path, base = os.path.split(base)
        dll_name = os.path.abspath(os.path.join(path, "_" + base + ".dll"))
    # Check we actually have it.
    if not hasattr(sys, "frozen"):
        CheckLoaderModule(dll_name)
    return dll_name

def InstallModule(conf_module_name, params):
    if not hasattr(sys, "frozen"):
        conf_module_name = os.path.abspath(conf_module_name)
        if not os.path.isfile(conf_module_name):
            raise ConfigurationError, "%s does not exist" % (conf_module_name,)

    loader_dll = GetLoaderModuleName(conf_module_name)
    _PatchParamsModule(params, loader_dll)
    Install(params)

def UninstallModule(conf_module_name, params):
    loader_dll = GetLoaderModuleName(conf_module_name)
    _PatchParamsModule(params, loader_dll, False)
    Uninstall(params)

# Later we will probably need arguments that allow us to change the
# name of the default server, etc - ie, at the moment, we only work
# when the WWW server is named _DEFAULT_SERVER_NAME ("Default Web Site")
def HandleCommandLine(params, argv=None, conf_module_name = None):
    global verbose
    from optparse import OptionParser

    argv = argv or sys.argv
    conf_module_name = conf_module_name or sys.argv[0]
    
    parser = OptionParser(usage="%prog [options] [install|remove]")
    parser.add_option("-q", "--quiet",
                      action="store_false", dest="verbose", default=True,
                      help="don't print status messages to stdout")
    parser.add_option("-v", "--verbosity",
                      dest="verbose", default=1,
                      help="set the verbosity of status messages")

    (options, args) = parser.parse_args(argv[1:])
    verbose = options.verbose
    if not args:
        args = ["install"]
    for arg in args:
        if arg == "install":
            InstallModule(conf_module_name, params)
            log(1, "Installation complete.")
        elif arg in ["remove", "uninstall"]:
            UninstallModule(conf_module_name, params)
            log(1, "Uninstallation complete.")
        else:
            parser.error("Invalid arg '%s'" % (arg,))
