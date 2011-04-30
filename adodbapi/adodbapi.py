"""adodbapi - A python DB API 2.0 (PEP 249) interface to Microsoft ADO

Copyright (C) 2002 Henrik Ekelund, version 2.1 by Vernon Cole
* http://sourceforge.net/projects/pywin32
* http://sourceforge.net/projects/adodbapi

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    django adaptations and refactoring by Adam Vandenberg

DB-API 2.0 specification: http://www.python.org/dev/peps/pep-0249/

This module source should run correctly in CPython versions 2.3 and later,
or IronPython version 2.6 and later,
or, after running through 2to3.py, CPython 3.0 or later.
"""
__version__ = '2.4.2.2'
version = 'adodbapi v' + __version__
# N.O.T.E.:...
# if you have been using an older version of adodbapi and are getting errors because
# numeric and monitary data columns are now returned as Decimal data,
# try adding the following line to get that data as strings: ...
#adodbapi.variantConversions[adodbapi.adoCurrencyTypes]=adodbapi.cvtString # get currency as strings
#adodbapi.variantConversions[adodbapi.adoExactNumericTypes]=adodbapi.cvtString

import sys
import time
#import traceback
import datetime

try:
    import decimal
except ImportError:  #perhaps running Cpython 2.3 
    import win32com.decimal_23 as decimal
# or # from django.utils import _decimal as decimal

onIronPython = sys.platform == 'cli'
if not onIronPython:
    try:
        import win32com.client
    except ImportError:
        import warnings
        warnings.warn("pywin32 package required but not found.",ImportWarning)

# --- define objects to smooth out IronPython <-> CPython differences    
if onIronPython: 
    from System import Activator, Type, DBNull, DateTime, Array, Byte
    from System import Decimal as SystemDecimal
    from clr import Reference
    def Dispatch(dispatch):
        type = Type.GetTypeFromProgID(dispatch)
        return Activator.CreateInstance(type)
    def getIndexedValue(obj,index):
        return obj.Item[index]
else: #pywin32
    try:
        import pythoncom
        import pywintypes
        pythoncom.__future_currency__ = True
        def Dispatch(dispatch):
            return win32com.client.Dispatch(dispatch)
    except:
        pass #warning already given above
    def getIndexedValue(obj,index):
        return obj(index) 
    DBNull = type(None)
    DateTime = type(NotImplemented) #impossible value 

import ado_consts as adc  #internal to this module, use the new sub_module

# --- define objects to smooth out Python3000 <-> Python 2.x differences
unicodeType = unicode  #this line will be altered by 2to3.py to '= str'
longType = long        #this line will be altered by 2to3.py to '= int'
if sys.version[0] >= '3': #python 3.x
    StringTypes = str
    makeByteBuffer = bytes
    memoryViewType = memoryview
    _BaseException = Exception
else:                   #python 2.x
    from exceptions import StandardError as _BaseException
    memoryViewType = type(buffer(''))
    makeByteBuffer = buffer
    try:                #jdhardy -- handle bytes under IronPython
        bytes
    except NameError:
        bytes = str
    StringTypes = (str,unicode)  # will be messed up by 2to3 but never used
    from ado_consts import *  #define old way of getting constants (for callers)

def standardErrorHandler(connection,cursor,errorclass,errorvalue):
    err = (errorclass,errorvalue)
    connection.messages.append(err)
    if cursor is not None:
        cursor.messages.append(err)
    raise errorclass(errorvalue)

# -----     Time converters ----------------------------------------------
class TimeConverter(object):  # this is a generic time converter skeleton
    def __init__(self):       # the details will be filled in by instances
        self._ordinal_1899_12_31=datetime.date(1899,12,31).toordinal()-1
        #Use self.types to compare if an input parameter is a datetime 
        self.types = (type(self.Date(2000,1,1)),
                      type(self.Time(12,1,1)),
                      type(self.Timestamp(2000,1,1,12,1,1)))
    def COMDate(self,obj):
        'Returns a ComDate from a datetime in inputformat'
        raise NotImplementedError   #"Abstract class"
    def DateObjectFromCOMDate(self,comDate):
        'Returns an object of the wanted type from a ComDate'
        raise NotImplementedError   #"Abstract class"
    def Date(self,year,month,day):
        "This function constructs an object holding a date value. "
        raise NotImplementedError   #"Abstract class"
    def Time(self,hour,minute,second):
        "This function constructs an object holding a time value. "
        raise NotImplementedError   #"Abstract class"
    def Timestamp(self,year,month,day,hour,minute,second):
        "This function constructs an object holding a time stamp value. "
        raise NotImplementedError   #"Abstract class"
    def DateObjectToIsoFormatString(self,obj):
        "This function should return a string in the format 'YYYY-MM-dd HH:MM:SS:ms' (ms optional) "
        raise NotImplementedError   #"Abstract class"        

# -- Optional: if mx extensions are installed you may use mxDateTime ----
try:
    import mx.DateTime
    mxDateTime = True
# to change the default time converter, use something like:
# adodbapi.adodbapi.dateconverter = adodbapi.mxDateTimeConverter()
except:
    mxDateTime = False
if mxDateTime:
    class mxDateTimeConverter(TimeConverter): # used optionally if
        def COMDate(self,obj):
            return obj.COMDate()
        def DateObjectFromCOMDate(self,comDate):
            return mx.DateTime.DateTimeFromCOMDate(comDate)
        def DateObjectFromCOMDate(self,comDate):
            return mx.DateTime.DateTimeFromCOMDate(comDate)
        def Date(self,year,month,day):
            return mx.DateTime.Date(year,month,day)
        def Time(self,hour,minute,second):
            return mx.DateTime.Time(hour,minute,second)
        def Timestamp(self,year,month,day,hour,minute,second):
            return mx.DateTime.Timestamp(year,month,day,hour,minute,second)
        def DateObjectToIsoFormatString(self,obj):
            return obj.Format('%Y-%m-%d %H:%M:%S')
else:
    class mxDateTimeConverter(TimeConverter):
        pass    # if no mx is installed

