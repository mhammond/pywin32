# A simple skeleton for an ISAPI extension.

class SimpleExtension:
    "Base class for a a simple ISAPI extension"
    def GetExtensionVersion(self, vi):
        vi.ExtensionDesc = self.__doc__

    def HttpExtensionProc(self, control_block):
        raise NotImplementedError, "sub-classes should override HttpExtensionProc"

    def TerminateExtension(self, status):
        pass

class SimpleFilter:
    "Base class for a a simple ISAPI filter"
    filter_flags = None
    def GetFilterVersion(self, fv):
        if self.filter_flags is None:
            raise RuntimeError, "You must specify the filter flags"
        fv.Flags = self.filter_flags
        fv.FilterDesc = self.__doc__

    def HttpFilterProc(self, fc):
        raise NotImplementedError, "sub-classes should override HttpExtensionProc"

    def TerminateFilter(self, status):
        pass
 