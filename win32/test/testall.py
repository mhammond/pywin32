import sys, os
import unittest

def suite():
    # Loop over all .py files here, except me :)
    try:
        me = __file__
    except NameError:
        me = sys.argv[0]
    files = os.listdir(os.path.dirname(me))
    suite = unittest.TestSuite()
    for file in files:
        base, ext = os.path.splitext(file)
        if ext=='.py' and os.path.basename(me) != file:
            mod = __import__(base)
            if hasattr(mod, "suite"):
                test = mod.suite()
            else:
                test = unittest.defaultTestLoader.loadTestsFromModule(mod)
            suite.addTest(test)
    return suite

if __name__=='__main__':
    unittest.main(argv=sys.argv + ['suite'])
