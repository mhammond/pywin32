"""Contains knowledge to build a COM object definition.

This module is used by both the @dynamic@ and @makepy@ modules to build
all knowledge of a COM object.

This module contains classes which contain the actual knowledge of the object.
This include paramater and return type information, the COM dispid and CLSID, etc.

Other modules may use this information to generate .py files, use the information
dynamically, or possibly even generate .html documentation for objects.
"""

#
# NOTES: DispatchItem and MapEntry used by dynamic.py.
#        the rest is used by makepy.py
#
#        OleItem, DispatchItem, MapEntry, BuildCallList() is used by makepy

import sys
import string
import types
from keyword import iskeyword

import pythoncom
from pywintypes import UnicodeType

error = "PythonCOM.Client.Build error"
DropIndirection="DropIndirection"

NoTranslateTypes = [
	pythoncom.VT_BOOL,          pythoncom.VT_CLSID,        pythoncom.VT_CY,
	pythoncom.VT_DATE,          pythoncom.VT_DECIMAL,       pythoncom.VT_EMPTY,
	pythoncom.VT_ERROR,         pythoncom.VT_FILETIME,     pythoncom.VT_HRESULT,
	pythoncom.VT_I1,            pythoncom.VT_I2,           pythoncom.VT_I4,
	pythoncom.VT_I8,            pythoncom.VT_INT,          pythoncom.VT_NULL,
	pythoncom.VT_R4,            pythoncom.VT_R8,           pythoncom.VT_NULL,
	pythoncom.VT_STREAM,
	pythoncom.VT_UI1,            pythoncom.VT_UI2,           pythoncom.VT_UI4,
	pythoncom.VT_UI8,            pythoncom.VT_UINT,          pythoncom.VT_VOID,
	pythoncom.VT_UNKNOWN,
]

NoTranslateMap = {}
for v in NoTranslateTypes:
	NoTranslateMap[v] = None

class MapEntry:
	"Simple holder for named attibutes - items in a map."
	def __init__(self, desc_or_id, names=None, doc=None, resultCLSID=pythoncom.IID_NULL, resultDoc = None, hidden=0):
		if type(desc_or_id)==type(0):
			self.dispid = desc_or_id
			self.desc = None
		else:
			self.dispid = desc_or_id[0]
			self.desc = desc_or_id

		self.names = names
		self.doc = doc
		self.resultCLSID = resultCLSID
		self.resultDocumentation = resultDoc
		self.wasProperty = 0 # Have I been transformed into a function so I can pass args?
		self.hidden = hidden
	def GetResultCLSID(self):
		rc = self.resultCLSID
		if rc == pythoncom.IID_NULL: return None
		return rc
	# Return a string, suitable for output - either "'{...}'" or "None"
	def GetResultCLSIDStr(self):
		rc = self.GetResultCLSID()
		if rc is None: return "None"
		return repr(str(rc)) # Convert the IID object to a string, then to a string in a string.

	def GetResultName(self):
		if self.resultDocumentation is None:
			return None
		return self.resultDocumentation[0]

class OleItem:
  typename = "OleItem"

  def __init__(self, doc=None):
    self.doc = doc
    if self.doc:
        self.python_name = MakePublicAttributeName(self.doc[0])
    else:
        self.python_name = None
    self.bWritten = 0
    self.clsid = None

