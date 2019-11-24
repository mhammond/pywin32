"""adodbapi.server - A pyro server for remote adodbapi (from Linux)

Copyright (C) 2013 by Vernon Cole
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

DB-API 2.0 specification: http://www.python.org/dev/peps/pep-0249/

This module source should run correctly in CPython versions 2.6 and later,
or IronPython version 2.6 and later,
or, after running through 2to3.py, CPython 3.2 or later.
"""
from __future__ import print_function, absolute_import

__version__ = '2.6.2.0'
version = 'adodbapi.server v' + __version__

PYRO_HOST = '::0' # '::0' or '0.0.0.0' means any network
PYRO_PORT = 9099  # may be altered below for Python 3 based servers
PYRO_COMMTIMEOUT = 40 # to be larger than the default database timeout
SERVICE_NAME = "ado.connection"

import os
import sys
import time
import array
import datetime

# Pyro4 is required for server and remote operation --> https://pypi.python.org/pypi/Pyro4/
try:
    import Pyro4
except ImportError:
    print('* * * Sorry, server operation requires Pyro4. Please "pip import" it.')
    exit(11)
import adodbapi.apibase as api
import adodbapi
import adodbapi.process_connect_string

if sys.version[0] >= '3': #python 3.x
    makeByteBuffer = bytes
    _BaseException = Exception
    Binary = bytes
else:                   #python 2.x
    from exceptions import StandardError as _BaseException
    makeByteBuffer = buffer
    Binary = buffer
try:
    pyro_host = os.environ['PYRO_HOST']
except:
    pyro_host = PYRO_HOST
try:
    pyro_port = os.environ['PYRO_PORT']
except:
    pyro_port = PYRO_PORT

for arg in sys.argv[1:]:
    if arg.lower().startswith('host'):
        try:
            pyro_host = arg.split('=')[1]
        except _BaseException:
            raise TypeError('Must supply value for argument="%s"' % arg)

    if arg.lower().startswith('port'):
        try:
            pyro_port = int(arg.split('=')[1])
        except _BaseException:
            raise TypeError('Must supply numeric value for argument="%s"' % arg)

    if arg.lower().startswith('timeout'):
        try:
            PYRO_COMMTIMEOUT = int(arg.split('=')[1])
        except _BaseException:
            raise TypeError('Must supply numeric value for argument="%s"' % arg)

    if arg.lower().startswith('--verbose'):
        try:
            verbose = int(arg.split('=')[1])
        except _BaseException:
            raise TypeError('Must supply numeric value for argument="%s"' % arg)
        adodbapi.adodbapi.verbose = verbose
    else:
        verbose = False

print(adodbapi.adodbapi.version)
print(version)
Pyro4.config.DETAILED_TRACEBACK = True
Pyro4.config.COMMTIMEOUT = PYRO_COMMTIMEOUT
Pyro4.config.AUTOPROXY = False
Pyro4.config.SERVERTYPE = 'multiplex'
Pyro4.config.PREFER_IP_VERSION = 0  # allow system to prefer IPv6
Pyro4.config.SERIALIZERS_ACCEPTED = set(['serpent', 'pickle'])  # change when Py2.5 retired

connection_list = []
CONNECTION_TIMEOUT = datetime.timedelta(minutes=30)
CONNECTION_REMEMBER = datetime.timedelta(hours=3)
if '--debug' in sys.argv:
    CONNECTION_TIMEOUT = datetime.timedelta(minutes=10)
    CONNECTION_REMEMBER = datetime.timedelta(minutes=20)
HEARTBEAT_INTERVAL = CONNECTION_TIMEOUT / 10

KEEP_RUNNING = True  # global value which will kill server when set to False

def unfixpickle(x):
    """pickle barfs on buffer(x) so we pass as array.array(x) then restore to original form for .execute()"""
    if  x is None:
        return None
    if isinstance(x,dict):
        # for 'named' paramstyle user will pass a mapping
        newargs = {}
        for arg,val in x.items():
            if isinstance(arg, type(array.array('B'))):
                newargs[arg] = Binary(val)
            else:
                newargs[arg] = val
        return newargs
    # if not a mapping, then a sequence
    newargs = []
    for arg in x:
        if isinstance(arg, type(array.array('B'))):
            newargs.append(Binary(arg))
        else:
            newargs.append(arg)
    return newargs


