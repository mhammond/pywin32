/*
@doc

@topic ASP and Python | How Python integrates w/ASP

<nl>Active Server Pages has been a useful addition to Microsoft's Web
strategy. Because of the language neutral nature of COM, any language
that is capable of "Active X" (meaning COM) scripting can integrate
with it. Python, with it's excellent COM support, is one of those
languages. When you install it, by default, it is not setup for the
ASP enviroment with IIS. If you look at the html documentation in:
C:\Program Files\Python\win32com\HTML\index.html at the bottom of the
page you'll see 'Active X Scripting Demos'. If you then click on that
it will describe how to register the engire.  Essentially, there is a
program called pyscript.py in:
C:\Program Files\Python\win32comext\axscript\client
and if you run that it should register python for the ASP environment.

<nl>Microsoft typically positions vbscript in the IIS role. Vbscript
is adequate for many tasks, but is a very simplified language. Python
is a very rich and powerful language offering things like true
exception handleing, OOP style programming, excellent win32 access,
easy persistence w/pickle, etc. And, being multi-platform, you can
leverage the language in other web environments. You will also find
many ASP code examples in Vbscript are easily converted into python.
There is a slight difference with setting values with Application and
Session Objects. In cases where you'd see: Session('Key')= TheValue,
you'd translate to Session.SetValue('Key', TheValue) for python. This
is because vbscript makes it look like you are assigning something to
a function call which python does not support.

<nl>Another thing ASP allows you to do is to mix code and HTML. Thus,
you need some way to tell the server that the following is code. Since
python is not used by default, you need to set your page to use
it. You can do that by having the first line say: \<%@ LANGUAGE
=Python%\>.  Then after that point, anything between '\<%' and '%\>'
delimeter will run as python code. Everything else will be treated as
HTML. It is important to note that a python loop ends in the python
block -- they don't extend past intervening html to the next block. For
me that normally isn't an issue, since I do not like mixing code and
html. Preferring a clean split of code and html, I generally
generate webpages using templates all in python w/HTMLgen. Take a look
at: https://wiki.python.org/moin/WebBrowserProgramming and
https://wiki.python.org/moin/Asking%20for%20Help/How%20to%20run%20python%20from%20HTML
for available resources w/HTML.  A very basic page would look like:

@ex Basic Python ASP page: |

<%@ LANGUAGE = Python%>
<HTML>
<head></head>
<body>

<h1>Querying database</h1>
<%
#do some python stuff here
#import libraries, etc.
for i in query_database():
	Response.Write('output of results converted to a string')

%>
</body>
</html>

@ex If you use HTMLgen's template, then it could look like:|

<%@ LANGUAGE = Python%>

<%

#notice no raw HTML at all.  We instead use a template html file
#generated however you want (perhaps frontpage) that substitutes
#anything delimited by {} for output from our python.  In this case
#text {mid} in the template is substituted for results.

import HTMLgen
import HTMLcolors

results='<h1>Querying database</h1>'
for i in query_database():
	results=results+str(i)

######################
#Here we are using a template dictionary
#to substitute the text {mid} found in the template file
#for results
T=HTMLgen.TemplateDocument(/path/template_file.html)
T.substitutions['mid']=results  # here is where our results went!!
webpage=str(T)
Response.Write(webpage)
%>

@ex For one page this is overkill. However, for developing many pages,
something like this allows you to ignore the grunt work of making
pages and helps to keep them consistent. HTMLgen also gives you other useful
tools worth looking into.

<nl>The ASP model has several 'collection' objects: Request, Response,
Session, Application, and ObjectContext. The Request collection has
things like data returned from web forms(GET or POST) and environment
variables. The Response object is the opposite of the Request object
-- it deals with sending data back to the client, since you cannot do a
simple 'print'. Next, the Application collection allows programs to
globally store/share information for your site. The Session object
allows one to store per-user state. And, finally the Objectcontext
object is for use with MTS. We'll focus on the basics, Request and
Response.

<nl>Typically, you can treat these objects like simple versions of a python
dictionary or list. A good way to get an idea of what they have is to
print out their entire contents. Here is a simple function that
converts your server (environment) variables into a python dictionary:
|
def getenv ():
	d_env={} #initialize dictionary
	for key in Request.ServerVariables:
		d_env[key]=str(Request.ServerVariables(key))
	return d_env


@ex Request.ServerVariables responds to the python operator 'in' by
returning a list. And, it can also take a key and return it's
value. You notice the value is converted into a string, so it can be
treated like a python string.

<nl> For a more complicated example, here is a python function that
converts the Request.Form or Request.Querystring objects into a python
dictionary. This would be a function you'd use to get back data from a
form. It returns either the entire Request Collection as a python
dictionary, a subset if you provide a list of keys, or a single value,
if you provide a single key.

<nl> Get Request collection - 3 different ways to get Request data
|
def getdata (keys=''):
	'''
	3 possible ways to call this function:
	value=getdata('key')
	dict=getdata(('key1','key2')) #get subset
	dict=getdata() #return everything
	It assumes you don't have the same key for
	GET and POST methods
	'''
	import types
	key_type=type(keys)
	d_data={} #initialize dictionary
	if keys=='': #if they don't supply keys then return everything
		for key in Request.Form:
			d_data[key]=Request.Form(key)
		for key in Request.QueryString:
			d_data[key]=Request.QueryString(key)
		return d_data
	elif key_type == types.StringType: #if they provide a single string
		value=Request.Form(keys)
		if (value != 'None') and (str(value) == 'None'):
			return Request.QueryString(keys)
		else:
			return value
	#if they provide a list then return a dictionary with all the key/values
	elif key_type == types.TupleType or key_type == types.ListType:
		for key in keys:
			value=Request.Form(key)
			#now check if the data was empty, if so look at QueryString
			if (value != 'None') and (str(value) == 'None'):
				value=Request.QueryString(key)
			data[key]=value
		return d_data

@ex To print out this data you will need to use the Response object
which accepts python strings. A simple: Response.Write(str(d_data))
would suffice.  A better looking way would be to do something like:
|
for pair in d_data.items():
	Response.Write(pair[0]+':'+pair[1]+'<br>')

@ex Notice the adding of \<br\> to have a line break for each pair. If you
want it more fancy you can convert it to table output.

<nl>HTMLgen can help with it's Table object:|

Table=HTMLgen.Table('Key/Value pairs for Response object') #title
Table.heading=('Key','Value')
for pair in d_data.items() #get each key/val pair
  Table.body.append(list(pair))  #takes a list of lists
				 #[ [key1,val1], [key2,val2]]
				 #one pair for every table row
Response.Write(str(Table))

@ex HTMLgen deserves an entire article to itself. You can use it to
write web forms and manage other HTML elements. As a final example,
here is a simple ASP application that uses the functions written above
and a simple HTMLgen web form to spit data back. The typical idiom
w/HTMLgen is to create a web object like a form and append things to it.
In the code below, I append a textbox, radio buttons, and a checkbox.
|
<%@ LANGUAGE = Python%>

<%
from HTMLgen import *
from HTMLcolors import *
#import the file which has getenv and getdata from above

d_env=getenv() #get environment variables

#create a simple default document
webpage = SimpleDocument(title = 'Bedrock Housing')

#create form and append elements to it
F=Form(d_env['URL'])
F.append(Heading(1,'Rock Housing'))
F.append('What Street:',BR())
F.append( Input(type='text',name='street',size=30),BR())

F.append('Select your house type:',BR())
types=('limestone','granite','marble')
for i in types:
	F.append(Input(type='radio',name='house_type',rlabel=i,value=i),BR())
F.append('Select special features:',BR())

features=('stone roof','dishwasher','door bell')
for i in features:
	F.append(Input(type='checkbox',name='features',rlabel=i,value=i),BR())
#done with web form, now append it
webpage.append(F)
#get the data the user entered and return it
results=getdata() #get everything
webpage.append(str(results))
Response.Write(str(webpage))

@ex Have a great time with programming with python!
<nl>|John Nielsen   nielsenjf@my-deja.com


*/