class pythonDateTimeConverter(TimeConverter): # standard since Python 2.3
    def __init__(self):       
        TimeConverter.__init__(self)
    def COMDate(self,obj):
        tt=obj.timetuple()
        try:
            ms=obj.microsecond
        except:
            ms=0
        YMDHMSmsTuple=tuple(tt[:6] + (ms,))
        return self.COMDateFromTuple(YMDHMSmsTuple)
    def DateObjectFromCOMDate(self,comDate):
        if isinstance(comDate,datetime.datetime):
            return comDate.replace(tzinfo=None) # make non aware
        elif isinstance(comDate,DateTime):
            fComDate = comDate.ToOADate() # ironPython clr Date/Time
        else:
            fComDate=float(comDate) #ComDate is number of days since 1899-12-31
        integerPart = int(fComDate)
        floatpart=fComDate-integerPart
        ##if floatpart == 0.0:
        ##    return datetime.date.fromordinal(integerPart + self._ordinal_1899_12_31)
        dte=datetime.datetime.fromordinal(integerPart + self._ordinal_1899_12_31) \
           + datetime.timedelta(milliseconds=floatpart*86400000)
               # millisecondsperday=86400000 # 24*60*60*1000
        return dte
    def Date(self,year,month,day):
        return datetime.date(year,month,day)
    def Time(self,hour,minute,second):
        return datetime.time(hour,minute,second)
    def Timestamp(self,year,month,day,hour,minute,second):
        return datetime.datetime(year,month,day,hour,minute,second)
    def COMDateFromTuple(self,YMDHMSmsTuple):
        d = datetime.date(YMDHMSmsTuple[0],YMDHMSmsTuple[1],YMDHMSmsTuple[2])
        integerPart = d.toordinal() - self._ordinal_1899_12_31
        ms = ((YMDHMSmsTuple[3]*60 \
               +YMDHMSmsTuple[4])*60 \
              +YMDHMSmsTuple[5])*1000 \
           +YMDHMSmsTuple[6]
        fractPart = ms / 86400000.0
        return integerPart + fractPart
    def DateObjectToIsoFormatString(self,obj):
        if isinstance(obj,datetime.datetime):
            s = obj.strftime('%Y-%m-%d %H:%M:%S')
        elif isinstance(obj,datetime.date): #exact midnight
            s = obj.strftime('%Y-%m-%d 00:00:00')
        else:
            try:  #usually datetime.datetime
                s = obj.isoformat()
            except:  #but may be mxdatetime
                s = obj.Format('%Y-%m-%d %H:%M:%S')
        return s

class pythonTimeConverter(TimeConverter): # the old, ?nix type date and time 
    def __init__(self): #caution: this Class gets confised by timezones and DST
        TimeConverter.__init__(self)
    def COMDate(self,timeobj):
        return self.COMDateFromTuple(timeobj)
    def COMDateFromTuple(self,t):
        d = datetime.date(t[0],t[1],t[2])
        integerPart = d.toordinal() - self._ordinal_1899_12_31
        sec = (t[3]*60 + t[4])*60 + t[5]
        fractPart = sec / 86400.0
        return integerPart + fractPart
    def DateObjectFromCOMDate(self,comDate):
        'Returns ticks since 1970'
        if isinstance(comDate,datetime.datetime):
            return comDate.timetuple()
        elif isinstance(comDate,DateTime): # ironPython clr date/time
            fcomDate = comDate.ToOADate()
        else:
            fcomDate = float(comDate)
        secondsperday=86400 # 24*60*60
        #ComDate is number of days since 1899-12-31, gmtime epoch is 1970-1-1 = 25569 days
        t=time.gmtime(secondsperday*(fcomDate-25569.0))
        return t  #year,month,day,hour,minute,second,weekday,julianday,daylightsaving=t
    def Date(self,year,month,day):
        return self.Timestamp(year,month,day,0,0,0)
    def Time(self,hour,minute,second):
        return time.gmtime((hour*60+minute)*60 + second)
    def Timestamp(self,year,month,day,hour,minute,second):
        return time.localtime(time.mktime((year,month,day,hour,minute,second,0,0,-1)))
    def DateObjectToIsoFormatString(self,obj):
        try:
            s = time.strftime('%Y-%m-%d %H:%M:%S',obj)
        except:
            s = obj.strftime('%Y-%m-%d')
        return s

dateconverter = pythonDateTimeConverter() # default
# -----------------------------------------------------------
# ------- Error handlers ------
# Note: _BaseException is defined differently between Python 2.x and 3.x
class Error(_BaseException):
    pass   #Exception that is the base class of all other error
            #exceptions. You can use this to catch all errors with one
            #single 'except' statement. Warnings are not considered
            #errors and thus should not use this class as base. It must
            #be a subclass of the Python StandardError (defined in the
            #module exceptions).
class Warning(_BaseException):
    pass
class InterfaceError(Error):
    pass
class DatabaseError(Error):
    pass
class InternalError(DatabaseError):
    pass
class OperationalError(DatabaseError):
    pass
class ProgrammingError(DatabaseError):
    pass
class IntegrityError(DatabaseError):
    pass
class DataError(DatabaseError):
    pass
class NotSupportedError(DatabaseError):
    pass
verbose = False

# -----------------  The .connect method -----------------
def connect(connection_string, timeout=30):
    """Connect to a database.

    connection_string -- An ADODB formatted connection string, see:
         * http://www.connectionstrings.com
         * http://www.asp101.com/articles/john/connstring/default.asp
    timeout -- A command timeout value, in seconds (default 30 seconds)
    """
    try:
        if not onIronPython:
            pythoncom.CoInitialize()             #v2.1 Paj
        c=Dispatch('ADODB.Connection') #connect _after_ CoIninialize v2.1.1 adamvan
    except:
        raise InterfaceError #Probably COM Error
    if verbose:
        print '%s attempting: "%s"' % (version,connection_string)
    try:
        c.CommandTimeout = timeout
        c.ConnectionString = connection_string
        c.Open()
        return Connection(c)
    except (Exception), e:
        raise OperationalError(e, "Error opening connection: " + connection_string)

# ------ DB API required module attributes ---------------------
apilevel='2.0' #String constant stating the supported DB API level.

threadsafety=1 
# Integer constant stating the level of thread safety the interface supports,
# 1 = Threads may share the module, but not connections. 
# TODO: Have not tried this, maybe it is better than 1?
## 
## Possible values are:
##0 = Threads may not share the module. 
##1 = Threads may share the module, but not connections. 
##2 = Threads may share the module and connections. 
##3 = Threads may share the module, connections and cursors. 

paramstyle='qmark' # the default parameter style
# the API defines this as a constant:
#   "String constant stating the type of parameter marker formatting expected by the interf  ace." 
# -- but as an extension, adodbapi will allow the programmer to change paramstyles
# by making the paramstyle also an attribute of the connection,
# and allowing the programmer to assign one of the permitted values:
#  'qmark' = Question mark style, e.g. '...WHERE name=?'
#  'named' = Named style, e.g. '...WHERE name=:name'
#  'format' = ANSI C printf format codes, e.g. '...WHERE name=%s'
_accepted_paramstyles = ('qmark','named','format')
# so you could use something like:
#   myConnection.paramstyle = 'named'
# The programmer may also change the default.
#   For example, if I were using django, I would say:
#     import adodbapi as Database
#     Database.adodbapi.paramstyle = 'format'

