import os
import tempfile
import unittest
from unittest import mock

import win32con
import win32serviceutil


class TestLocatePythonServiceExe(unittest.TestCase):
    def testCrossVolumeMoveAllowsCopy(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source_dir = os.path.join(temp_dir, "source")
            destination_dir = os.path.join(temp_dir, "destination")
            os.mkdir(source_dir)
            os.mkdir(destination_dir)

            source = os.path.join(source_dir, "pythonservice.exe")
            destination = os.path.join(destination_dir, "pythonservice.exe")

            with open(source, "wb") as f:
                f.write(b"pythonservice test")

            with open(destination, "wb") as f:
                f.write(b"pythonservice test")

            with (
                mock.patch.object(
                    win32serviceutil.win32service,
                    "__file__",
                    os.path.join(source_dir, "win32service.pyd"),
                ),
                mock.patch.object(
                    win32serviceutil.sys,
                    "exec_prefix",
                    destination_dir,
                ),
                mock.patch.object(
                    win32serviceutil.win32api,
                    "MoveFileEx",
                    return_value=None,
                ) as move_file_ex,
                mock.patch.object(
                    win32serviceutil.win32api,
                    "GetModuleFileName",
                    return_value=os.path.join(destination_dir, "python.dll"),
                ),
            ):
                result = win32serviceutil.LocatePythonServiceExe()

            self.assertEqual(result, destination)
            move_file_ex.assert_called_once_with(
                source,
                destination,
                win32con.MOVEFILE_REPLACE_EXISTING | win32con.MOVEFILE_COPY_ALLOWED,
            )


if __name__ == "__main__":
    unittest.main()
