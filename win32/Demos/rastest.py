import win32ras

entryName = "Boyer"

def Callback( hras, msg, state, error, exterror):
	print "Callback called with ", hras, msg, state, error, exterror

def test():
	print "Current Connections:"
	for con in win32ras.EnumConnections():
		print con

	try:
		win32ras.EditPhonebookEntry(0,None,entryName)
	except win32ras.error, (rc, function, msg):
		print "Can not edit/find the RAS entry -", msg
		

#	theCallback = Callback
	theCallback = None
	hras, rc = win32ras.Dial(None, None, (entryName,),theCallback)
	try:
		print hras, rc
		if rc <> 0:
			print "Could not dial the RAS connection:", win32ras.GetErrorString(rc)
	finally:
		if hras > 0:
			win32ras.HangUp(hras)

test()