# ------- other module level defaults --------
defaultIsolationLevel = adc.adXactReadCommitted
#  Set defaultIsolationLevel on module level before creating the connection.
#   For example:
#   import adodbapi, ado_consts
#   adodbapi.adodbapi.defaultIsolationLevel=ado_consts.adXactBrowse"
#
#  Set defaultCursorLocation on module level before creating the connection.
# It may be one of the "adUse..." consts.
defaultCursorLocation = adc.adUseClient   # changed from adUseServer as of v 2.3.0

# ----- handy constansts --------
# Used for COM to Python date conversions.
_ordinal_1899_12_31 = datetime.date(1899,12,31).toordinal()-1

def format_parameters(ADOparameters, show_value=False):
    """Format a collection of ADO Command Parameters.

    Used by error reporting in _execute_command.
    """
    try:
        if show_value:
            desc = [
                "Name: %s, Dir.: %s, Type: %s, Size: %s, Value: \"%s\", Precision: %s, NumericScale: %s" %\
                (p.Name, adc.directions[p.Direction], adc.adTypeNames.get(p.Type, str(p.Type)+' (unknown type)'), p.Size, p.Value, p.Precision, p.NumericScale)
                for p in ADOparameters ]
        else:
            desc = [
                "Name: %s, Dir.: %s, Type: %s, Size: %s, Precision: %s, NumericScale: %s" %\
                (p.Name, adc.directions[p.Direction], adTypeNames.get(p.Type, str(p.Type)+' (unknown type)'), p.Size, p.Precision, p.NumericScale)
                for p in ADOparameters ]
        return '[' + '\n'.join(desc) + ']'
    except:
        return '[]'

def _configure_parameter(p, value, settings_known):
    """Configure the given ADO Parameter 'p' with the Python 'value'."""
    if verbose > 3:
        print 'Configuring  parameter %s type=%s value="%s"' % (p.Name,p.Type,repr(value))

    if p.Direction not in [adc.adParamInput, adc.adParamInputOutput, adc.adParamUnknown]:
        return

    if isinstance(value,StringTypes):            #v2.1 Jevon
        L = len(value)
        if p.Type in adoStringTypes: #v2.2.1 Cole
            if settings_known: L = min(L,p.Size) #v2.1 Cole limit data to defined size
            p.Value = value[:L]       #v2.1 Jevon & v2.1 Cole
        else:
            p.Value = value    # dont limit if db column is numeric
        if L>0:   #v2.1 Cole something does not like p.Size as Zero
            p.Size = L           #v2.1 Jevon

    elif isinstance(value, memoryViewType):
        p.Size = len(value)
        p.AppendChunk(value)

    elif isinstance(value, decimal.Decimal):
        if onIronPython:
            s = str(value)
            p.Value = s
            p.Size = len(s)
        else:
            p.Value = value
        exponent = value.as_tuple()[2]
        digit_count = len(value.as_tuple()[1])
        p.Precision =  digit_count
        if exponent == 0:
            p.NumericScale = 0
        elif exponent < 0:
            p.NumericScale = -exponent
            if p.Precision < p.NumericScale:
                p.Precision = p.NumericScale            
        else:  # exponent > 0:
            p.NumericScale = 0
            p.Precision = digit_count + exponent

    elif type(value) in dateconverter.types:
        if settings_known and p.Type in adoDateTimeTypes:
            p.Value=dateconverter.COMDate(value)
        else: #probably a string
                #Known problem with JET Provider. Date can not be specified as a COM date.
                # See for example..http://support.microsoft.com/default.aspx?scid=kb%3ben-us%3b284843
                # One workaround is to provide the date as a string in the format 'YYYY-MM-dd'
            s = dateconverter.DateObjectToIsoFormatString(value)
            p.Value = s
            p.Size = len(s)

    elif isinstance(value, longType) and onIronPython: # Iron Python Long
        s = SystemDecimal(value)  # feature workaround for IPy 2.0
        p.Value = s

    else:
        # For any other type, set the value and let pythoncom do the right thing.
        p.Value = value

