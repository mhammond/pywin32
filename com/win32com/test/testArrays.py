# Originally contributed by Stefan Schukat as part of this arbitrary-sized
# arrays patch.
from win32com.client import gencache
import win32com.test.util
import unittest

ZeroD = 0
OneDEmpty = []
OneD  = [1,2,3]
TwoD = [
        [1,2,3],
        [1,2,3],
        [1,2,3]
       ]

TwoD1 = [
          [
            [1,2,3,5],
            [1,2,3],
            [1,2,3]
          ],
          [
            [1,2,3],
            [1,2,3],
            [1,2,3]
          ]
       ]

OneD1 = [
          [
            [1,2,3],
            [1,2,3],
            [1,2,3]
          ],
          [
             [1,2,3],
             [1,2,3]
          ]
         ]

OneD2 = [
          [1,2,3],
          [1,2,3,4,5],
          [
             [1,2,3,4,5],
             [1,2,3,4,5],
             [1,2,3,4,5]
          ]
         ]


ThreeD = [
          [
            [1,2,3],
            [1,2,3],
            [1,2,3]
          ],
          [
              [1,2,3],
              [1,2,3],
              [1,2,3]
          ]
          ]

FourD = [
          [
            [[1,2,3],[1,2,3],[1,2,3]],
            [[1,2,3],[1,2,3],[1,2,3]],
            [[1,2,3],[1,2,3],[1,2,3]]
          ],
          [
              [[1,2,3],[1,2,3],[1,2,3]],
              [[1,2,3],[1,2,3],[1,2,3]],
              [[1,2,3],[1,2,3],[1,2,3]]
          ]
          ]

def _normalize_array(a):
    if type(a) != type(()):
        return a
    ret = []
    for i in a:
        ret.append(_normalize_array(i))
    return ret

class ArrayTest(win32com.test.util.TestCase):
    def setUp(self):
        self.arr = gencache.EnsureDispatch("PyCOMTest.ArrayTest")
    def tearDown(self):
        self.arr = None
    def _doTest(self, array):
        self.arr.Array = array
        self.failUnlessEqual(_normalize_array(self.arr.Array), array)
    def testZeroD(self):
        self._doTest(ZeroD)
    def testOneDEmpty(self):
        self._doTest(OneDEmpty)
    def testOneD(self):
        self._doTest(OneD)
    def testTwoD(self):
        self._doTest(TwoD)
    def testThreeD(self):
        self._doTest(ThreeD)
    def testFourD(self):
        self._doTest(FourD)
    def testTwoD1(self):
        self._doTest(TwoD1)
    def testOneD1(self):
        self._doTest(OneD1)
    def testOneD2(self):
        self._doTest(OneD2)

if __name__ == "__main__":
    unittest.main()
