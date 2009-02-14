Project
-------
adodbapi

A Python DB-API 2.0 module that makes it easy to use Microsoft ADO 
for connecting with databases and other data sources
using either CPython or IronPython.

Home page: <http://sourceforge.net/projects/adodbapi>

Features:
* 100% DB-API 2.0 compliant. 
* Includes pyunit testcases that describes how to use the module.  
* Fully implemented in Python. 
* Licensed under the LGPL license, which means that it can be used freely even in commercial programs subject to certain restrictions. 
* Supports eGenix mxDateTime, Python 2.3 datetime module and Python time module.

Prerequisites:
* C Python 2.3 or higher
 and pywin32 (Mark Hammond's python for windows extensions.)
 Note: as of 2.1.1, adodbapi is included in pywin32 versions 211 and later. 
or
 Iron Python 2.0 or higher. 

NOTE: ...........
If you do not like the new default operation of returning Numeric columns as decimal.Decimal,
you can select other options by the user defined convertion feature. 
Try:
        adodbapi.adodbapi.variantConversions[adodbapi.adNumeric] = adodbapi.adodbapi.cvtString
or:
        adodbapi.adodbapi.variantConversions[adodbapi.adNumeric] = adodbapi.adodbapi.cvtFloat
or:
	adodbapi.adodbapi.variantConversions[adodbapi.adNumeric] = write_your_own_convertion_function
............
Whats new in version 2.2.6
1. Actually works in Python 3.0 (using pywin32 212.6) after running thru 2to3
2. RESTRICTION: Python Time (as opposed to datetime.datetime, which is the default) may return
     incorrect results. To avoid this problem, do not use adodbapi.pythonTimeConverter. 
3. The python time converter test has been loosened so that it will pass in any time zone.
4. Several improvements in the test routines, including alteration of dbapi20 for Python 3.0 compatibility.
   (Some requirements of PEP249 are incompatible with Python 3.0)

Whats new in version 2.2.5
1. Exception definition cleanups for for Python 3.0 readiness [Mark Hammond]
2. Remove depreciated pythoncom.MakeTime calls (now uses pywintypes.Time)
3. Change tests to default to local SQL server. 
4. Add an access-type database file for demo use.

Whats new in version 2.2.4
1. Ready for Python3? -- refactored so that 2to3 will inject very few errors, seems to be almost runnable in Pyk3.
2. Use new function getIndexedValue() to hide differences between IronPython and pywin32.

What happened to version 2.2.3?
   It was an attempt to be Python3 ready, but done wrong, so killed off.

whats new in version 2.2.2
1. Works with Iron Python 2.0RC1 (passes all tests except BINARY columns and old python time date conversion.)
2. Passes all dbapi20 tests (All errors and warnings are now defined for the connection object.)
3. Adds predefined conversion functions cvtBuffer and cvtUnicode if you want to use them.

Whats new in version 2.2.1
1. Bugfix for v2.1 item 6: but do not correct the string length if the output column is not a string.

Whats new in version 2.2
1. Runs on Iron Python 2.0b4 with a few restrictions. It will not handle Longs or BLOB correcly, 
   and has some date/time problems (using the "time" module as opposed to "datetime").
   Bug reports have been submitted to Iron Python, which, if fixed, will remove all restrictions.
2. More agressive at making sure to retrieve Numeric and Currency colums as decimal.Decimal.
3. Has a new conversion module. To make Numerics retrieve as strings, execute:
       adodbapi.variantConversions[adodbapi.adNumeric] = adodbapi.cvtString
4. Switch to new-style classes and eliminate string exceptions.
5. Lots of cleanup in the code and the unit test.
6. More agressive at determining .rowcount after an operation.

Whats new in version 2.1.1?
1. Bugfix so nextset() will work even if a rowset is empty [ Bob Kline ]
2. Bugfix to call CoInitailize() before calling Dispatch() [ Adam Vandenberg ]

Whats new in version 2.1?
1. Use of Decimal.decimal data type for currency and numeric data. [ Cole ]
2. add optional timeout parameter to the connect method i.e.: 
      adodbapi.connect(string,timeout=nuberOfSeconds)  [thanks to: Patrik Simons]
3. improved detection of underlying transaction support [ Thomas Albrecht ]
4. fixed record set closing bug and adoRowIDtype [ Erik Rose ]
5. client-side cursoring [ Erik Rose ]
6. correct string length for qmark parameter substitution [ Jevon a.k.a. Guybrush Threepwood ]
7. try-multiple-strategy loop replaced by other logic. [ Cole ]
8. correct error code raised when next recordset not available. [ ekelund ]
9. numerous changes in unit test (including test against mySQL)

Whats new in version 2.0.1?
Added missing __init__.py file so it installs correctly.

Whats new in version 2.0?
1. Improved performance through GetRows method.
2. Flexible date conversions. 
   Supports eGenix mxExtensions, python time module and python 2.3 datetime module.
3. More exact mappings of numeric datatypes. 
4. User defined conversions of datatypes through "plug-ins".
5. Improved testcases, not dependent on Northwind data.
6. Works with Microsoft Access currency datatype.
7. Improved DB-API 2.0 compliance.
8. rowcount property works on statements Not returning records.
9. "Optional" error handling extension, see DB-API 2.0 specification.

Installation
------------
This version will be installed as part of the win32 package.

Authors (up to version 2.0.1)
-------
Henrik Ekelund, 
Jim Abrams.
Bjorn Pettersen.

Author (version 2.1 and later)
-------
Vernon Cole

License
-------
LGPL, see http://www.opensource.org/licenses/lgpl-license.php

Documentation
-------------
Start with:
http://www.python.org/topics/database/DatabaseAPI-2.0.html
read adodbapi/test/db_print.py
and look at the test cases in adodbapi/test directory. 

Mailing lists
-------------
The adodbapi mailing lists have been deactivated. Submit comments to the 
pywin32 lists.


Relase history
--------------
2.2.4   Ready for 2to3 convertion. Refactor to be more readable. Added function getIndexedValue() for IPy 2.0.
2.2.3   (withdrawn)
2.2.2   Iron Python support complete. 
2.2.1   Bugfix for string truncation
2.2     Code cleanup. added feature: "adodbapi.variantConversions[adodbapi.adNumeric] = adodbapi.cvtString"
2.1.1	Bugfix to CoIninialize() and nextset()
2.1	Python 2.4 version
2.0     See what's new above.
1.0.1   Bug fix: Null values for numeric fields. Thanks to Tim Golden. 
1.0     the first release