# # # # # ----- the Class that defines a connection ----- # # # # # 
class Connection(object):
    # include connection attributes required by api definition.
    Warning = Warning
    Error = Error
    InterfaceError = InterfaceError
    DataError = DataError
    DatabaseError = DatabaseError
    OperationalError = OperationalError
    IntegrityError = IntegrityError
    InternalError = InternalError
    NotSupportedError = NotSupportedError
    ProgrammingError = ProgrammingError

    def __init__(self,adoConn):       
        self.adoConn=adoConn
        self.paramstyle = paramstyle
        self.supportsTransactions=False
        self.dbms_name = ''
        self.dbms_version = ''
        for property in adoConn.Properties:  #Rod Mancisidor ( mancisidor ) 
            if property.Name == 'Transaction DDL':
                if property.Value != 0:        #v2.1 Albrecht
                    self.supportsTransactions=True
            if property.Name == 'DBMS Name':
                self.dbms_name = property.Value
            if property.Name == 'DBMS Version':
                self.dbms_version = property.Value
        self.adoConn.CursorLocation = defaultCursorLocation #v2.1 Rose
        if self.supportsTransactions:
            self.adoConn.IsolationLevel=defaultIsolationLevel
            self.adoConn.BeginTrans() #Disables autocommit
        self.errorhandler=None
        self.messages=[]
        if verbose:
            print 'adodbapi New connection at %X' % id(self)

    def _raiseConnectionError(self, errorclass, errorvalue):
        eh = self.errorhandler
        if eh is None:
            eh = standardErrorHandler
        eh(self, None, errorclass, errorvalue)

    def _closeAdoConnection(self):                  #all v2.1 Rose
        """close the underlying ADO Connection object,
           rolling it back first if it supports transactions."""
        if self.supportsTransactions:
            self.adoConn.RollbackTrans()
        self.adoConn.Close()
        if verbose:
            print 'adodbapi Closed connection at %X' % id(self)

    def close(self):
        """Close the connection now (rather than whenever __del__ is called).

        The connection will be unusable from this point forward;
        an Error (or subclass) exception will be raised if any operation is attempted with the connection.
        The same applies to all cursor objects trying to use the connection. 
        """
        self.messages=[]

        try:
            self._closeAdoConnection()                      #v2.1 Rose
        except (Exception), e:
            self._raiseConnectionError(InternalError,e)
        self.adoConn = None                             #v2.4.2.2 fix subtle timeout bug

    def commit(self):
        """Commit any pending transaction to the database.

        Note that if the database supports an auto-commit feature,
        this must be initially off. An interface method may be provided to turn it back on. 
        Database modules that do not support transactions should implement this method with void functionality. 
        """
        self.messages = []
        if not self.supportsTransactions:
            return

        try:
            self.adoConn.CommitTrans()
            if not(self.adoConn.Attributes & adc.adXactCommitRetaining):
                #If attributes has adXactCommitRetaining it performs retaining commits that is,
                #calling CommitTrans automatically starts a new transaction. Not all providers support this.
                #If not, we will have to start a new transaction by this command:
                self.adoConn.BeginTrans()
        except (Exception), e:
            self._raiseConnectionError(Error, e)

    def rollback(self):
        """In case a database does provide transactions this method causes the the database to roll back to
        the start of any pending transaction. Closing a connection without committing the changes first will
        cause an implicit rollback to be performed.

        If the database does not support the functionality required by the method, the interface should
        throw an exception in case the method is used. 
        The preferred approach is to not implement the method and thus have Python generate
        an AttributeError in case the method is requested. This allows the programmer to check for database
        capabilities using the standard hasattr() function. 

        For some dynamically configured interfaces it may not be appropriate to require dynamically making
        the method available. These interfaces should then raise a NotSupportedError to indicate the
        non-ability to perform the roll back when the method is invoked. 
        """
        self.messages=[]        
        if not self.supportsTransactions:
            self._raiseConnectionError(NotSupportedError, None)
        self.adoConn.RollbackTrans()
        if not(self.adoConn.Attributes & adc.adXactAbortRetaining):
            #If attributes has adXactAbortRetaining it performs retaining aborts that is,
            #calling RollbackTrans automatically starts a new transaction. Not all providers support this.
            #If not, we will have to start a new transaction by this command:
            self.adoConn.BeginTrans()

        #TODO: Could implement the prefered method by havins two classes,
        # one with trans and one without, and make the connect function choose which one.
        # the one without transactions should not implement rollback

    def cursor(self):
        "Return a new Cursor Object using the connection."
        self.messages = []        
        return Cursor(self)

    def printADOerrors(self):
        j=self.adoConn.Errors.Count
        if j:
            print 'ADO Errors:(%i)' % j
        for e in self.adoConn.Errors:
            print 'Description: %s' % e.Description
            print 'Error: %s %s ' % (e.Number, adc.adoErrors.get(e.Number, "unknown"))
            if e.Number == adc.ado_error_TIMEOUT:
                print 'Timeout Error: Try using adodbpi.connect(constr,timeout=Nseconds)'
            print 'Source: %s' % e.Source
            print 'NativeError: %s' % e.NativeError
            print 'SQL State: %s' % e.SQLState

    def _suggest_error_class(self):
        """Introspect the current ADO Errors and determine an appropriate error class.

        Error.SQLState is a SQL-defined error condition, per the SQL specification:
        http://www.contrib.andrew.cmu.edu/~shadow/sql/sql1992.txt

        The 23000 class of errors are integrity errors.
        Error 40002 is a transactional integrity error.
        """
        if self.adoConn is not None:
            for e in self.adoConn.Errors:
                state = str(e.SQLState)
                if state.startswith('23') or state=='40002':
                    return IntegrityError
        return DatabaseError

    def __del__(self):
        try:
            self._closeAdoConnection()                  #v2.1 Rose
        except:
            pass
        self.adoConn = None

    def __setattr__(self,name,value):
        if name == 'paramstyle':
            if value not in _accepted_paramstyles:
                self._raiseConnectionError(NotSupportedError,
                                           'paramstyle="'+value+'" not in:'+repr(_accepted_paramstyles))
        object.__setattr__(self, name, value)

# # # # # classes to emulate the result of cursor.fetchxxx() as a sequence of sequences # # # # #
class _SQLrow(object): # a single database row
    # class to emulate a sequence, so that a column may be retrieved by either number or name
    def __init__(self,rows,index): # "rows" is an _SQLrows object, index is which row
        # note: self.__setattr__ is disabled to prevent users from trying to store in a row 
        object.__setattr__(self,'rows',rows) # parent 'fetch' container object
        object.__setattr__(self,'index',index) # row number within parent
    def __getattr__(self,name): # used for row.columnName type of value access
        return self._getValue(self.rows.columnNames[name.lower()])
    def __setattr__(self,name,value):
        raise NotSupportedError('Cannot assign value to SQL record column')
    def _getValue(self,key):  # key must be an integer
        if onIronPython: # retrieve from two-dimensional array
            v = self.rows.ado_results[key,self.index]
        else: # pywin32 - retrieve from tuple of tuples
            v = self.rows.ado_results[key][self.index]
        return _convert_to_python(v,self.rows.converters[key])
    def __len__(self):
        return len(self.rows.converters)
    def __getitem__(self,key):       # used for row[key] type of value access
        if isinstance(key,int):       # normal row[1] designation
            try:
                return self._getValue(key)
            except IndexError:
                raise
        if isinstance(key, slice):
            indices = key.indices(len(self.rows.converters))
            vl = [self._getValue(i) for i in range(*indices)]
            return tuple(vl)
        try:
            return self._getValue(self.rows.columnNames[key.lower()])  # extension row[columnName] designation
        except (KeyError,TypeError):
            er, st, tr = sys.exc_info()
            raise er,'No such key as "%s" in %s'%(repr(key),self.__repr__()),tr
    def __iter__(self):
        return iter(self.__next__())
    def __next__(self):
        for n in range(len(self.rows.converters)):
             yield self._getValue(n)
    def __repr__(self): # create a human readable representation
        try: #python 2.4 and later
            taglist = sorted(self.rows.columnNames.items(),key=lambda x:x[1])
        except NameError: # no such function as "sorted" on 2.3
            taglist = list(self.rows.columnNames.iteritems())
            taglist.sort(lambda x, y: cmp(x[1], y[1])) #deprecated on 3.0
        s = "<SQLrow={"
        for name, i in taglist:
            s += name + ':' + repr(self._getValue(i)) + ', '
        return s[:-2] + '}>'
    def __str__(self): # create a pretty human readable representation
        return str(tuple([str(self._getValue(i)) for i in range(len(self.rows.converters))]))
# # # #
class _SQLrows(object):
    # class to emulate a sequence for multiple rows using a container object
    def __init__(self,ado_results,numberOfRows,cursor):
        self.ado_results = ado_results # raw result of SQL get
        self.numberOfRows = numberOfRows
        self.converters = []  # convertion function for each column
        self.columnNames = {} # names of columns {name:number,...}
        for i,desc in enumerate(cursor.description):
            self.columnNames[desc[0].lower()] = i  # columnNames lookup
            self.converters.append(variantConversions[desc[1]]) # default convertion function
    def __len__(self):
         return self.numberOfRows
    def __getitem__(self,item):     # used for row or row,column access
        if isinstance(item, slice): # will return a tuple of row objects 
            indices = item.indices(self.numberOfRows)
            l = [_SQLrow(self,k) for k in range(*indices)]
            return tuple(l) #no generator expressions in Python 2.3
        elif isinstance(item, tuple) and len(item)==2:
            # d = some_rowsObject[i,j] will return a datum from a two-dimension address
            i,j = item
            if not isinstance(j,int):
                try:
                    j = self.columnNames[j.lower()] # convert named column to numeric
                except KeyError:
                    raise KeyError, 'adodbapi: no such column name as "%s"'%repr(j)
            if onIronPython: # retrieve from two-dimensional array
                v = self.ado_results[j,i]
            else: # pywin32 - retrieve from tuple of tuples
                v = self.ado_results[j][i]
            return _convert_to_python(v,self.converters[j])
        else:
            row = _SQLrow(self,item) # new row descriptor
            return row
    def __iter__(self):
        return iter(self.__next__())
    def __next__(self):
        for n in range(self.numberOfRows):
            row = _SQLrow(self,n)
            yield row