class ServerConnection(object):
    def __init__(self):
        self.server_connection = None
        self.cursors = {}
        self.last_used = datetime.datetime.now()
        self.timed_out = False

    def _check_timeout(self):
        if self.timed_out:
            raise api.OperationalError('Remote Connection Timed Out')

    def build_cursor(self):
        "Return a new Cursor Object using the connection."
        self._check_timeout()
        lc = self.server_connection.cursor() # get a new real cursor
        self.cursors[lc.id] = lc
        return lc.id

    def close(self, remember=False):
        global connection_list
        for c in self.cursors.values()[:]:
            c.close()
        self.server_connection.close()
        self._pyroDaemon.unregister(self)
        if not remember:
            connection_list.remove(self)

    def connect(self, kwargs):
        global connection_list
        kw = adodbapi.process_connect_string.process([], kwargs, True)

        if verbose:
            print('%s trying to connect %s', (version, repr(kw)))
        # kwargs has been loaded with all the values we need
        try:
            conn = adodbapi.adodbapi.connect(kw)
            if verbose:
                print("result = %s", repr(conn))
            self.server_connection = conn
            connection_list.append(self)
            return True
        except api.Error, e:
            return e

    def commit(self):
        try:
            self.server_connection.commit()
        except api.Error, e:
            return str(e)

    def rollback(self):
        try:
            self.server_connection.rollback()
        except api.Error, e:
            return str(e)

    def get_table_names(self):
        return self.server_connection.get_table_names()

    def get_attribute_for_remote(self, item):
        self._check_timeout()
        if item == 'autocommit':
            item = '_autocommit'
        if item in ('paramstyle', 'messages', 'supportsTransactions', 'dbms_name', 'dbms_version',
                    'connection_string', 'timeout', '_autocommit'):
            return getattr(self.server_connection, item)
        raise AttributeError('No provision for remote access to attribute="%s"' % item)

    def send_attribute_to_host(self, name, value): # to change autocommit or paramstyle on host
        self._check_timeout()
        self.server_connection.__setattr__(name, value)

# # # # # #  following are cursor methods called by the remote (using the connection) with a cursor id "cid" # # #

    def crsr_execute(self, cid, operation, parameters=None):
        self._check_timeout()
        fp = unfixpickle(parameters)
        try:
            self.cursors[cid].execute(operation, fp)
        except api.Error, e:
            try: errorclass = self.server_connection.messages[0][0]
            except: errorclass = api.Error
            return errorclass, str(e) # the error class should have been stored by the standard error handler

    def crsr_prepare(self, cid, operation):
        self._check_timeout()
        self.cursors[cid].prepare(operation)

    def crsr_executemany(self, cid, operation, seq_of_parameters):
        self._check_timeout()
        sq = [unfixpickle(x) for x in seq_of_parameters]
        self.cursors[cid].executemany(operation, sq)

    def crsr_callproc(self, cid, procname, parameters=None):
        self._check_timeout()
        fp = unfixpickle(parameters)
        return self.cursors[cid].callproc(procname, fp)

    def crsr_fetchone(self, cid):
        self._check_timeout()
        r = self.cursors[cid].fetchone()
        if r is None:
            return None
        return r[:]

    def crsr_fetchmany(self, cid, size=None):
        self._check_timeout()
        rows = []
        for row in self.cursors[cid].fetchmany(size):
            r = row[:]
            rows.append(r)
        return rows

    def crsr_fetchall(self, cid):
        self._check_timeout()
        rows = []
        for row in self.cursors[cid].fetchall():
            rows.append(row[:])   #[item for item in row])
        return rows

    def crsr_get_rowcount(self, cid):
        return self.cursors[cid].rowcount

    def crsr_get_description(self, cid):
        return self.cursors[cid].description

    def crsr_get_columnNames(self, cid):
        return self.cursors[cid].columnNames

    def crsr_nextset(self, cid):
        r = self.cursors[cid].nextset()
        return r

    def crsr_close(self, cid):
        try:
            self.cursors[cid].close()
        except: pass
        del self.cursors[cid]

    def crsr_set_arraysize(self, cid, value):
        self.cursors[cid].arraysize = value

    def crsr_set_conversion(self, cid, index, value):
        self.cursors[cid].conversion[index] = value

    def crsr_set_paramstyle(self, cid, value):
        self.cursors[cid].paramstyle = value

    def crsr_get_attribute_for_remote(self, cid, item):
        if verbose > 3:
            print('remote %s asking for=%s' % (cid, item))
        self._check_timeout()
        r = getattr(self.cursors[cid], item) # use the built-in function
        if verbose > 3:
            print('server replying with=%s' % repr(r))
        return r

    def suicide(self):
        """shut down the server service"""
        global KEEP_RUNNING
        KEEP_RUNNING = False
        print('Shutdown request received')


