import sys
import win32api
import win32net
import win32netcon
import win32security
import getopt
import traceback

server = None # Run on local machine.

def CreateUser():
	"Creates a new test user, then deletes the user"
	testName = "PyNetTestUser"
	try:
		win32net.NetUserDel(server, testName)
		print "Warning - deleted user before creating it!"
	except win32net.error:
		pass

	d = {}
	d['name'] = testName
	d['password'] = 'deleteme'
	d['priv'] = win32netcon.USER_PRIV_USER
	d['comment'] = "Delete me - created by Python test code"
	d['flags'] = win32netcon.UF_NORMAL_ACCOUNT | win32netcon.UF_SCRIPT
	win32net.NetUserAdd(server, 1, d)
	try:
		try:
			win32net.NetUserChangePassword(server, testName, "wrong", "new")
			print "ERROR: NetUserChangePassword worked with a wrong password!"
		except win32net.error:
			pass
		win32net.NetUserChangePassword(server, testName, "deleteme", "new")
	finally:
		win32net.NetUserDel(server, testName)
	print "Created a user, changed their password, and deleted them!"
	
def UserEnum():
	"Enumerates all the local servers"
	resume = 0
	while 1:
		data, total, resume = win32net.NetUserEnum(server, 3, win32netcon.FILTER_NORMAL_ACCOUNT, resume)
		print "Call to NetUserEnum obtained %d entries of %d total" % (len(data), total)
		for user in data:
			print "Found user %s" % user['name']
		if not resume:
			break

def GroupEnum():
	"Enumerates all the domain groups"
	resume = 0
	while 1:
		data, total, resume = win32net.NetGroupEnum(server, 1, resume)
#		print "Call to NetGroupEnum obtained %d entries of %d total" % (len(data), total)
		for group in data:
			print "Found group %(name)s:%(comment)s " % group
			memberresume = 0
			while 1:
				memberdata, total, memberresume = win32net.NetGroupGetUsers(server, group['name'], 0, resume)
				for member in memberdata:
					print " Member %(name)s" % member
				if memberresume==0:
					break
		if not resume:
			break
			
def LocalGroupEnum():
	"Enumerates all the local groups"
	resume = 0
	while 1:
		data, total, resume = win32net.NetLocalGroupEnum(server, 1, resume)
		for group in data:
			print "Found group %(name)s:%(comment)s " % group
			memberresume = 0
			while 1:
				memberdata, total, memberresume = win32net.NetLocalGroupGetMembers(server, group['name'], 2, resume)
				for member in memberdata:
					# Just for the sake of it, we convert the SID to a username
					username, domain, type = win32security.LookupAccountSid(server, member['sid'])
					print " Member %s (%s)" % (username, member['domainandname'])
				if memberresume==0:
					break
		if not resume:
			break

def ServerEnum():
	"Enumerates all servers on the network"
	resume = 0
	while 1:
		data, total, resume = win32net.NetServerEnum(server, 100, win32netcon.SV_TYPE_ALL, None, resume)
		for s in data:
			print "Found server %s" % s['name']
			# Now loop over the shares.
			shareresume=0
			while 1:
				sharedata, total, shareresume = win32net.NetShareEnum(server, 2, shareresume)
				for share in sharedata:
					print " %(netname)s (%(path)s):%(remark)s - in use by %(current_uses)d users" % share
				if not shareresume:
					break
		if not resume:
			break
	
def GetInfo(userName=None):
	"Dumps level 3 information about the current user"
	if userName is None: userName=win32api.GetUserName()
	print "Dumping level 3 information about user", userName
	info = win32net.NetUserGetInfo(server, userName, 3)
	for key, val in info.items():
		print key,"=",str(val)

def SetInfo(userName=None):
	"Attempts to change the current users comment, then set it back"
	if userName is None: userName=win32api.GetUserName()
	oldData = win32net.NetUserGetInfo(server, userName, 3)
	try:
		d = oldData.copy()
		d["usr_comment"] = "Test comment"
		win32net.NetUserSetInfo(server, userName, 3, d)
		new = win32net.NetUserGetInfo(server, userName, 3)['usr_comment']
		if  str(new) != "Test comment":
			raise RuntimeError, "Could not read the same comment back - got %s" % new
	finally:
		win32net.NetUserSetInfo(server, userName, 3, oldData)

def usage(tests):
	import os
	print "Usage: %s [-s server ] Test [Test ...]" % os.path.basename(sys.argv[0])
	print "where Test is 'all' or one of:"
	for t in tests:
		print t.__name__,":", t.__doc__
	sys.exit(1)

def main():
	tests = []
	for ob in globals().values():
		if type(ob)==type(main) and ob.__doc__:
			tests.append(ob)
	opts, args = getopt.getopt(sys.argv[1:], "s:")
	for opt, val in opts:
		if opt=="-s":
			global server
			server = val

	if len(args)==0:
		usage(tests)			
	if args[0]=="all":
		dotests = tests
	else:
		dotests = []
		for arg in args:
			for t in tests:
				if t.__name__==arg:
					dotests.append(t)
					break
			else:
				print "Test '%s' unknown - skipping" % arg
	if not len(dotests):
		print "Nothing to do!"
		usage(tests)
	for test in dotests:
		try:
			test()
		except:
			print "Test %s failed" % test.__name__
			traceback.print_exc()
		
if __name__=='__main__':
	main()