# # # # #
def _convert_to_python(variant, function): # convert DB value into Python value
    if isinstance(variant,DBNull):
        return None
    return function(variant)

# # # # # ----- the Class that defines a cursor ----- # # # # #
class Cursor(object):
## ** api required attributes:
## description...
##    This read-only attribute is a sequence of 7-item sequences.
##    Each of these sequences contains information describing one result column:
##        (name, type_code, display_size, internal_size, precision, scale, null_ok).
##    This attribute will be None for operations that do not return rows or if the
##    cursor has not had an operation invoked via the executeXXX() method yet.
##    The type_code can be interpreted by comparing it to the Type Objects specified in the section below.
## rowcount...
##    This read-only attribute specifies the number of rows that the last executeXXX() produced
##    (for DQL statements like select) or affected (for DML statements like update or insert). 
##    The attribute is -1 in case no executeXXX() has been performed on the cursor or
##    the rowcount of the last operation is not determinable by the interface.[7]
##    NOTE: -- adodbapi returns "-1" by default for all select statements
## arraysize...
##    This read/write attribute specifies the number of rows to fetch at a time with fetchmany().
##    It defaults to 1 meaning to fetch a single row at a time. 
##    Implementations must observe this value with respect to the fetchmany() method,
##    but are free to interact with the database a single row at a time.
##    It may also be used in the implementation of executemany(). 
## ** extension attributes:
## paramstyle...
##   allows the programmer to override the connection's default paramstyle
## errorhandler...
##   allows the programmer to override the connection's default error handler
    
    def __init__(self,connection):
        self.messages=[]        
        self.connection = connection
        self.paramstyle = connection.paramstyle  # used for overriding the paramstyle
        self.rs = None  # the ADO recordset for this cursor
        self.description = None
        self.errorhandler = connection.errorhandler
        self.arraysize = 1
        if verbose:
            print 'adodbapi New cursor at %X on conn %X' % (id(self),id(self.connection))

    def __iter__(self):                   # [2.1 Zamarev]
        return iter(self.fetchone, None)  # [2.1 Zamarev]

    def next(self):
        r = self.fetchone()
        if r:
            return r
        raise StopIteration

    def __enter__(self):
        "Allow database cursors to be used with context managers."
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        "Allow database cursors to be used with context managers."
        self.close()

    def _raiseCursorError(self,errorclass,errorvalue):
        eh = self.errorhandler
        if eh is None:
            eh = standardErrorHandler
        eh(self.connection,self,errorclass,errorvalue)

    def format_description(self,d):
        """Format db_api description tuple for printing."""
        if isinstance(d,int):
            d = self.description[d]
        desc = "Name= %s, Type= %s, DispSize= %s, IntSize= %s, Precision= %s, Scale= %s NullOK=%s" %\
             (d[0], adc.adTypeNames.get(d[1], str(d[1])+' (unknown type)'),
              d[2], d[3], d[4], d[5], d[6])
        return desc

    def _namedToQmark(self,op,parms):  #convert from 'named' paramstyle to ADO required '?'mark parameters
        if type(parms) != type({}):
            raise ProgrammingError("paramstyle 'named' requires dict parameters")
        outOp = ''
        outparms=[]
        chunks = op.split("'")   #quote all literals -- odd numbered list results are literals.
        inQuotes = False
        for chunk in chunks:
            if inQuotes: # this is inside a quote
                if inQuotes != '' and chunk == '': # double apostrophe to quote one apostrophe
                    outOp = outOp[:-1]  # so take one away
                else:
                    outOp += "'"+chunk+"'" # else pass the quoted string as is.
            else: # is SQL code -- look for a :namedParameter
                while chunk: # some SQL string remains
                    sp = chunk.split(':',1)
                    outOp += sp[0]  # concat the part up to the :
                    s = ''
                    if len(sp)>1:
                        chunk = sp[1]
                    else:
                        chunk = None
                    while chunk:  # there was a parameter - parse it out letter by letter
                        c = chunk[0]
                        if c.isalnum() or c == '_':
                            s += c
                            chunk = chunk[1:]
                        else: #end of parameter
                            break
                    if s:
                        outparms.append(parms[s]) # list the parameters in order
                        outOp += '?'  # put in the Qmark
            inQuotes = not inQuotes
        return (outOp,outparms)

    def _formatToQmark(self,op):  #convert from 'format' paramstyle to ADO required '?'mark parameters
        outOp = ''
        chunks = op.split("'")   #quote all literals -- odd numbered list results are literals.
        inQuotes = False
        for chunk in chunks:
            if inQuotes:
                if outOp != '' and chunk=='': # he used a double apostrophe to quote one apostrophe
                    outOp = outOp[:-1]  # so take one away
                else:
                    outOp += "'"+chunk+"'" # else pass the quoted string as is.
            else: # is SQL code -- look for a %s parameter
                sp = chunk.split('%s')  # make each %s
                outOp += "?".join(sp)   # into ?
            inQuotes =  not inQuotes # every other chunk is a quoted string
        return outOp

    def _makeDescriptionFromRS(self,recordset):
        # Abort if closed or no recordset.
        if (recordset is None) or (recordset.State == adc.adStateClosed):
            self.rs = None
            self.description = None
            return
        self.rs = recordset        #v2.1.1 bkline

        # The ADO documentation hints that obtaining the recordcount may be timeconsuming
        #   "If the Recordset object does not support approximate positioning, this property
        #    may be a significant drain on resources # [ekelund]
        # Therefore, COM will not return rowcount for server-side cursors. [Cole]
        # Client-side cursors (the default since v2.8) will force a static
        # cursor, and rowcount will then be set accurately [Cole]
        nOfFields=recordset.Fields.Count
        desc = []
        for i in range(nOfFields):
            f=getIndexedValue(recordset.Fields,i)
            if not(recordset.EOF or recordset.BOF):
                display_size=f.ActualSize #TODO: Is this the correct defintion according to the DB API 2 Spec ?
            else:
                display_size=None
            null_ok= bool(f.Attributes & adc.adFldMayBeNull)          #v2.1 Cole 
            desc.append((f.Name, f.Type, display_size, f.DefinedSize, f.Precision, f.NumericScale, null_ok))
        self.description = desc

    def close(self):
        """Close the cursor now (rather than whenever __del__ is called).
            The cursor will be unusable from this point forward; an Error (or subclass)
            exception will be raised if any operation is attempted with the cursor.
        """
        self.messages = []                
        self.connection = None    #this will make all future method calls on me throw an exception
        if self.rs and self.rs.State != adc.adStateClosed: # rs exists and is open      #v2.1 Rose
            self.rs.Close()                                                         #v2.1 Rose
            self.rs = None  #let go of the recordset so ADO will let it be disposed #v2.1 Rose

    def _new_command(self, command_text, command_type=adc.adCmdText):
        self.cmd = None
        self.command = command_text
        self.messages = []

        if self.connection is None:
            self._raiseCursorError(Error, None)
            return
        try:
            self.cmd = Dispatch("ADODB.Command")
            self.cmd.ActiveConnection = self.connection.adoConn
            self.cmd.CommandTimeout = self.connection.adoConn.CommandTimeout  #v2.1 Simons
            self.cmd.CommandType = command_type
        except:
            self._raiseCursorError(DatabaseError, None)

    def _execute_command(self):
        # Sprocs may have an integer return value
        self.return_value = None
        recordset = None; count = -1 #default value
        if verbose:
            print 'Executing command="%s"'%self.command
        try:
            # ----- the actual SQL is executed here ---
            if onIronPython:
                ra = Reference[int]()
                recordset = self.cmd.Execute(ra)
                count = ra.Value 
            else: #pywin32
                recordset, count = self.cmd.Execute()
            # ----- ------------------------------- ---
        except (Exception), e:
            _message = ""
            if hasattr(e, 'args'): _message += str(e.args)+"\n"
            _message += "Command:\n%s\nParameters:\n%s" %  (self.cmd.CommandText, format_parameters(self.cmd.Parameters, True))
            klass = self.connection._suggest_error_class()
            self._raiseCursorError(klass, _message)
        try:
            self.rowcount = recordset.RecordCount
        except:
            self.rowcount = count
        self._makeDescriptionFromRS(recordset)

    def callproc(self, procname, parameters=None):
        """Call a stored database procedure with the given name.
        The sequence of parameters must contain one entry for each
        argument that the sproc expects. The result of the
        call is returned as modified copy of the input
        sequence.  Input parameters are left untouched, output and
        input/output parameters replaced with possibly new values. 

        The sproc may also provide a result set as output,
        which is available through the standard .fetch*() methods.
        Extension: A "return_value" property may be set on the
        cursor if the sproc defines an integer return value.
        """
        self._new_command(procname, adc.adCmdStoredProc)
        self._buildADOparameterList(procname, parameters)
        self._execute_command()

        if parameters != None:
            retLst=[]
            for p in tuple(self.cmd.Parameters):
                if verbose > 2:
                    print 'returning=', p.Name, p.Type, p.Direction, repr(p.Value)
                pyObject=_convert_to_python(p.Value,variantConversions[p.Type])
                if p.Direction == adc.adParamReturnValue:
                    self.returnValue=pyObject
                else:
                    retLst.append(pyObject)
            return retLst

    def _reformat_operation(self,operation,parameters):
        if parameters:
            if self.paramstyle == 'format': # convert %s to ?
                operation = self._formatToQmark(operation)
            elif self.paramstyle == 'named':  # convert :name to ?
                operation, parameters = self._namedToQmark(operation,parameters)
        return operation,parameters

    def _buildADOparameterList(self, operation, parameters):
        self.parameters = parameters
        self.cmd.CommandText = operation
        if parameters != None:
            try: # attempt to use ADO's parameter list
                self.cmd.Parameters.Refresh()
                self.parameters_known = True
            except: # if it blows up
                self.parameters_known = False
            if not self.parameters_known:  #-- build own parameter list
                if verbose:
                    print('error in COM Refresh(), so adodbapi is building a parameter list')
                for i,elem in enumerate(parameters):
                    name='p%i' % i
                    adotype = pyTypeToADOType(elem)
                    p=self.cmd.CreateParameter(name,adotype) # Name, Type, Direction, Size, Value
                    if adotype in adoBinaryTypes:
                        p.Size = len(elem)
                    self.cmd.Parameters.Append(p)  
                if verbose > 2:
                    for i in range(self.cmd.Parameters.Count):
                        P = self.cmd.Parameters[i]
                        print 'adodbapi parameter attributes=', P.Name, P.Type, P.Direction, P.Size

            ##parameter_replacements = list()
            i = 0
            for value in parameters:
                p=getIndexedValue(self.cmd.Parameters,i)
                if p.Direction == adc.adParamReturnValue:
                    i += 1
                    p=getIndexedValue(self.cmd.Parameters,i)
                try:
                    _configure_parameter(p, value, self.parameters_known)
                except (Exception), e:
                    _message = u'Error Converting Parameter %s: %s, %s <- %s\n' %\
                             (p.Name, adc.ado_type_name(p.Type), p.Value, repr(value))
                    self._raiseCursorError(DataError, _message+'->'+repr(e.args))
                i += 1

    def execute(self, operation, parameters=None):
        """Prepare and execute a database operation (query or command).

            Parameters may be provided as sequence or mapping and will be bound to variables in the operation.
            Variables are specified in a database-specific notation
            (see the module's paramstyle attribute for details). [5] 
            A reference to the operation will be retained by the cursor.
            If the same operation object is passed in again, then the cursor
            can optimize its behavior. This is most effective for algorithms
            where the same operation is used, but different parameters are bound to it (many times). 

            For maximum efficiency when reusing an operation, it is best to use
            the setinputsizes() method to specify the parameter types and sizes ahead of time.
            It is legal for a parameter to not match the predefined information;
            the implementation should compensate, possibly with a loss of efficiency. 

            The parameters may also be specified as list of tuples to e.g. insert multiple rows in
            a single operation, but this kind of usage is depreciated: executemany() should be used instead. 

            Return value is not defined.

            [5] The module will use the __getitem__ method of the parameters object to map either positions
            (integers) or names (strings) to parameter values. This allows for both sequences and mappings
            to be used as input. 
            The term "bound" refers to the process of binding an input value to a database execution buffer.
            In practical terms, this means that the input value is directly used as a value in the operation.
            The client should not be required to "escape" the value so that it can be used -- the value
            should be equal to the actual database value. """
        self._new_command(operation)
        if self.paramstyle != 'qmark':
            operation,parameters = self._reformat_operation(operation,parameters)
        self._buildADOparameterList(operation,parameters)
        self._execute_command()

    def executemany(self, operation, seq_of_parameters):
        """Prepare a database operation (query or command)
        and then execute it against all parameter sequences or mappings found in the sequence seq_of_parameters.

            Return values are not defined. 
        """
        self.messages = list()                
        total_recordcount = 0

        for params in seq_of_parameters:
            self.execute(operation, params)

            if self.rowcount == -1:
                total_recordcount = -1

            if total_recordcount != -1:
                total_recordcount += self.rowcount

        self.rowcount = total_recordcount

    def _fetch(self, limit=None):
        """Fetch rows from the current recordset.

        limit -- Number of rows to fetch, or None (default) to fetch all rows.
        """
        if self.connection is None or self.rs is None:
            self._raiseCursorError(Error, None)
            return

        if self.rs.State == adc.adStateClosed or self.rs.BOF or self.rs.EOF:
            return list()

        if limit: # limit number of rows retrieved
            ado_results = self.rs.GetRows(limit)
        else:    # get all rows
            ado_results = self.rs.GetRows()
        if onIronPython:  # result of GetRows is a two-dimension array
            length = len(ado_results)//len(self.description) # length of first dimension
        else: #pywin32
            length = len(ado_results[0]) #result of GetRows is tuples in a tuple
        fetchObject = _SQLrows(ado_results,length,self) # new object to hold the results of the fetch
        return fetchObject

    def fetchone(self):
        """ Fetch the next row of a query result set, returning a single sequence,
            or None when no more data is available.

            An Error (or subclass) exception is raised if the previous call to executeXXX()
            did not produce any result set or no call was issued yet. 
        """
        self.messages = []                
        result = self._fetch(1)
        if result: # return record (not list of records)
            return result[0]
        return None


    def fetchmany(self, size=None):
        """Fetch the next set of rows of a query result, returning a list of tuples. An empty sequence is returned when no more rows are available.

        The number of rows to fetch per call is specified by the parameter.
        If it is not given, the cursor's arraysize determines the number of rows to be fetched.
        The method should try to fetch as many rows as indicated by the size parameter.
        If this is not possible due to the specified number of rows not being available,
        fewer rows may be returned. 

        An Error (or subclass) exception is raised if the previous call to executeXXX()
        did not produce any result set or no call was issued yet. 

        Note there are performance considerations involved with the size parameter.
        For optimal performance, it is usually best to use the arraysize attribute.
        If the size parameter is used, then it is best for it to retain the same value from
        one fetchmany() call to the next. 
        """
        self.messages=[]                
        if size is None:
            size = self.arraysize
        return self._fetch(size)

    def fetchall(self):
        """Fetch all (remaining) rows of a query result, returning them as a sequence of sequences (e.g. a list of tuples).

            Note that the cursor's arraysize attribute
            can affect the performance of this operation. 
            An Error (or subclass) exception is raised if the previous call to executeXXX()
            did not produce any result set or no call was issued yet. 
        """
        self.messages=[]                
        return self._fetch()

    def nextset(self):
        """Skip to the next available recordset, discarding any remaining rows from the current recordset.

            If there are no more sets, the method returns None. Otherwise, it returns a true
            value and subsequent calls to the fetch methods will return rows from the next result set. 

            An Error (or subclass) exception is raised if the previous call to executeXXX()
            did not produce any result set or no call was issued yet.
        """
        self.messages=[]                
        if self.connection is None or self.rs is None:
            self._raiseCursorError(Error,None)
            return None

        if onIronPython:
            try:
                recordset = self.rs.NextRecordset()
            except TypeError:
                recordset = None
            except Error, exc:
                self._raiseCursorError(NotSupportedError, exc.args)
        else: #pywin32
            try:                                               #[begin 2.1 ekelund]
                rsTuple=self.rs.NextRecordset()                # 
            except pywintypes.com_error, exc:                  # return appropriate error
                self._raiseCursorError(NotSupportedError, exc.args)#[end 2.1 ekelund]
            recordset = rsTuple[0]
        if recordset is None:
            return None
        self._makeDescriptionFromRS(recordset)
        return True

    def setinputsizes(self,sizes):
        pass

    def setoutputsize(self, size, column=None):
        pass

    def _last_query(self):  # let the programmer see what query we actually used
        try:
            if self.parameters == None:
                ret = self.cmd.CommandText
            else:
                ret = "%s,parameters=%s" % (self.cmd.CommandText,repr(self.parameters))
        except:
            ret = None
        return ret
    query = property(_last_query, None, None,
                         "returns the last query executed")
    
 # # # # # ----- Type Objects and Constructors ----- # # # # #