class DispatchItem(OleItem):
	typename = "DispatchItem"

	def __init__(self, typeinfo=None, attr=None, doc=None, bForUser=1):
		OleItem.__init__(self,doc)
		self.propMap = {}
		self.propMapGet = {}
		self.propMapPut = {}
		self.mapFuncs = {}
		self.defaultDispatchName = None
		self.hidden = 0

		if typeinfo:
			self.Build(typeinfo, attr, bForUser)

	def _propMapPutCheck_(self,key,item):
		ins, outs, opts = self.CountInOutOptArgs(item.desc[2])
		if ins>1: # if a Put property takes more than 1 arg:
			if opts+1==ins or ins==item.desc[6]+1:
				newKey = "Set" + key
				deleteExisting = 0 # This one is still OK
			else:
				deleteExisting = 1 # No good to us
				if self.mapFuncs.has_key(key) or self.propMapGet.has_key(key):
					newKey = "Set" + key
				else:
					newKey = key
			item.wasProperty = 1
			self.mapFuncs[newKey] = item
			if deleteExisting:
				del self.propMapPut[key]

	def _propMapGetCheck_(self,key,item):
		ins, outs, opts = self.CountInOutOptArgs(item.desc[2])
		if ins > 0: # if a Get property takes _any_ in args:
			if item.desc[6]==ins or ins==opts:
				newKey = "Get" + key
				deleteExisting = 0 # This one is still OK
			else:
				deleteExisting = 1 # No good to us
				if self.mapFuncs.has_key(key):
					newKey = "Get" + key
				else:
					newKey = key
			item.wasProperty = 1
			self.mapFuncs[newKey] = item
			if deleteExisting:
				del self.propMapGet[key]

	def	_AddFunc_(self,typeinfo,fdesc,bForUser):
		id = fdesc.memid
		funcflags = fdesc.wFuncFlags

		# skip [restricted] methods, unless it is the
		# enumerator (which, being part of the "system",
		# we know about and can use)
		if funcflags & pythoncom.FUNCFLAG_FRESTRICTED and \
			id != pythoncom.DISPID_NEWENUM:
			return None

		# skip any methods that can't be reached via DISPATCH
		if fdesc.funckind != pythoncom.FUNC_DISPATCH:
			return None

		try:
			names = typeinfo.GetNames(id)
			name=names[0]
		except pythoncom.ole_error:
			name = ""
			names = None

		doc = None
		try:
			if bForUser:
				doc = typeinfo.GetDocumentation(id)
		except pythoncom.ole_error:
			pass

		if id==0 and name:
			self.defaultDispatchName = name

		invkind = fdesc.invkind

		# Translate any Alias or Enums in result.
		fdesc.rettype, resultCLSID, resultDoc = _ResolveUserDefined(fdesc.rettype, typeinfo)
		# Translate any Alias or Enums in argument list.
		argList = []			
		for argDesc in fdesc.args:
			argList.append(_ResolveUserDefined(argDesc, typeinfo)[0])
		fdesc.args = tuple(argList)

		hidden = (funcflags & pythoncom.FUNCFLAG_FHIDDEN) != 0
#			if hidden and name != "Item": # XXX - special hack case.
#				return 0

		if id == pythoncom.DISPID_NEWENUM:
			map = self.mapFuncs
		elif invkind == pythoncom.INVOKE_PROPERTYGET:
			map = self.propMapGet
		# This is not the best solution, but I dont think there is
		# one without specific "set" syntax.
		# If there is a single PUT or PUTREF, it will function as a property.
		# If there are both, then the PUT remains a property, and the PUTREF
		# gets transformed into a function.
		# (in vb, PUT=="obj=other_obj", PUTREF="set obj=other_obj
		elif invkind in (pythoncom.INVOKE_PROPERTYPUT, pythoncom.INVOKE_PROPERTYPUTREF):
			# Special case
			existing = self.propMapPut.get(name, None)
			if existing is not None:
				if existing.desc[4]==pythoncom.INVOKE_PROPERTYPUT: # Keep this one
					map = self.mapFuncs
					name = "Set"+name
				else: # Existing becomes a func.
					existing.wasProperty = 1
					self.mapFuncs["Set"+name]=existing
					map = self.propMapPut # existing gets overwritten below.
			else:
				map = self.propMapPut # first time weve seen it.

		elif invkind == pythoncom.INVOKE_FUNC:
			map = self.mapFuncs
		else:
			map = None
		if not map is None: 
