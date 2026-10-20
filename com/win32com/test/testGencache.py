# Tests for win32com.client.gencache, in particular the guard against
# returning a namespace package when gen_py/<clsid>/__init__.py is missing
# (see mhammond/pywin32#2813).

import os
import shutil
import sys
import tempfile
import unittest

import win32com
from win32com.client import gencache

# The Scripting.FileSystemObject typelib - part of Windows itself, so no
# particular application is needed for the integration test below.
FSO_TYPELIB_CLSID = "{420B2830-E718-11CF-893D-00A0C9054228}"


class TestGetModule(unittest.TestCase):
    def setUp(self):
        self.gen_dir = tempfile.mkdtemp()
        self._orig_gen_path = win32com.__gen_path__
        self._orig_gen_py_path = list(win32com.gen_py.__path__)
        self._orig_clsid_to_typelib = dict(gencache.clsidToTypelib)
        self._orig_version_redirects = dict(gencache.versionRedirectMap)
        win32com.__gen_path__ = self.gen_dir
        win32com.gen_py.__path__[0] = self.gen_dir

    def tearDown(self):
        win32com.__gen_path__ = self._orig_gen_path
        win32com.gen_py.__path__[:] = self._orig_gen_py_path
        gencache.clsidToTypelib.clear()
        gencache.clsidToTypelib.update(self._orig_clsid_to_typelib)
        gencache.versionRedirectMap.clear()
        gencache.versionRedirectMap.update(self._orig_version_redirects)
        # Remove anything imported under win32com.gen_py.<name>.
        for name in [
            m for m in sys.modules if m.startswith("win32com.gen_py.")
        ]:
            del sys.modules[name]
        shutil.rmtree(self.gen_dir, ignore_errors=True)

    def _clear_gen_py_modules(self):
        for name in [
            m for m in sys.modules if m.startswith("win32com.gen_py.")
        ]:
            del sys.modules[name]

    def test_regular_module(self):
        fname = "AAAAAAAA000000000000000000000000x0x1x0"
        with open(os.path.join(self.gen_dir, fname + ".py"), "w") as f:
            f.write("CLSIDToClassMap = {}\n")
        mod = gencache._GetModule(fname)
        self.assertIsNotNone(mod.__file__)
        self.assertIn("CLSIDToClassMap", mod.__dict__)

    def test_regular_package(self):
        # A demand-generated module is a package directory with __init__.py.
        fname = "AAAAAAAA000000000000000000000001x0x1x0"
        os.makedirs(os.path.join(self.gen_dir, fname))
        with open(os.path.join(self.gen_dir, fname, "__init__.py"), "w") as f:
            f.write("CLSIDToClassMap = {}\n")
        mod = gencache._GetModule(fname)
        self.assertIsNotNone(mod.__file__)
        self.assertIn("CLSIDToClassMap", mod.__dict__)

    def test_namespace_package_raises_and_is_evicted(self):
        # The directory exists but __init__.py is missing, so the import
        # succeeds as a PEP 420 namespace package.  _GetModule must not return
        # it: it has none of the generated attributes and would only fail
        # later with a confusing AttributeError (issue 2813).
        fname = "AAAAAAAA000000000000000000000002x0x1x0"
        os.makedirs(os.path.join(self.gen_dir, fname))
        with open(os.path.join(self.gen_dir, fname, "child.py"), "w") as f:
            f.write("# a generated child module\n")
        mod_name = "win32com.gen_py." + fname
        with self.assertRaises(ImportError) as ctx:
            gencache._GetModule(fname)
        self.assertIn("namespace package", str(ctx.exception))
        # The import succeeded, so the broken object is cached in
        # sys.modules - it must have been evicted, or any retry would get it
        # straight back from memory.
        self.assertNotIn(mod_name, sys.modules)

    def test_ensure_module_recovers_from_missing_init_py(self):
        # End to end: after __init__.py of a generated package goes missing,
        # EnsureModule must regenerate the module instead of dying with
        # "module 'win32com.gen_py.<clsid>' has no attribute 'CLSIDToClassMap'".
        import pythoncom

        try:
            tla = pythoncom.LoadRegTypeLib(FSO_TYPELIB_CLSID, 1, 0, 0).GetLibAttr()
        except pythoncom.com_error:
            self.skipTest("the Scripting typelib is not installed")
        info = (str(tla[0]), tla[1], tla[3], tla[4])

        mod = gencache.EnsureModule(*info, bForDemand=True)
        self.assertIsNotNone(mod)
        fname = gencache.GetGeneratedFileName(*info)
        pkg_dir = os.path.join(self.gen_dir, fname)
        self.assertTrue(
            os.path.isfile(os.path.join(pkg_dir, "__init__.py"))
        )

        # Break it the way %TEMP% cleanup, antivirus quarantine or a failed
        # regeneration do: remove only __init__.py, keep the child modules.
        os.unlink(os.path.join(pkg_dir, "__init__.py"))
        self._clear_gen_py_modules()

        mod = gencache.EnsureModule(*info, bForDemand=True)
        self.assertIsNotNone(mod)
        self.assertIsNotNone(mod.__file__)
        self.assertIn("CLSIDToClassMap", mod.__dict__)


if __name__ == "__main__":
    unittest.main()