#Many databases need to have the input in a particular format for binding to an operation's input parameters.
#For example, if an input is destined for a DATE column, then it must be bound to the database in a particular
#string format. Similar problems exist for "Row ID" columns or large binary items (e.g. blobs or RAW columns).
#This presents problems for Python since the parameters to the executeXXX() method are untyped.
#When the database module sees a Python string object, it doesn't know if it should be bound as a simple CHAR
#column, as a raw BINARY item, or as a DATE. 
#
#To overcome this problem, a module must provide the constructors defined below to create objects that can
#hold special values. When passed to the cursor methods, the module can then detect the proper type of
#the input parameter and bind it accordingly. 

#A Cursor Object's description attribute returns information about each of the result columns of a query.
#The type_code must compare equal to one of Type Objects defined below. Type Objects may be equal to more than
#one type code (e.g. DATETIME could be equal to the type codes for date, time and timestamp columns;
#see the Implementation Hints below for details). 

def Date(year,month,day):
    "This function constructs an object holding a date value. "
    return dateconverter.Date(year,month,day)

def Time(hour,minute,second):
    "This function constructs an object holding a time value. "
    return dateconverter.Time(hour,minute,second)

def Timestamp(year,month,day,hour,minute,second):
    "This function constructs an object holding a time stamp value. "
    return dateconverter.Timestamp(year,month,day,hour,minute,second)