class ConnectionDispatcher(object):
    def make_connection(self):
        new_connection = ServerConnection()
        pyro_uri = self._pyroDaemon.register(new_connection)
        return pyro_uri


class Heartbeat_Timer(object):
    def __init__(self, interval, work_function, tick_result_function):
        self.interval = interval
        self.last_tick = datetime.datetime.now()
        self.work_function = work_function
        self.tick_result_function = tick_result_function

    def tick(self):
        now = datetime.datetime.now()
        if now - self.last_tick > self.interval:
            self.last_tick = now
            self.work_function()
        return self.tick_result_function()

def heartbeat_timer_work():
        global connection_list
        now = datetime.datetime.now()
        for conn in connection_list[:]:  # step through a copy of the list
            if now - conn.last_used > CONNECTION_TIMEOUT:
                try:
                    if not conn.timed_out:
                        conn.timed_out = True
                        conn.close(remember=True)
                    else:
                        if now - conn.last_used > CONNECTION_REMEMBER:
                            connection_list.remove(conn)
                except:
                    pass

def still_running():
    return KEEP_RUNNING

heartbeat_timer = Heartbeat_Timer(HEARTBEAT_INTERVAL, heartbeat_timer_work, still_running)

def serve():
    service_name = SERVICE_NAME
    if "use_nameserver" in sys.argv:
        # advertise self using nameserver
        if pyro_host in ('::0', '0.0.0.0'):
            raise Warning('Use a specified IP address when using the nameserver')
        i = 10 # wait for nameserver to come up
        while i:
            i -= 1
            time.sleep(2)
            try:
                ns = Pyro4.naming.locateNS()
                break
            except  Pyro4.errors.PyroError:
                if i == 0:
                    print('..unable to find nameserver..')
                    sys.exit(1)
        ns_p = Pyro4.core.Proxy(ns._pyroUri)
        if ':' in pyro_host and pyro_host[0] != '[':
            ph = pyro_host.join(('[',']')) # but [] around bare IPv6 addresses
        else:
            ph = pyro_host
        uri = 'PYRO:{}@{}:{}'.format(service_name, ph, int(pyro_port))
        ns_p.register(SERVICE_NAME, uri)
        print('registered {} to nameserver as={}'.format(SERVICE_NAME, uri))
        print('')

    daemon = Pyro4.Daemon(host=pyro_host, port=int(pyro_port))
    uri = daemon.register(ConnectionDispatcher(), service_name)
    print("%s server running on uri=%s" % (service_name, uri))
    print("(call using HOST=nnn and PORT=nnn to change interface addresses)")
    print("(use ^C or <Ctrl-Break> to interrupt...)")

    while KEEP_RUNNING:
        try:
            daemon.requestLoop(heartbeat_timer.tick)
        except KeyboardInterrupt:
            break

if __name__ == '__main__':
    serve()
    for conn in connection_list:  # clean up when done
        try:
            conn.server_connection.close()
        except:
            pass