#				if map.has_key(name):
#					sys.stderr.write("Warning - overwriting existing method/attribute %s\n" % name)
			map[name] = MapEntry(tuple(fdesc), names, doc, resultCLSID, resultDoc, hidden)
			return (name,map)
		else:
			return None

	def _AddVar_(self,typeinfo,fdesc,bForUser):
		### need pythoncom.VARFLAG_FRESTRICTED ...
		### then check it

		if fdesc.varkind == pythoncom.VAR_DISPATCH:
			id = fdesc.memid
			names = typeinfo.GetNames(id)
			# Translate any Alias or Enums in result.
			fdesc.elemdescVar, resultCLSID, resultDoc = _ResolveUserDefined(fdesc.elemdescVar, typeinfo)
			doc = None
			try:
				if bForUser: doc = typeinfo.GetDocumentation(id)
			except pythoncom.ole_error:
				pass

			# handle the enumerator specially
			if id == pythoncom.DISPID_NEWENUM:
				map = self.mapFuncs
				### hack together a minimal FUNCDESC
				fdesc = (fdesc[0], None, None, None, pythoncom.INVOKE_PROPERTYGET, )
			else:
				map = self.propMap

			map[names[0]] = MapEntry(tuple(fdesc), names, doc, resultCLSID, resultDoc)
			return (names[0],map)
		else:
			return None

	def Build(self, typeinfo, attr, bForUser = 1):
		self.clsid = attr[0]
		if typeinfo is None: return
		# Loop over all methods
		for j in xrange(attr[6]):
			fdesc = typeinfo.GetFuncDesc(j)
			self._AddFunc_(typeinfo,fdesc,bForUser)

		# Loop over all variables (ie, properties)
		for j in xrange(attr[7]):
			fdesc = typeinfo.GetVarDesc(j)
			self._AddVar_(typeinfo,fdesc,bForUser)
		
		# Now post-process the maps.  For any "Get" or "Set" properties
		# that have arguments, we must turn them into methods.  If a method
		# of the same name already exists, change the name.
		for key, item in self.propMapGet.items():
			self._propMapGetCheck_(key,item)
					
		for key, item in self.propMapPut.items():
			self._propMapPutCheck_(key,item)

	def CountInOutOptArgs(self, argTuple):
		"Return tuple counting in/outs/OPTS.  Sum of result may not be len(argTuple), as some args may be in/out."
		ins = out = opts = 0
		for argCheck in argTuple:
			inOut = argCheck[1]
			if inOut==0:
				ins = ins + 1
				out = out + 1
			else:
				if inOut & pythoncom.PARAMFLAG_FIN:
					ins = ins + 1
				if inOut & pythoncom.PARAMFLAG_FOPT:
					opts = opts + 1
				if inOut & pythoncom.PARAMFLAG_FOUT:
					out = out + 1
		return ins, out, opts

	def MakeFuncMethod(self, entry, name, bMakeClass = 1):
		# If we have a type description, and not varargs...
		if entry.desc is not None and (len(entry.desc) < 6 or entry.desc[6]!=-1):
			return self.MakeDispatchFuncMethod(entry, name, bMakeClass)
		else:
			return self.MakeVarArgsFuncMethod(entry, name, bMakeClass)

	def MakeDispatchFuncMethod(self, entry, name, bMakeClass = 1):
		fdesc = entry.desc
		doc = entry.doc
		names = entry.names
		ret = []
		if bMakeClass:
			linePrefix = "\t"
			defNamedOptArg = "defaultNamedOptArg"
			defNamedNotOptArg = "defaultNamedNotOptArg"
			defUnnamedArg = "defaultUnnamedArg"
		else:
			linePrefix = ""
			defNamedOptArg = "pythoncom.Missing"
			defNamedNotOptArg = "pythoncom.Missing"
			defUnnamedArg = "pythoncom.Missing"
		id = fdesc[0]
		s = linePrefix + 'def ' + name + '(self' + BuildCallList(fdesc, names, defNamedOptArg, defNamedNotOptArg, defUnnamedArg) + '):'
		ret.append(s)
		if doc and doc[1]:
			ret.append(linePrefix + '\t"""' + doc[1] + '"""')