def DateFromTicks(ticks):
    """This function constructs an object holding a date value from the given ticks value
    (number of seconds since the epoch; see the documentation of the standard Python time module for details). """
    return Date(*time.gmtime(ticks)[:3])

def TimeFromTicks(ticks):
    """This function constructs an object holding a time value from the given ticks value
    (number of seconds since the epoch; see the documentation of the standard Python time module for details). """
    return Time(*time.gmtime(ticks)[3:6])

def TimestampFromTicks(ticks):
    """This function constructs an object holding a time stamp value from the given
    ticks value (number of seconds since the epoch;
    see the documentation of the standard Python time module for details). """
    return Timestamp(*time.gmtime(ticks)[:6])

def Binary(aString):
    """This function constructs an object capable of holding a binary (long) string value. """
    return makeByteBuffer(aString)

#SQL NULL values are represented by the Python None singleton on input and output. 

#Note: Usage of Unix ticks for database interfacing can cause troubles because of the limited date range they cover. 

# ------- utilities for converting python data to ADO data
def pyTypeToADOType(d):
    tp=type(d)
    try:
        return typeMap[tp]
    except KeyError:
        if isinstance(d,datetime.datetime):
            return adc.adDBTimeStamp
        if isinstance(d,datetime.time):
            return adc.adDBTime
        if tp in dateconverter.types:
            return adc.adDate
        if isinstance(d,decimal.Decimal):
            return adc.adDecimal
    raise DataError('cannot convert "%s" (type=%s) to ADO'%(repr(d),tp))

