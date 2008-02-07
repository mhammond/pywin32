# A Demo of a service that takes advantage of the additional notifications
# available in later Windows versions.
import win32serviceutil, win32service
import win32event
import servicemanager

class EventDemoService(win32serviceutil.ServiceFramework):
    _svc_name_ = "PyServiceEventDemo"
    _svc_display_name_ = "Python Service Event Demo"
    _svc_description_ = "Demonstrates a Python service which takes advantage of the extra notifications"

    def __init__(self, args):
        win32serviceutil.ServiceFramework.__init__(self, args)
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None)

    # Override the base class so we can accept additional events.
    def GetAcceptedControls(self):
        # say we accept them all.
        rc = win32serviceutil.ServiceFramework.GetAcceptedControls(self)
        rc |= win32service.SERVICE_ACCEPT_PARAMCHANGE \
              | win32service.SERVICE_ACCEPT_NETBINDCHANGE \
              | win32service.SERVICE_ACCEPT_HARDWAREPROFILECHANGE \
              | win32service.SERVICE_ACCEPT_POWEREVENT \
              | win32service.SERVICE_ACCEPT_SESSIONCHANGE
        return rc

    # All extra events are sent via SvcOtherEx (SvcOther remains as a
    # function taking only the first args for backwards compat)
    def SvcOtherEx(self, control, event_type, data):
        # This is only showing a few of the extra events - see the MSDN
        # docs for "HandlerEx callback" for more info.
        # XXX can't do SERVICE_CONTROL_DEVICEEVENT until we wrap RegisterDeviceNotification.
        if control == win32service.SERVICE_CONTROL_HARDWAREPROFILECHANGE:
            msg = "A hardware profile changed: change type %d" % (event_type,)
        elif control == win32service.SERVICE_CONTROL_POWEREVENT:
            msg = "A power event: setting guid=%d, raw_data=%s" % data
        elif control == win32service.SERVICE_CONTROL_SESSIONCHANGE:
            # data is a single elt tuple, but this could potentially grow
            # in the future if the win32 struct does
            msg = "Session event: session ID=%d" % data[:1]
        else:
            msg = "Other event: code=%d, event_type=%d, data=%s" \
                  % (control, event_type, data)

        servicemanager.LogMsg(
                servicemanager.EVENTLOG_INFORMATION_TYPE,
                0xF000, #  generic message
                (msg, '')
                )
    
    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        win32event.SetEvent(self.hWaitStop)

    def SvcDoRun(self):
        # do nothing at all - just wait to be stopped
        win32event.WaitForSingleObject(self.hWaitStop, win32event.INFINITE)
        # Write a stop message.
        servicemanager.LogMsg(
                servicemanager.EVENTLOG_INFORMATION_TYPE,
                servicemanager.PYS_SERVICE_STOPPED,
                (self._svc_name_, '')
                )

if __name__=='__main__':
    win32serviceutil.HandleCommandLine(EventDemoService)
