# testExplorer -

import string
import sys
import os
import win32com.client.dynamic
import win32api
import glob
import pythoncom

def TestExplorer(iexplore):
	if not iexplore.Visible: iexplore.Visible = -1
	try:
		iexplore.Navigate(win32api.GetFullPathName('..\\readme.htm'))
	except pythoncom.com_error, details:
		print "Warning - could not open the test HTML file", details
#	for fname in glob.glob("..\\html\\*.html"):
#		print "Navigating to", fname
#		while iexplore.Busy:
#			win32api.Sleep(100)
#		iexplore.Navigate(win32api.GetFullPathName(fname))
	win32api.Sleep(4000)
	iexplore.Quit()

def TestAll():
	try:
		iexplore = win32com.client.dynamic.Dispatch("InternetExplorer.Application")
		TestExplorer(iexplore)

		win32api.Sleep(1000)
		from win32com.client import gencache
		gencache.EnsureModule("{EAB22AC0-30C1-11CF-A7EB-0000C05BAE0B}", 0, 1, 1)
		iexplore = win32com.client.Dispatch("InternetExplorer.Application")
		TestExplorer(iexplore)
		

	finally:
		iexplore = None

if __name__=='__main__':
	TestAll()

