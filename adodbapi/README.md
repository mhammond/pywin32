<!-- markdownlint-disable MD003 MD031 -->
# adodbapi

A Python [DB-API 2.0 (PEP 249)](https://peps.python.org/pep-0249/) module that makes it easy to use Microsoft ADO for connecting with databases and other data sources using CPython.

Home page: <https://github.com/mhammond/pywin32/tree/main/adodbapi#readme>

Features
--------

* 100% [DB-API 2.0 (PEP 249)](https://peps.python.org/pep-0249/) compliant (including most extensions and recommendations).
* Includes [PyUnit](https://wiki.python.org/moin/PyUnit) testcases that describe how to use the module.
* Fully implemented in Python. -- runs in current versions of Python 3.
* Licensed under the LGPL license, which means that it can be used freely even in commercial programs subject to certain restrictions.
* The user can choose between paramstyles: `qmark`, `named`, `format`, `pyformat` or `dynamic`
* Supports data retrieval by column name e.g.:
  ```python
  for row in myCurser.execute("select name,age from students"):
      print("Student", row.name, "is", row.age, "years old.")
  ```
* Supports user-definable system-to-Python data conversion functions (selected by ADO data type, or by column)

Prerequisites
-------------

* Python 3.7 or higher

Installation
------------

* (CPython on Windows): Install [pywin32](https://pypi.org/project/pywin32/) (`python -m pip install pywin32`) which includes adodbapi.
* (IronPython on Windows): Refer to <https://sourceforge.net/projects/adodbapi/> instead.

NOTE
----

If you do not like the new default operation of returning `Numeric` columns as `decimal.Decimal`, you can select other options by the user defined conversion feature.

Try:

```python
adodbapi.apibase.variantConversions[adodbapi.ado_consts.adNumeric] = adodbapi.apibase.cvtString
```

or:

```python
adodbapi.apibase.variantConversions[adodbapi.ado_consts.adNumeric] = adodbapi.apibase.cvtFloat
```

or:

```python
adodbapi.apibase.variantConversions[adodbapi.ado_consts.adNumeric] = write_your_own_conversion_function
```

CHANGES
-------

notes for 2.6.2:

* The definitive source has been moved to <https://github.com/mhammond/pywin32/tree/main/adodbapi> .
* Remote has proven too hard to configure and test with Pyro4. I am moving it to unsupported status until I can change to a different connection method.

what's new in version 2.6

* A `cursor.prepare()` method and support for prepared SQL statements.
* Lots of refactoring, especially of the Remote and Server modules (still to be treated as Beta code).
* The quick start document `quick_reference.odt` will export as a nice-looking pdf.
* Added paramstyles `pyformat` and `dynamic`. If your `paramstyle` is `named` you _must_ pass a dictionary of parameters to your .execute() method. If your `paramstyle` is `format` `pyformat` or `dynamic`, you _may_ pass a dictionary of parameters -- provided your SQL operation string is formatted correctly.

what's new in version 2.5

* Remote module: (works on Linux!) allows a Windows computer to serve ADO databases via PyRO
* Server module: PyRO server for ADO.  Run using a command like
    ```shell
    python -m adodbapi.server
    ```
   (server has simple connection string macros: `is64bit`, `getuser`, `sql_provider`, `auto_security`)
* Brief documentation included.  See [adodbapi/examples](/adodbapi/examples) folder adodbapi.rtf
* New connection method `conn.get_table_names()` --> list of names of tables in database
* Vastly refactored. Data conversion things have been moved to the new `adodbapi.apibase` module.
* Many former module-level attributes are now class attributes. (Should be more thread-safe)
* Connection objects are now context managers for transactions and will commit or rollback.
* `Cursor` objects are context managers and will automatically close themselves.
* Autocommit can be switched on and off.
* Keyword and positional arguments on the `connect()` method work as documented in [PEP 249](https://peps.python.org/pep-0249/) .
* Keyword arguments from the `connect` call can be formatted into the connection string.
* New keyword arguments defined, such as: `autocommit`, `paramstyle`, `remote_proxy`, `remote_port`.

*** Breaking change:

* `variantConversion` lookups are simplified: the following will raise `KeyError`:
    ```python
    oldconverter=adodbapi.variantConversions[adodbapi.adoStringTypes]
    ```
    Refactor as:
    ```python
    oldconverter=adodbapi.variantConversions[adodbapi.adoStringTypes[0]]
    ```

License
-------

[LGPL-2.1](https://opensource.org/license/lgpl-2-1)

Documentation
-------------

Look at:

* [adodbapi/quick_reference.md](/adodbapi/quick_reference.md)
* <https://www.python.org/topics/database/DatabaseAPI-2.0.html>
* read the examples in [adodbapi/examples](/adodbapi/examples)
* and the test cases in [adodbapi/test](/adodbapi/test) directory.

Mailing lists
-------------

The adodbapi mailing lists have been deactivated. Submit comments to the
[python-win32 mailing list](https://mail.python.org/mailman/listinfo/python-win32) .

* the bug tracker on <https://sourceforge.net/projects/adodbapi/> may be checked, (infrequently).
* please use: <https://github.com/mhammond/pywin32/issues>
