import unittest
from unittest import mock

import win32api
import win32con
import win32evtlog
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


class TestDefaultMessageDLL(unittest.TestCase):
    """win32evtlog is AddSourceToRegistry's default message DLL."""

    def setUp(self):
        self.handle = win32api.LoadLibraryEx(
            win32evtlog.__file__, 0, win32con.LOAD_LIBRARY_AS_DATAFILE
        )
        self.addCleanup(win32api.FreeLibrary, self.handle)

    def testMessageTableResourceType(self):
        # Not a user-defined type *named* "MESSAGETABLE" (as llvm-windres would emit)
        self.assertIn(win32con.RT_MESSAGETABLE, win32api.EnumResourceTypes(self.handle))

    def testFormatMessage(self):
        # Message 1 in win32evtlog_messages.mc is just "%1"
        message = win32api.FormatMessageW(
            win32con.FORMAT_MESSAGE_FROM_HMODULE, self.handle, 1, 0, ["hello"]
        )
        self.assertEqual(message.rstrip(), "hello")


if __name__ == "__main__":
    unittest.main()
