import win32wnet
import sys
from winnetwk import *

g = win32wnet.NETRESOURCE()

g.lpProvider = 'ProviderName'
g.lpLocalName = 'LocalName'
g.lpRemoteName ='RemoteName'
g.lpComment = 'Commentlineis here'

print 'start script comment'
print 'by attribute'

print g.lpLocalName
print g.lpRemoteName
print g.lpProvider
print g.lpComment

print ''

try:
	retval = win32wnet.WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,None)
except:
	print 'exception taken on OpenEnum'
	print g
	raise

print 'retvalue valid'
print retval

try:
	root = win32wnet.WNetEnumResource(retval, 0xffffffff)
	win32wnet.WNetCloseEnum(retval)
	
except:
	print 'exception taken on EnumResource'
	raise
	

h = win32wnet.NETRESOURCE()
h.lpRemoteName ='\\\\171.69.164.176'
#h.setstring('lpProvider', 'Microsoft Windows Network')
h.dwUsage = RESOURCEUSAGE_CONTAINER
h.dwScope = RESOURCE_GLOBALNET
h.dwType = RESOURCETYPE_ANY
h.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC





try:
	k = win32wnet.WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY,0,h)
	print 'back from openenum'
	sub1 = win32wnet.WNetEnumResource(k, 2)
	sub2 = win32wnet.WNetEnumResource(k, 2)
	win32wnet.WNetCloseEnum(k)
	print sub1
	print sub2
	
	
except:
	print 'exception taken on sub 1 enumeration'
	raise


print 'End of Program'	