class DBAPITypeObject(object):
    def __init__(self,valuesTuple):
        self.values = valuesTuple

    def __eq__(self,other):
        return other in self.values

    def __ne__(self, other):
        return other not in self.values

# define similar types for generic convertion routines
adoIntegerTypes=(adc.adInteger,adc.adSmallInt,adc.adTinyInt,adc.adUnsignedInt,
                 adc.adUnsignedSmallInt,adc.adUnsignedTinyInt,
                 adc.adBoolean,adc.adError) #max 32 bits
adoRowIdTypes=(adc.adChapter,)          #v2.1 Rose
adoLongTypes=(adc.adBigInt,adc.adFileTime,adc.adUnsignedBigInt)
adoExactNumericTypes=(adc.adDecimal,adc.adNumeric,adc.adVarNumeric,adc.adCurrency)      #v2.3 Cole     
adoApproximateNumericTypes=(adc.adDouble,adc.adSingle)                          #v2.1 Cole
adoStringTypes=(adc.adBSTR,adc.adChar,adc.adLongVarChar,adc.adLongVarWChar,
                adc.adVarChar,adc.adVarWChar,adc.adWChar,adc.adGUID)
adoBinaryTypes=(adc.adBinary,adc.adLongVarBinary,adc.adVarBinary)
adoDateTimeTypes=(adc.adDBTime, adc.adDBTimeStamp, adc.adDate, adc.adDBDate)            
adoRemainingTypes=(adc.adEmpty,adc.adIDispatch,adc.adIUnknown,
                   adc.adPropVariant,adc.adArray,adc.adUserDefined,
                   adc.adVariant)

"""This type object is used to describe columns in a database that are string-based (e.g. CHAR). """
STRING   = DBAPITypeObject(adoStringTypes)

"""This type object is used to describe (long) binary columns in a database (e.g. LONG, RAW, BLOBs). """
BINARY   = DBAPITypeObject(adoBinaryTypes)

"""This type object is used to describe numeric columns in a database. """
NUMBER   = DBAPITypeObject(adoIntegerTypes + adoLongTypes + \
                           adoExactNumericTypes + adoApproximateNumericTypes)

"""This type object is used to describe date/time columns in a database. """

DATETIME = DBAPITypeObject(adoDateTimeTypes)
"""This type object is used to describe the "Row ID" column in a database. """
ROWID    = DBAPITypeObject(adoRowIdTypes)

typeMap= { memoryViewType: adc.adVarBinary,
           float: adc.adDouble,
           type(None): adc.adEmpty,
           unicode: adc.adBSTR, # this line will be altered by 2to3 to 'str:'
           bool:adc.adBoolean          #v2.1 Cole
           }
if longType != int: #not Python 3
    typeMap[longType] = adc.adBigInt  #works in python 2.x
    typeMap[int] = adc.adInteger
    typeMap[bytes] = adc.adBSTR     # 2.x string type
else:             #python 3.0 integrated integers
    ## Should this differentiote between an int that fits in a long and one that requires 64 bit datatype?
    typeMap[int] = adc.adBigInt
    typeMap[bytes] = adc.adVarBinary

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# functions to convert database values to Python objects

# variant type : function converting variant to Python value
def variantConvertDate(v):
    return dateconverter.DateObjectFromCOMDate(v)

def cvtString(variant):  # use to get old action of adodbapi v1 if desired
    if onIronPython:
        try: 
            return variant.ToString()
        except:
            pass
    return str(variant)

def cvtDecimal(variant): #better name
    return _convertNumberWithCulture(variant, decimal.Decimal)
def cvtNumeric(variant): #older name - don't break old code
    return cvtDecimal(variant)

def cvtFloat(variant):
    return _convertNumberWithCulture(variant, float)

def _convertNumberWithCulture(variant, f):
    try:
        return f(variant)
    except (ValueError,TypeError,decimal.InvalidOperation):
        try:
            europeVsUS = str(variant).replace(",",".")
            return f(europeVsUS)
        except (ValueError,TypeError,decimal.InvalidOperation): pass

def cvtInt(variant):
    return int(variant)

def cvtLong(variant):  # only important in old versions where long and int differ
    return long(variant)

def cvtBuffer(variant):
    return makeByteBuffer(variant)

def cvtUnicode(variant):
    return unicode(variant) # will be altered by 2to3 to 'str(variant)'

def identity(x): return x

class VariantConversionMap(dict): #builds a dictionary from {[list,of,keys]function}
    #useful for defining conversion functions for groups of similar data types.
    def __init__(self, aDict):
        for k, v in aDict.items():
            self[k] = v # we must call __setitem__
    def __setitem__(self, adoType, cvtFn):
        "don't make adoType a string :-)"
        try: # user passed us a tuple, set them individually
            for type in adoType:
                dict.__setitem__(self, type, cvtFn)
        except TypeError: # single value
            dict.__setitem__(self, adoType, cvtFn)
    def __getitem__(self, fromType):
        try:
            return dict.__getitem__(self, fromType)
        except KeyError:
            return identity

##def _convert_to_python(variant, adType):
##    if verbose > 3:
##        print 'Converting type_code=%s, val=%s'%(adType,repr(variant))
##        print 'conversion function=',repr(variantConversions[adType])
##        print '                     output=%s'%repr(variantConversions[adType](variant))
##    if isinstance(variant,DBNull):
##        return None
##    return variantConversions[adType](variant)

#initialize variantConversions dictionary used to convert SQL to Python
# this is the dictionary of default convertion functions, built by the class above.
variantConversions = VariantConversionMap( {
    adoDateTimeTypes : variantConvertDate,
    adoApproximateNumericTypes: cvtFloat,
    adoExactNumericTypes: cvtDecimal, # use to force decimal rather than unicode
    adoLongTypes : cvtLong,
    adoIntegerTypes: cvtInt,
    adoRowIdTypes: cvtInt,
    adoStringTypes: identity,
    adoBinaryTypes: cvtBuffer,
    adoRemainingTypes: identity })

if __name__ == '__main__':
    raise ProgrammingError(version + ' cannot be run as a main program.')

