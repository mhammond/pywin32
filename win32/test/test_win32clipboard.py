# tests for win32gui
import unittest

import win32clipboard


class TestGetSetClipboardData(unittest.TestCase):
    def copyData(self, data, format_):
        win32clipboard.OpenClipboard()
        ret = None
        try:
            win32clipboard.SetClipboardData(format_, data)
            ret = win32clipboard.GetClipboardData(format_)
        finally:
            win32clipboard.CloseClipboard()
        return ret

    def copyText(self, data, format_):
        win32clipboard.OpenClipboard()
        ret = None
        try:
            win32clipboard.SetClipboardText(data, format_)
            ret = win32clipboard.GetClipboardData(format_)
        finally:
            win32clipboard.CloseClipboard()
        return ret

    def test_data(self):
        test_data = {
            "Dummy str": win32clipboard.CF_UNICODETEXT,
            b"Dummy bytes text": win32clipboard.CF_TEXT,
            b"Dummy\x00\xff bytes": win32clipboard.CF_DIB,
        }
        for data, fmt in test_data.items():
            self.assertEqual(data, self.copyData(data, fmt))
        test_data = {
            "Dummy str": (win32clipboard.CF_TEXT, win32clipboard.CF_DIB),
            b"Dummy\x00\xff bytes": (win32clipboard.CF_UNICODETEXT,),
        }
        for data, formats in test_data.items():
            for fmt in formats:
                self.assertNotEqual(data, self.copyData(data, fmt))

    def test_text(self):
        test_data = {
            "Dummy str": win32clipboard.CF_UNICODETEXT,
            b"Dummy bytes": win32clipboard.CF_TEXT,
        }
        for data, fmt in test_data.items():
            self.assertEqual(data, self.copyText(data, fmt))
            self.assertRaises(ValueError, self.copyText, data, win32clipboard.CF_DIB)
        s = "Dummy str"
        self.assertEqual(
            s.encode(), self.copyText(s, win32clipboard.CF_TEXT)
        )  # @TODO - cfati: Do we want this?

    def test_numeric_looking_data(self):
        # mhammond#2132: data that int() can parse must be copied, not used as a handle
        test_data = [
            ("201920", win32clipboard.CF_UNICODETEXT, "201920"),
            (" 7_000 ", win32clipboard.CF_UNICODETEXT, " 7_000 "),
            (b"531815", win32clipboard.CF_TEXT, b"531815"),
            (bytearray(b"42"), win32clipboard.CF_TEXT, b"42"),
            (memoryview(b"42"), win32clipboard.CF_TEXT, b"42"),
        ]
        for data, fmt, expected in test_data:
            self.assertEqual(expected, self.copyData(data, fmt))

    def test_text_from_buffer(self):
        # mhammond#2132: any buffer object sent as a text format gets the terminator,
        # not just bytes; bytearray and memoryview used to come back one byte short
        for data in (bytearray(b"Dummy bytearray"), memoryview(b"Dummy memoryview")):
            self.assertEqual(bytes(data), self.copyData(data, win32clipboard.CF_TEXT))


if __name__ == "__main__":
    unittest.main()
