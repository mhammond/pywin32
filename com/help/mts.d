/*
@doc

@topic MTS and Python for NT |How to use python w/MTS on NT.

<nl>MTS represents microsoft's attempt to simplify computing for a
distributed system. Where COM is all about identity, MTS manages
state.  It introduces new ideas and you'll find corresponding
terminology like: object and state context, activity, causality. It
also represents a push towards declarative programming -- where you
declare how a class can exist and the operating system provides the
services to handle that (instead of you having to deal with
it). Windows 2000 offers a further set of refinements, though what's
documented here will hopefully be backward compatible.

<nl>In this example, we'll specifically deal with a simple case that
can show the transaction monitor in use. We have an active server page
(using python of course) making calls to a python MTS component. The
component then makes simple use of ObjectContext interfaces and
decides whether or not the transaction is a success or failure.

<nl>To insert a python COM object into MTS, you want to register your
COM object,setup an MTS package, and then add the python component to
the package as a component that is already registered. After it is
added, do not then register the component using python, as it will
overwrite the changes MTS did. To create a package, open up the
Transaction Server Explorer from the Option Pack, go to My Computer ->
Packages Installed, right click and select new and create an empty
package. From there you can call the package whatever you want and let
it run as whomever. Then open up the package you created, right click
on Components, select New Component, and finally select Import
components that are already registered, from which you highlight your
python component it provides in the list. You can also right click on
Roles in your package to create a role you can put users or groups
in. The last thing you need to do is right-click on your component in
the package, select properties, and finally transaction. Then select
the radio button next to: 'Requires a transaction'.

<nl>As stated before, this example is interested in ObjectContext. The
ObjectContext interface is like a friend doing things on your
behalf. You declare what you want, and it helps you with that, always
acting as a buffer between you and the outside world. Many methods are
available from this interface: CreateInstance, SetComplete, SetAbort,
EnableCommit, DisableCommit, IsInTransaction, IsCallerInRole, etc.

<nl>In this example we will deal with: SetComplete and SetAbort, and
IsCallerInRole. Invoking SetComplete will tell MTS that the object is
done and it can go ahead and commit. In contrast, SetAbort means the
object is done but for whatever the transaction must roll
back. IsCallerInRole determines whether or not the client calling the object
is in the role you created that has lists of users or groups.

@ex In python you setup the ObjectContext object in the following manner:|
		mtsobj = win32com.client.Dispatch("MTXAS.AppServer.1")
		mts=mtsobj.GetObjectContext()

@ex and can then make calls like:|
		if mts.IsCallerInRole('bedrock'):
			do_this()

<nl>Below is the active server page and python object that you can
watch at work using "Transaction Statistics" in the Transaction Server
Explorer. Anything but wilma and betty will cause the aborted list to
increment. If wila or betty are supplied from the ASP, you notice
Committed increasing.

@ex Here is an extremely simple python active server page that calls the mts object: |

<SCRIPT Language="Python" RunAt="Server">

import win32com
find_pebbles = win32com.client.dynamic.Dispatch("mts1")

a=find_pebbles('wilma')
Response.Write(a)

</SCRIPT>


@ex Here is the mts COM object: |

from win32com.server.exception import COMException
import win32com.server.util
import win32com.client.dynamic

#to generate guids use:
#import pythoncom
#print pythoncom.CreateGuid()

class Mts:
	# COM attributes.
	_reg_clsid_ = '{3D094770-B73E-11D3-99FC-00902776D585}' 
	               #guid for your class in registry
	_reg_desc_ = "test mts functions" 
	_reg_progid_ = "mts1" #The progid for this class
	_reg_class_spec_ = "mts_test.Mts" 
                           #tells Python how to create the object: filename.class
	_public_methods_ = ['getkid' ]  #names of callable methods

	def __init__(self):
		pass

	def getkid(self, person):

		mtsobj = win32com.client.Dispatch("MTXAS.AppServer.1")
		mts=mtsobj.GetObjectContext()

		if mts is None:
			#com obj -- no mts
			result='error: mts not available'
		else:
			#mts is available
			#first check if they are in the right role	
			if mts.IsCallerInRole('bedrock'):
				moms={'wilma':'bambam','betty':'pebbles'}
				person=str(person) #convert from unicode to string
				if moms.has_key(person):
					mts.SetComplete()
					result=moms[person]
				else:
					result='not in bedrock'
					mts.SetAbort()
			else:
				result='sorry can't let you know'
		return result

if __name__=='__main__':
	import win32com.server.register
	win32com.server.register.UseCommandLine(Mts)
		

@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com       


*/