#		print "fdesc is ", fdesc

		resclsid = entry.GetResultCLSID()
		if resclsid:
			resclsid = "'%s'" % resclsid
		else:
			resclsid = 'None'
		# Strip the default values from the arg desc
		retDesc = fdesc[8][:2]
		argsDesc = tuple(map(lambda what: what[:2], fdesc[2]))
		s = None
		if len(retDesc)==2 and retDesc[1]==0:
			rd = retDesc[0]
			if NoTranslateMap.has_key(rd):
				s = '%s\treturn self._oleobj_.InvokeTypes(0x%x, LCID, %s, %s, %s%s)' % (linePrefix, id, fdesc[4], retDesc, argsDesc, _BuildArgList(fdesc, names))
			elif rd==pythoncom.VT_DISPATCH:
				s = '%s\tret = self._oleobj_.InvokeTypes(0x%x, LCID, %s, %s, %s%s)\n' % (linePrefix, id, fdesc[4], retDesc, `argsDesc`, _BuildArgList(fdesc, names))
				s = s + '%s\tif ret is not None: ret = win32com.client.Dispatch(ret, %s, %s, UnicodeToString=1)\n' % (linePrefix,`name`, resclsid) 
				s = s + '%s\treturn ret' % (linePrefix)
			elif rd == pythoncom.VT_BSTR:
				s = '%s\treturn str(self._oleobj_.InvokeTypes(0x%x, LCID, %s, %s, %s%s))' % (linePrefix, id, fdesc[4], retDesc, `argsDesc`, _BuildArgList(fdesc, names))
			# else s remains None
		if s is None:
			s = '%s\treturn self._ApplyTypes_(0x%x, %s, %s, %s, %s, %s%s)' % (linePrefix, id, fdesc[4], retDesc, argsDesc, `name`, resclsid, _BuildArgList(fdesc, names))

		ret.append(s)
		ret.append("")
		return ret

	def MakeVarArgsFuncMethod(self, entry, name, bMakeClass = 1):
		fdesc = entry.desc
		names = entry.names
		doc = entry.doc
		ret = []
		argPrefix = "self"
		if bMakeClass:
			linePrefix = "\t"
			defArg = "defaultArg"
		else:
			linePrefix = ""
			defArg = "pythoncom.Missing"
		ret.append(linePrefix + 'def ' + name + '(' + argPrefix + ', *args):')
		if doc and doc[1]: ret.append(linePrefix + '\t"' + doc[1] + '"')
		if fdesc:
			invoketype = fdesc[4]
		else:
			invoketype = pythoncom.DISPATCH_METHOD
		s = linePrefix + '\treturn self._get_good_object_(apply(self._oleobj_.Invoke,('
		ret.append(s + hex(entry.dispid) + ",0,%d,1)+args),'%s')" % (invoketype, names[0]))
		ret.append("")
		return ret

# A Lazy dispatch item - builds an item on request using info from
# an ITypeComp.  The dynamic module makes the called to build each item,
# and also holds the references to the typeinfo and typecomp.
class LazyDispatchItem(DispatchItem):
	typename = "LazyDispatchItem"
	def __init__(self, attr, doc):
		self.clsid = attr[0]
		DispatchItem.__init__(self, None, attr, doc, 0)
		

def _DoResolveType(typeinfo, hrefResult):
  resultTypeInfo = typeinfo.GetRefTypeInfo(hrefResult)
  resultAttr = resultTypeInfo.GetTypeAttr()
  typeKind = resultAttr[5]
  if typeKind == pythoncom.TKIND_ALIAS:
    ai = resultAttr[14]
    if type(ai)==type(0):
      return ai, resultTypeInfo
    if type(ai)==type(()):
      return _DoResolveType(resultTypeInfo, ai[1])
    return None

  elif typeKind == pythoncom.TKIND_ENUM or \
       typeKind == pythoncom.TKIND_MODULE:
    # For now, assume Long
    return pythoncom.VT_I4, None

  elif typeKind == pythoncom.TKIND_DISPATCH or \
       typeKind == pythoncom.TKIND_INTERFACE:
    return pythoncom.VT_DISPATCH, resultTypeInfo

  elif typeKind == pythoncom.TKIND_COCLASS:
    # Should get default?
    return pythoncom.VT_DISPATCH, resultTypeInfo

def _DoResolveTypeTuple(typeTuple, typeinfo):
  t = typeTuple
  if type(t) != types.TupleType:
    return t, None, None
  if type(t[0]) == types.IntType and type(t[1]) == types.IntType:
    if t[0]==pythoncom.VT_USERDEFINED:
      # Have href.
      rc = _DoResolveType(typeinfo, t[1])
      if rc is None: return t, None, None
      retType, retTypeInfo = rc
      if retTypeInfo:
              retCLSID = retTypeInfo.GetTypeAttr()[0]
              retDocumentation = retTypeInfo.GetDocumentation(-1)
      else:
              retCLSID = retDocumentation = None
      if retType==pythoncom.VT_DISPATCH and retCLSID:
              raise DropIndirection, (retType, retCLSID, retDocumentation)
      return retType, retCLSID, retDocumentation
  if type(t[1])==types.TupleType:	
    try:
      resolved, ret2, ret3 = _DoResolveTypeTuple(t[1],typeinfo)
      return (t[0],resolved), ret2, ret3
    except DropIndirection, (resolved, ret2, ret3):
      return (resolved), ret2, ret3

  return t, None, None
		
