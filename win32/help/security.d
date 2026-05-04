/*
@doc

@topic Windows NT Security -- Impersonation |Python's win32 access to help to simplify providing privileged access.

<nl>There may be times when you want to give specific access to
someone with NT. One mechanism to do this is with the win32 calls:
LogonUser and ImpersonateLoggedOnUser. LogonUser gives you a handle
which ImpersonateLoggedOnUser can then use to "become" the user. To do
this the thread calling, LogonUser, needs SE_TCB_NAME,
SE_CHANGE_NOTIFY_NAME, and SE_ASSIGNPRIMARYTOKEN_NAME privileges.  If
you plan to do this with something like IIS and cgi, be careful, the anonymous
account IIS uses is already impersonated from the system account. You
will need to use the RevertToSelf, api call to first terminate the
impersonation.  And, the system account, a local account, ultimately
limits you, regardless of who you log in as (COM/MTS can provide an
alternative security solution).

@ex The c++ api call for Logonasuser looks like:
|
BOOL LogonUser(
  LPTSTR lpszUsername,
  LPTSTR lpszDomain,
  LPTSTR lpszPassword,
  DWORD dwLogonType,
  DWORD dwLogonProvider,
  PHANDLE phToken
);


@ex The python documentation says this:
|PyHANDLE = LogonUser( userName, domain , password , logonType , logonProvider )


@ex The api call is very similar in both cases except in python the
handle is returned separately to the caller. The interesting options
in this case are logonType and logonProvider.  To give values for
these, you need to use the constants present in win32con (you can use
the browser in pythonwin->tools to list the constants in
win32con). Unless you have unusual server requirements, for logonType,
win32con.LOGON32_LOGON_INTERACTIVE should be fine. With regards to
logonProvider, generally use win32con.LOGON32_PROVIDER_DEFAULT -- it's
for specifying the type of logon NT 3.5, 4.0, win2000. Generally,
default is fine.

ImpersonateLoggedOnUser is extremely simple and you'll see it's usage in the
examples.

<nl>Now for some code|

#A raw example looks like this:
handle=win32security.LogonUser('barney','bedrock','bambam'\
	,win32con.LOGON32_LOGON_INTERACTIVE,win32con.LOGON32_PROVIDER_DEFAULT)
win32security.ImpersonateLoggedOnUser(handle)

# do stuff here
print(win32api.GetUserName())  # show you're someone else

win32security.RevertToSelf() #terminates impersonation
handle.Close()

#The impersonate code can be encapsulated in a class, which then makes it even more
#trivial to use

import win32security
import win32con
import win32api

class Impersonate:
    def __init__(self,login,password):
        self.domain='bedrock'
        self.login=login
        self.password=password
    def logon(self):
        self.handle=win32security.LogonUser(self.login,self.domain,self.password,\
        win32con.LOGON32_LOGON_INTERACTIVE,win32con.LOGON32_PROVIDER_DEFAULT)
        win32security.ImpersonateLoggedOnUser(self.handle)
    def logoff(self):
        win32security.RevertToSelf() #terminates impersonation
        self.handle.Close() #guarantees cleanup


a=Impersonate('barney','bambam')

try:
    a.logon() #become the user
    #do whatever here
    print(win32api.GetUserName())  # show you're someone else
    a.logoff() #return to normal
except:
    print(sys.exc_type, sys.exc_value)

@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com


*/
