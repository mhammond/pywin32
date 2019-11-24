**As of the present time, the remote function is unsupported.
I have received no indication that anyone is actually using this functionality,
and have not created a test system for it. This may change in the future.**

### Remote Connection

Also available for the adodbapi package is a second db-api interface module,
which is used to obtain proxy access to a host ADO system. The intended
use of this is to run on a Linux computer allowing it to reach ADO data
sources, such as MS SQL Server databases or \"Jet\" (a.k.a. ACCESS) .mdb
data files.

It should take the same argument keywords as the host adodbapi server,
and will pass them to it. In addition, other connection keywords are
used to control the connection to the proxy server.

The extended code for this can be located in the `./remote` directory of the 
source code. It is not included in the pywin32 binaries.

\[Implementation note: adodbapi version 2.5 and 2.6 use PYRO4 for proxy
communication. The will probably change in the future to use ØMQ.\]

\-\-\-\-\-\-\-\--

keywords for remote connections:
- pyro_connection:
    'PYRO:ado.connection@%(proxy_host)s:%(proxy_port)s' #
    used to build the Pyro4 connection to the proxy. 
    You may never need to change the default.

- proxy_host: '::1' \# the address of the ADO proxy server. 
    (default = IPv6 localhost)

- proxy_port: '9099' # the IP port for the proxy connection.

To connect to the same database as above, assuming that the Windows box
running the proxy server (an SQLEXPRESS server) was at IPv4 address
10.11.12.13, you would use something like:

```python
import adodbapi.remote as db

conn_args = {'host': r"\\SQLEXPRESS\ ",
    'database': "Northwind",
    'user': "guest",
    'password': "12345678"}

conn_args['connection_string'] = """Provider=SQLOLEDB.1;
    User ID=%(user)s; Password=%(password)s;
    Initial Catalog=%(database)s; Data Source= %(host)s"""

conn_args['proxy_host'] = '10.11.12.13'

myConn = db.connect(conn_args)
```

In other words, you only need to add the address of the proxy server to
whatever connection string you would have used at the server itself,
then connect using adodbapi.remote.connect() rather than
adodbapi.connect().

\-\--

**Some limitations:** Remote connections do not allow varientConversion
customization, nor customized error handlers.

\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--

### Proxy Server

The proxy server is a module for the adodbapi package. It can be run
from the command line using the \"-m\" switch. (\*) The host address and
port number can be passed on the command line or by environment
variables. (You should also set the environment variable
\"PYRO\_HMAC\_KEY\" to some unique string for your installation.) The
environment variables are \"PYRO\_HOST\" and \"PYRO\_PORT\". The command
line arguments are \"HOST=aa.bb.cc.dd\" \"PORT=nnn\". IPv6 addresses
will also work. The default is address ::0 and port 9099 (all IPv6
interfaces).

`C:\>python -m adodbapi.remote.server HOST=0.0.0.0`

ado.connection server running on uri=PYRO:ado.connection\@0.0.0.0:9099
