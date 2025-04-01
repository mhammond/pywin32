"""
Test for long path support in win32file module.

This test verifies that pywin32 can handle paths longer than the traditional
MAX_PATH limit (260 characters) when the system is configured to support them.
"""

import os
import shutil
import sys
import tempfile
import unittest
import winreg

import win32file
from pywin32_testutil import TestSkipped, testmain

# Traditional MAX_PATH limit
MAX_PATH = 260

def is_long_path_enabled_in_registry():
    """Check if long path support is enabled in the Windows registry"""
    try:
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SYSTEM\CurrentControlSet\Control\FileSystem") as key:
            value, _ = winreg.QueryValueEx(key, "LongPathsEnabled")
            return value == 1
    except Exception:
        return False

def is_windows_10_1607_or_later():
    """Check if the system is Windows 10 version 1607 or later"""
    try:
        version = sys.getwindowsversion()
        # Windows 10 is version 10.0
        if version.major == 10 and version.minor == 0:
            # Build 14393 is version 1607
            return version.build >= 14393
        # Future Windows versions
        return version.major > 10
    except Exception:
        return False

def can_use_long_paths():
    """Check if the system can use long paths"""
    return is_windows_10_1607_or_later() and is_long_path_enabled_in_registry()

class TestLongPaths(unittest.TestCase):
    def setUp(self):
        # Skip tests if long path support is not enabled
        if not can_use_long_paths():
            raise TestSkipped("Long path support not enabled on this system")

        # Create a temporary directory for our tests
        self.temp_dir = tempfile.mkdtemp(prefix="pywin32_longpath_test_")

        # Create a path longer than MAX_PATH
        # We'll use a deep directory structure to exceed MAX_PATH
        self.long_dir_name = os.path.join(self.temp_dir, "long_" + "x" * 200)
        os.makedirs(self.long_dir_name, exist_ok=True)

        # Create a filename that will result in a path > MAX_PATH
        self.long_filename = "test_" + "y" * 100 + ".txt"
        self.long_path = os.path.join(self.long_dir_name, self.long_filename)

        # Verify our path is actually longer than MAX_PATH
        self.assertGreater(len(self.long_path), MAX_PATH)

    def tearDown(self):
        # Clean up our temporary directory
        try:
            shutil.rmtree(self.temp_dir)
        except Exception:
            pass


    def test_find_files(self):
        """Test finding files with long paths"""
        # Create multiple files in our long path directory
        for i in range(3):
            file_path = os.path.join(self.long_dir_name, f"find_test_{i}.txt")
            with open(file_path, "w") as f:
                f.write(f"Test content {i}")

        # Use FindFiles to list the directory
        files = win32file.FindFiles(os.path.join(self.long_dir_name, "*"))

        # We should have at least 3 files (plus possibly . and ..)
        self.assertGreaterEqual(len(files), 3)

        # Verify we can find our specific files
        file_names = [file[8] for file in files]  # Index 8 is the filename
        for i in range(3):
            self.assertIn(f"find_test_{i}.txt", file_names)

if __name__ == '__main__':
    testmain()
