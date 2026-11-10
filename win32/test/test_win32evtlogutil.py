import unittest
from unittest import mock

import win32evtlogutil


class TestFeedEventLogRecords(unittest.TestCase):
    def setUp(self):
        self.handle = object()
        patcher = mock.patch.multiple(
            win32evtlogutil.win32evtlog,
            OpenEventLog=mock.DEFAULT,
            ReadEventLog=mock.DEFAULT,
            CloseEventLog=mock.DEFAULT,
        )
        self.mocks = patcher.start()
        self.addCleanup(patcher.stop)
        self.mocks["OpenEventLog"].return_value = self.handle

    def testFeederCalledForEachRecord(self):
        self.mocks["ReadEventLog"].side_effect = [["a", "b"], ["c"], []]
        feeder = mock.Mock()

        win32evtlogutil.FeedEventLogRecords(feeder, "machine", "System", 42)

        self.mocks["OpenEventLog"].assert_called_once_with("machine", "System")
        self.assertEqual(
            self.mocks["ReadEventLog"].call_args_list,
            [mock.call(self.handle, 42, 0)] * 3,
        )
        self.assertEqual(
            feeder.call_args_list,
            [mock.call("a"), mock.call("b"), mock.call("c")],
        )
        self.mocks["CloseEventLog"].assert_called_once_with(self.handle)

    def testLogClosedWhenFeederRaises(self):
        self.mocks["ReadEventLog"].side_effect = [["a"], []]
        feeder = mock.Mock(side_effect=RuntimeError)

        with self.assertRaises(RuntimeError):
            win32evtlogutil.FeedEventLogRecords(feeder)

        self.mocks["CloseEventLog"].assert_called_once_with(self.handle)


if __name__ == "__main__":
    unittest.main()
