/*
@doc

@topic ADSI, Exchange, and Python|Python's adsi access works really well
with Exchange
(late or early binding since you can read microsoft's type library). To get
started, you will need to download adsi from microsoft:
http://www.microsoft.com/windows/server/Technical/directory/adsilinks.asp.
Microsoft has documentation for using languages other than python in the
sdk.

@comm Before doing anything else you need to go through the next two steps:

@flagh Task|Description
@flag Create the Global Providers object|adsiNameSpaces =
win32com.client.Dispatch('ADsNameSpaces')
@flag Now get the LDAP Provider object|ldapNameSpace =
adsiNameSpaces.getobject("","LDAP:") 

@comm Now you have to decide how you want to access the exchange server. I
have chosen to authenticate, in which case you need to use OpenDSObject()

@flagh Task|Description
@flag Specify the login and domain|logon_ex='cn=wilma, dc=bedrock'
@flag Specify the password|password='dino'
@flag Login to the server|myDSObject =
ldapNameSpace.OpenDSObject(ex_path,logon_ex,password,0)

@comm Note -- the fourth argument to opendsobject has various options for how
to authenticate.  For example, if you use 1 instead of zero, it should
either use NTLM or Kerberos for authentication.  For more information,
check out: http://msdn.microsoft.com/library/psdk/adsi/if_core_3uic.htm

<nl>The ex_path in the above example specifies the resource you are trying to access.  For example:
@flag A specific user|
ex_path="LDAP://server/cn=fredflintsone,cn=Recipients,ou=rubble,o=bedrock" 
@flag A mailing list|
ex_path="LDAP://server/cn=bedrock,cn=Recipients,ou=rubble,o=bedrock"
@flag All Recipients|
ex_path="LDAP://server/cn=Recipients,ou=rubble,o=bedrock"


@ex Accessing and Modifying a user:|
ex_path="LDAP://server/cn=fredflint,cn=Recipients,ou=rubble,o=bedrock"
myDSObject = ldapNameSpace.OpenDSObject(ex_path,logon_ex,password,0)
myDSObject.Getinfo()
# To access a user's data try:
attribute = myDSObject.Get('Extension-Attribute-1')
print attribute
# To modify a user try:
myDSObject.Put('Extension-Attribute-1','barney was here')
myDSObject.Setinfo()
@comm Note -- To make any changes permanent setinfo is required.


@ex Adding new account to exchange|
# Adding a new account to exchange is simple except for one thing. 
# You need to associate an NT account with an exchange account. 
# To do so at this point requires some c++ to produce some hex SID 
# and trustee information that adsi can use. 
# At this point assume we have C++ magic 
#
# Note we are accessing Recipients directly now
ex_path="LDAP://server/cn=Recipients,ou=rubble,o=bedrock"
logon_ex='cn=wilma,dc=bedrock'
password='dino'
myDSObject = ldapNameSpace.OpenDSObject(ex_path,logon_ex,password,0) 
newobj = myDSObject.create("OrganizationalPerson", "cn=betty")
newobj.put('MailPreferenceOption', 0)
# etc . . . add whatever else you want. There are a few required fields. 
# Now the part to get exchange associated with NT
# The Magic is here
import win32pipe
assoc_nt=win32pipe.popen('getsid bedrock\\fredflint')
nt_security=win32pipe.popen('gettrustee bedrock\\fredflint')
newobj.put('NT-Security-Descriptor',assoc_nt)
newobj.put('NT-Security-Descriptor',nt_security)
newobj.SetInfo  

@ex Deleting an account from  exchange|
#Here we connect to Recipients and then
#delete a user
#This is an example with more generic code:
#data is a dictionary that contains info
#that may be dynamic like the domain,
#admin login, or exchange server
#notice I am using a try/except clause here
#to catch any exceptions
try:
  #ADSI here       
  # Create the Global Providers object
  logon_ex='cn='+data['NT_admin']+', dc='+data['NT_domain']+',cn=admin'
  ex_list_path="LDAP://"+data['EX_site_srv']+"/cn=Recipients,ou="\
	+data['ou']+",o="+data['o']
  adsi = win32com.client.Dispatch('ADsNameSpaces')
  #
  # Now get the LDAP Provider object 
  ldap = adsi.getobject("","LDAP:")
  dsobj = ldap.OpenDSObject(ex_list_path,logon_ex,data['NT_password'],0);
  dsobj.Getinfo()
  dsobj.Delete("OrganizationalPerson", "cn="+login)
  dsobj.Setinfo()
except:
  print 'Error deleting '+login, sys.exc_type , sys.exc_value

@ex Adding to a distribution list|
# I've added code here to make it a more generic example
# I used putex instead of put because it has more options
# The '3' value means append. The SDK has specific info on it
ex_list_path="LDAP://"+server+"/cn="+list+",cn=Recipients,ou="+ou+",o="+o
dsobj = ldap.OpenDSObject(ex_list_path,logon_ex,password,0);
dsobj.Getinfo()
list_member='cn='+user+',cn=Recipients,ou='+ou+',o='+o
append_list=[list_member]
dsobj.putEx(3,'Member',append_list);
dsobj.SetInfo()


<nl>Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com       


*/