def _ResolveUserDefined(typeTuple, typeinfo):
  "Resolve VT_USERDEFINED (often aliases or typed IDispatches)"
  realTypeTuple, inOut, default = typeTuple
  try:
    resolved, ret2, ret3 = _DoResolveTypeTuple(realTypeTuple, typeinfo)
  except DropIndirection, det:
    resolved, ret2, ret3 = det
  return (resolved, inOut, default), ret2, ret3

def _BuildArgList(fdesc, names):
    "Builds list of args to the underlying Invoke method."
    # Word has TypeInfo for Insert() method, but says "no args"
    numArgs = max(fdesc[6], len(fdesc[2]))
    names = list(names)
    while None in names:
    	i = names.index(None)
    	names[i] = "arg%d" % (i,)
    names = map(MakePublicAttributeName, names[1:])
    while len(names) < numArgs:
        names.append("arg%d" % (len(names),))
    return "," + string.join(names, ", ")


valid_identifier_chars = string.letters + string.digits + "_"

# Given a "public name" (eg, the name of a class, function, etc)
# make sure it is a legal (and reasonable!) Python name.
def MakePublicAttributeName(className):
	# Given a class attribute that needs to be public, but Python munges
	# convert it.
	if className[:2]=='__' and className[-2:]!='__':
		return className[1:]
	elif iskeyword(className):
		return string.capitalize(className)
	# Strip non printable chars
	return filter( lambda char: char in valid_identifier_chars, className)

# Given a default value passed by a type library, return a string with
# an appropriate repr() for the type.
# Takes a raw ELEMDESC and returns a repr string, or None
# (NOTE: The string itself may be '"None"', which is valid, and different to None.
# XXX - To do: Dates are probably screwed, but can they come in?
def MakeDefaultArgRepr(defArgVal):
  try:
    inOut = defArgVal[1]
  except IndexError:
    # something strange - assume is in param.
    inOut = pythoncom.PARAMFLAG_FIN

  if not inOut==pythoncom.PARAMFLAG_NONE and not (inOut & pythoncom.PARAMFLAG_FIN):
    # If it is not an inout param, override default to "None".
    # This allows us to "hide" out params.
    return "None"
  elif inOut & pythoncom.PARAMFLAG_FHASDEFAULT:
    # hack for Unicode until it repr's better.
    val = defArgVal[2]
    if type(val) is UnicodeType:
      return repr(str(val))
    else:
      return repr(val)
  return None

def BuildCallList(fdesc, names, defNamedOptArg, defNamedNotOptArg, defUnnamedArg):
  "Builds a Python declaration for a method."
  # Names[0] is the func name - param names are from 1.
  numArgs = len(fdesc[2])
  numOptArgs = fdesc[6]
  strval = ''
  if numOptArgs==-1:	# Special value that says "var args after here"
    numArgs = numArgs - 1
  else:
    firstOptArg = numArgs - numOptArgs
  for arg in xrange(numArgs):
    try:
      argName = names[arg+1] 
      namedArg = argName is not None
    except IndexError:
      namedArg = 0
    if not namedArg: argName = "arg%d" % (arg+1)
      
    # See if the IDL specified a default value
    defArgVal = MakeDefaultArgRepr(fdesc[2][arg])
    if defArgVal is None:
      # Unnamed arg - always allow default values.
      if namedArg:
        # Is a named argument
        if arg >= firstOptArg:
          defArgVal = defNamedOptArg
        else:
          defArgVal = defNamedNotOptArg
      else:
        defArgVal = defUnnamedArg

    argName = MakePublicAttributeName(argName)
    strval = strval + ", " + argName
    if defArgVal:
      strval = strval + "=" + defArgVal
  if numOptArgs==-1:
    strval = strval + ", *" + names[-1]

  return strval


if __name__=='__main__':
  print "Use 'makepy.py' to generate Python code - this module is just a helper"
