/*
@doc

@topic Network Management Functions and Python|Python's win32 access to
Network Management functions are very clean and simple. A good example of
this is with with NetWkstaUserEnum.

<nl>If you want to get the logins currently present on a workstation you
need to use NetWkstaUserEnum api call.  

@ex In c++ the call looks like this: |

NET_API_STATUS NetWkstaUserEnum(
  LPWSTR servername,    
  DWORD level,          
  LPBYTE *bufptr,       
  DWORD prefmaxlen,     
  LPDWORD entriesread,  
  LPDWORD totalentries, 
  LPDWORD resumehandle  
);

@ex The python documentation says this: | ([dict, ...], total, resumeHandle)
= NetWkstaUserEnum( server, level , resumeHandle , prefLen )

@ex (see <om win32net.NetWkstaUserEnum>).  Notice that the api call is split in 2 pieces with python, instead of
being all combined in one call.  So, how do they relate to each other. In
c++, you check the NET_API_STATUS to determine the result of the call and if
you have more data. You also have to extract from structures like
LPWKSTA_USER_INFO_0 to get the user data keeping track of entriesread and
the totalentries. Rather tedious. Thankfully, this is easy in python. With
python a dictionary is returned which trivial to parse and if resumeHandel
is true you make the win32 call again.

<nl>Now for some code|
 
import win32net

def getusers(srv):
  res=0  #constant mentioned in python win32 docs
  pref=4096 #constant mentioned in python win32 docs
  level=0 #setting it to 1 will provide more detailed info
  try:
    (dict,total,res2)=win32net.NetWkstaUserEnum(srv,level,res,pref)
    res=res2
    print dict  #print out entire dictionary
    while res: #loop until res2
	try:
        (dict,total,res2)=win32net.NetWkstaUserEnum(srv,level,res,pref)
	  print dict #print out entire dictionary
        res=res2
	  except win32net.error:
	    return "error"
   except win32net.error:
     return "error"

Have a great time with programming with python!
John Nielsen   nielsenjf@my-deja.com       




@topic Using the win32net module|The <o win32net> module offers most of
the Windows NT Networking API from Python. This topic provides an
overview of using this module, and was graciously contrbuted by John
Nielsen (nielsenjf@my-deja.com)

<nl>To demonstrate the use of this module, we will work with Windows
shares. The exact same pattern is used for all features in the win32net
module, be it users, servers, shares, etc.

<nl>For example, if we want to add a new share, the help documentation
says is that I need a "dictionary holding the share data, in the format
of <o PySHARE_INFO_*>".

<nl>Selecting that link will show a number of
different PySHARE_INFO structures; lets assume we want to use the <o
PySHARE_INFO_2> structure.
                     
<nl>So do you create this PySHARE_INFO_2 Object? It is really quite simple:
<nl>                     
<nl>In c++, for example, the SHARE_INFO_2 structure looks like:
<nl>                    typedef struct _SHARE_INFO_2 {
<nl>                        LPWSTR    shi2_netname;
<nl>                        DWORD    shi2_type;
<nl>                        LPWSTR    shi2_remark;
<nl>                        DWORD    shi2_permissions;
<nl>                        DWORD    shi2_max_uses;
<nl>                        DWORD    shi2_current_uses;
<nl>                        LPWSTR    shi2_path;
<nl>                        LPWSTR    shi2_passwd;
<nl>                    } SHARE_INFO_2, *PSHARE_INFO_2, *LPSHARE_INFO_2;
<nl>                     
<nl>What does that mean in python?

<nl>You simply make a dictionary with the entries matching the
structure above, except you remove the "shi2_" prefix.  In general, not
all of the structure items will be required; you will need to consult the
Win32 SDK for more information on these structures, and exactly what elements
are required for what operation.

<nl>the final trick is knowing where to get the constant values for some of these
items.  For example, the "shi2_type" element is defined as an integer - what values
are valid?  If we consult the Win32 documentation, we will find valid values include 
STYPE_DISKTREE, STYPE_PRINTQ and a number of others.  In general, you can find these
constants in the win32netcon module.  The Pythonwin object browser can show you what's 
available in this module.

@ex Given this knowledge, we could then write the following Python code to
add a new share|

                    import win32net
                    import win32netcon
                    shinfo={}
                    shinfo['netname']='python test'
                    shinfo['type']=win32netcon.STYPE_DISKTREE
                    shinfo['remark']='bedrock_rubbel'
                    shinfo['permissions']=0
                    shinfo['max_uses']=-1
                    shinfo['current_uses']=0
                    shinfo['path']='c:\rubbel_share'
                    shinfo['passwd']=''
                    server='betty_server'
                   
                    try:
                      win32net.NetShareAdd(server,2,shinfo)
                      return "success"
                    except win32net.error:
                      return "error"


@topic User Management Functions and Python|Python's win32 access to
User Management functions are very clean and simple. A good example of
this is with with NetUserGetInfo and NetUserSetInfo.

<nl>If you want to get or set attributes about a user in NT you need
to use NetUserGetInfo and NetUserSetInfo, respectively.

@ex In c++ the setinfo call looks like this: |

NET_API_STATUS NetUserSetInfo(
  LPCWSTR servername, 
  LPCWSTR username,   
  DWORD level,       
  LPBYTE buf,        
  LPDWORD parm_err   
);

@ex It's reasonably self explanatory except for level. It turns out
there are many levels. One of the most useful one is USER_INFO_3 this
structure lets you change just about anything you want for a
user.Also, not all levels are available for both NetUserSetInfo and
NetUserGetInfo. NetUserGetInfo has about 7 available to it.  (btw,
http://msdn.microsoft.com would have more information about this).
I'll show an example with NetUserSetInfo using USER_INFO_1008 that has
no corresponding NetUserGetInfo.
<nl>
<nl>
The python call looks like this: |NetUserSetInfo( server , username ,
level , data )

@ex For it the most interesting parts are server, level, and data.  
<nl>
Server is only interesting because the server name has to be
prepended with \\. It is not obvious from the python api
description, but very important.  Luckily, as we'll see later, some
functions in python return the server already in the correct format.

<nl> Level is similar to the C++ level except that you simply use a
number. In our case we would use 3.

<nl> For data the documentation says you need a dictionary holding the
user data in the format of PyUSER_INFO_*. If you check out that object
you will see many available levels and find the level 3 we are
interested in. It is simply a very long python dictionary with all sorts of user
attributes. Of these attributes, one can be confusing: flags. If you want to change flags
for a user, you need to have all of the appropriate flags bitwise OR'ed together. The 
second example below will better explain what is happening.
<nl>
<nl>
Now for some code:
<nl>
In the example below, you will notice a couple of things. First,
win32net.NetGetDCName returns the primary domain controller already in
the correct format with double backslashes prepended. Also, python
exception handling is a very powerful asset. The try block explicitly
catches and win32net errors from your system calls. And, we can
extract what happened with sys.exc_type and sys.exc_value.
|
import sys 
import win32net 
import win32netcon

domain="bedrock"
login="slate"

try:
    #get the server for the domain -- it has to be a primary dc
    server=str(win32net.NetGetDCName("",domain)) 
    #info returns a dictionary of information
    info = win32net.NetUserGetInfo(server, login, 3)
    print info['full_name']
    info['full_name']="Mr. Slate"
    win32net.NetUserSetInfo(server,login,3,info)
    info = win32net.NetUserGetInfo(server, login, 3)
    print info['full_name']

except win32net.error:
    print 'Failed to get info for login'
    print sys.exc_type , sys.exc_value


@ex A trickier 2nd example|
 
The example below deals with the 1008 structure and flags. The python
documentation says the structure is a dictionary holding the
information in a Win32 USER_INFO_1008 structure.  with properties: int
flags. What that means is you have something like
dict={'flags':int_value}.  In our case, each flag represents a bit
that is turned off or on. The constants in win32netcon come in very
handy here. Make sure you have all of the bits included even if you
are only going to change one of the values. For example, if you want
to tell NT to not expire a password for a user, you need to use:
win32netcon.UF_DONT_EXPIRE_PASSWD. However, if you only use that, then
all the remaining bits aren't set. You have 2 options, either bitwise
OR the appropriate options together, or  do a bitwise OR of the flag with
the current user flags To remove a flag use the bitwise compliment ~.
This will be more clear in the example below.


Here is some code that turns on the
UF_DONT_EXPIRE_PASSWD bit for a user. 
|
import sys 
import win32net 
import win32netcon

domain="bedrock"
login="slate"

try:
    #get the server for the domain -- it has to be a primary dc
    server=str(win32net.NetGetDCName("",domain))

    #Commented out here is a typical flag setting 
    #flag=win32netcon.UF_NORMAL_ACCOUNT|win32netcon.UF_SCRIPT|win32netcon.UF_DONT_EXPIRE_PASSWD
    #however, if you want to preserve any non-standard flags then do the following
    d=win32net.NetUserGetInfo(server,login,3) #get the current flags, note, I didn't use 1008 here!
    flag=d['flags']| win32netcon.UF_DONT_EXPIRE_PASSWD  #this adds the flag
    # flag=d['flags']& ~win32netcon.UF_DONT_EXPIRE_PASSWD  #this removes the flag

    flag_dict={'flags':flag}
    win32net.NetUserSetInfo(server,login,1008,flag_dict)
except win32net.error:
    print 'Failed to get info for login'
    print sys.exc_type , sys.exc_value





@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com       


*/

