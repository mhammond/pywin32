from __future__ import generators

# Some raw iter tests.  Some "high-level" iterator tests can be found in
# testvb.py and testOutlook.py
import sys
import unittest

from win32com.client.gencache import EnsureDispatch
import pythoncom

def yield_iter(iter):
    while 1:
        yield iter.next()

class _BaseTestCase(unittest.TestCase):
    def test_enumvariant_vb(self):
        ob, iter = self.iter_factory()
        num=0
        for v in iter:
            num += 1
        self.failUnless(num==self.expected_length, "didnt get the %d items (got %d)" % (self.expected_length, num))
    def test_yield(self):
        ob, i = self.iter_factory()
        num=0
        for v in yield_iter(iter(i)):
            num += 1
        self.failUnless(num==self.expected_length, "didnt get the %d items (got %d)" % (self.expected_length, num))

    def test_nonenum(self):
        try:
            for i in self.object:
                pass
            self.fail("Could iterate over a non-iterable object")
        except TypeError:
            pass # this is expected.
        self.assertRaises(TypeError, iter, self.object)
        self.assertRaises(AttributeError, getattr, self.object, "next")

class VBTestCase(_BaseTestCase):
    def setUp(self):
        def factory():
            # Our VB test harness exposes a property with IEnumVariant.
            ob = self.object.EnumerableCollectionProperty
            ob.Add(1)
            ob.Add("Two")
            ob.Add("3")
            # Get the raw IEnumVARIANT.
            invkind = pythoncom.DISPATCH_METHOD | pythoncom.DISPATCH_PROPERTYGET
            iter = ob._oleobj_.InvokeTypes(pythoncom.DISPID_NEWENUM,0,invkind,(13, 10),())
            return ob, iter.QueryInterface(pythoncom.IID_IEnumVARIANT)
        # We *need* generated dispatch semantics, so dynamic __getitem__ etc
        # don't get in the way of our tests.
        self.object = EnsureDispatch("PyCOMVBTest.Tester")
        self.iter_factory = factory
        self.expected_length = 3

    def tearDown(self):
        self.object = None

def suite():
    # We dont want our base class run
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(VBTestCase))
    return suite

if __name__=='__main__':
    unittest.main(argv=sys.argv + ['suite'])
