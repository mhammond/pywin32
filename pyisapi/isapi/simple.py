# A simple skeleton for an ISAPI extension.

class SimpleExtension:
    "Base class for a a simple ISAPI extension"
    def __init__(self):
        pass

    def GetExtensionVersion(self, vi):
        # nod to our reload capability - vi is None when we are reloaded.
        if vi is not None:
            vi.ExtensionDesc = self.__doc__

    def HttpExtensionProc(self, control_block):
        raise NotImplementedError, "sub-classes should override HttpExtensionProc"

    def TerminateExtension(self, status):
        pass

class SimpleFilter:
    "Base class for a a simple ISAPI filter"
    filter_flags = None
    def __init__(self):
        pass

    def GetFilterVersion(self, fv):
        if self.filter_flags is None:
            raise RuntimeError, "You must specify the filter flags"
        # nod to our reload capability - fv is None when we are reloaded.
        if fv is not None:
            fv.Flags = self.filter_flags
            fv.FilterDesc = self.__doc__

    def HttpFilterProc(self, fc):
        raise NotImplementedError, "sub-classes should override HttpExtensionProc"

    def TerminateFilter(self, status):
        pass
 