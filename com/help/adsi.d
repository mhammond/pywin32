/*
@doc

@topic ADSI, Exchange, and Python|Python's adsi access works really well
with Exchange
(late or early binding since you can read microsoft's type library). To get
started, you will need to download adsi from microsoft:
http://www.microsoft.com/windows/server/Technical/directory/adsilinks.asp.
Microsoft has documentation for using languages other than python in the
sdk.

<nl>This documentation was generously provided by John F Nielson (nielsenjf@my-deja.com)

<nl>Before doing anything else you need to go through the next two steps:

<nl>
@flag Create the Global Providers object|adsiNameSpaces =
win32com.client.Dispatch('ADsNameSpaces')
@flag Now get the LDAP Provider object|ldapNameSpace =
adsiNameSpaces.getobject("","LDAP:") 

<nl>Now you have to decide how you want to access the exchange server. I
have chosen to authenticate in which case you need to use opendsobject

@flag The login and domain|logon_ex='cn=wilma, dc=bedrock'
@flag password|password='dino'
@flag now login|myDSObject =
ldapNameSpace.OpenDSObject(ex_path,logon_ex,password,0)
<nl>So what is this ex_path in the login?
<nl>It is the resource you are trying to access, for example:
@flag a specific
user|ex_path="LDAP://server/cn=fredflintsone,cn=Recipients,ou=rubble,o=bedro
ck" 
@flag a mailing
list|ex_path="LDAP://server/cn=bedrock,cn=Recipients,ou=rubble,o=bedrock"
@flag all of
Recipients|ex_path="LDAP://server/cn=Recipients,ou=rubble,o=bedrock"


@ex Accessing and Modifying a user:|
ex_path="LDAP://server/cn=fredflint,cn=Recipients,ou=rubble,o=bedrock"
myDSObject = ldapNameSpace.OpenDSObjec(ex_path,logon_ex,password,0)
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
myDSObject = ldapNameSpace.OpenDSObjec(ex_path,logon_ex,password,0) 
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


*/

