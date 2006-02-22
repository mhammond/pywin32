# odbc test suite kindly contributed by Frank Millman.
import os
import unittest
import odbc
import tempfile

# We use the DAO ODBC driver
from win32com.client.gencache import EnsureDispatch
from win32com.client import constants

class TestSequenceFunctions(unittest.TestCase):
    def setUp(self):
        self.db_filename = os.path.join(tempfile.gettempdir(), "test_odbc.mdb")
        if os.path.isfile(self.db_filename):
            os.unlink(self.db_filename)

        # Create a brand-new database - what is the story with these?
        for suffix in (".36", ".35", ".30"):
            try:
                dbe = EnsureDispatch("DAO.DBEngine" + suffix)
                break
            except pythoncom.com_error:
                pass
        else:
            raise RuntimeError, "Can't find a DB engine"

        workspace = dbe.Workspaces(0)

        newdb = workspace.CreateDatabase(self.db_filename, 
                                         constants.dbLangGeneral,
                                         constants.dbEncrypt)

        newdb.Close()

        conn_str = "Driver={Microsoft Access Driver (*.mdb)};dbq=%s;Uid=;Pwd=;" \
                   % (self.db_filename,)
        self.conn = odbc.odbc(conn_str)
        # And we expect a 'users' table for these tests.
        self.cur = self.conn.cursor()
        self.assertEqual(self.cur.execute("create table users "
            "(userid varchar(5), username varchar(25))"),-1)

    def tearDown(self):
        self.cur.close()
        self.cur = None
        self.conn.close()
        self.conn = None
        os.unlink(self.db_filename)

    def test_insert_select(self, userid='Frank', username='Frank Millman'):
        self.assertEqual(self.cur.execute("insert into users \
            values (?,?)", [userid, username]),1)
        self.assertEqual(self.cur.execute("select * from users \
            where userid = ?", [userid.lower()]),0)
        self.assertEqual(self.cur.execute("select * from users \
            where username = ?", [username.lower()]),0)

    def test_insert_select_large(self):
        # hard-coded 256 limit in ODBC to trigger large value support
        self.test_insert_select(userid='Frank' * 200, username='Frank Millman' * 200)

    def test_insert_select_unicode(self, userid=u'Frank', username=u"Frank Millman"):
        self.assertEqual(self.cur.execute("insert into users \
            values (?,?)", [userid, username]),1)
        self.assertEqual(self.cur.execute("select * from users \
            where userid = ?", [userid.lower()]),0)
        self.assertEqual(self.cur.execute("select * from users \
            where username = ?", [username.lower()]),0)

    def test_insert_select_unicode_ext(self):
        userid = unicode("test-\xe0\xf2", "mbcs")
        username = unicode("test-\xe0\xf2 name", "mbcs")
        self.test_insert_select_unicode(userid, username)

if __name__ == '__main__':
    unittest.main()
