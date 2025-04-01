"""
Test for long path support in win32file module.

This test verifies that pywin32 can handle paths longer than the traditional
MAX_PATH limit (260 characters) when the system is configured to support them.
"""

import os
import sys
import tempfile
import unittest
import shutil
import winreg

import win32api
import win32con
import win32file
import winerror
import pywintypes
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

    def test_create_file(self):
        """Test creating a file with a long path"""
        # Create a file with a long path
        handle = win32file.CreateFile(
            self.long_path,
            win32file.GENERIC_WRITE | win32file.GENERIC_READ,
            0,  # No sharing
            None,  # Default security
            win32con.CREATE_ALWAYS,
            win32con.FILE_ATTRIBUTE_NORMAL,
            None  # No template
        )
        
        # Write some data to the file
        test_data = b"Testing long path support in pywin32"
        win32file.WriteFile(handle, test_data)
        
        # Close the handle
        handle.Close()
        
        # Verify the file exists
        self.assertTrue(os.path.exists(self.long_path))
        
        # Now try to open it again
        handle = win32file.CreateFile(
            self.long_path,
            win32file.GENERIC_READ,
            0,  # No sharing
            None,  # Default security
            win32con.OPEN_EXISTING,
            win32con.FILE_ATTRIBUTE_NORMAL,
            None  # No template
        )
        
        # Read the data back
        hr, data = win32file.ReadFile(handle, len(test_data))
        handle.Close()
        
        # Verify the data
        self.assertEqual(data, test_data)

    def test_copy_file(self):
        """Test copying a file with a long path"""
        # Create a source file
        with open(self.long_path, "w") as f:
            f.write("Test content for copy operation")
        
        # Create a destination path that's also long
        dest_path = os.path.join(self.long_dir_name, "copy_" + "z" * 100 + ".txt")
        
        # Copy the file
        win32file.CopyFile(self.long_path, dest_path, False)
        
        # Verify the copy exists
        self.assertTrue(os.path.exists(dest_path))
        
        # Verify the content
        with open(dest_path, "r") as f:
            content = f.read()
        self.assertEqual(content, "Test content for copy operation")

    def test_move_file(self):
        """Test moving a file with a long path"""
        # Create a source file
        with open(self.long_path, "w") as f:
            f.write("Test content for move operation")
        
        # Create a destination path that's also long
        dest_path = os.path.join(self.long_dir_name, "moved_" + "z" * 100 + ".txt")
        
        # Move the file
        win32file.MoveFile(self.long_path, dest_path)
        
        # Verify the source no longer exists
        self.assertFalse(os.path.exists(self.long_path))
        
        # Verify the destination exists
        self.assertTrue(os.path.exists(dest_path))
        
        # Verify the content
        with open(dest_path, "r") as f:
            content = f.read()
        self.assertEqual(content, "Test content for move operation")

    def test_get_file_attributes(self):
        """Test getting attributes of a file with a long path"""
        # Create a file
        with open(self.long_path, "w") as f:
            f.write("Test content for attributes")
        
        # Get the attributes
        attrs = win32file.GetFileAttributes(self.long_path)
        
        # Verify we got attributes (not testing specific values)
        self.assertIsNotNone(attrs)
        
        # Set a specific attribute
        win32file.SetFileAttributes(self.long_path, win32con.FILE_ATTRIBUTE_READONLY)
        
        # Get the attributes again
        attrs = win32file.GetFileAttributes(self.long_path)
        
        # Verify the readonly attribute is set
        self.assertTrue(attrs & win32con.FILE_ATTRIBUTE_READONLY)
        
        # Reset attributes to allow cleanup
        win32file.SetFileAttributes(self.long_path, win32con.FILE_ATTRIBUTE_NORMAL)

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

    def test_get_full_path_name(self):
        """Test getting the full path name of a long path"""
        # Create a file
        with open(self.long_path, "w") as f:
            f.write("Test content")
        
        # Get the full path
        full_path = win32file.GetFullPathName(self.long_path)
        
        # Verify it's the same as our long path (case might differ)
        self.assertEqual(full_path.lower(), self.long_path.lower())

    def test_delete_file(self):
        """Test deleting a file with a long path"""
        # Create a file
        with open(self.long_path, "w") as f:
            f.write("Test content for deletion")
        
        # Verify it exists
        self.assertTrue(os.path.exists(self.long_path))
        
        # Delete it
        win32file.DeleteFile(self.long_path)
        
        # Verify it's gone
        self.assertFalse(os.path.exists(self.long_path))

    def test_create_directory(self):
        """Test creating a directory with a long path"""
        # Create a path for a new directory that's longer than MAX_PATH
        long_subdir = os.path.join(self.long_dir_name, "subdir_" + "a" * 100)
        
        # Create the directory
        win32file.CreateDirectory(long_subdir, None)
        
        # Verify it exists
        self.assertTrue(os.path.isdir(long_subdir))
        
        # Create a file in the new directory to verify it works
        test_file = os.path.join(long_subdir, "test.txt")
        with open(test_file, "w") as f:
            f.write("Test content in subdirectory")
        
        # Verify the file exists
        self.assertTrue(os.path.exists(test_file))

    def test_remove_directory(self):
        """Test removing a directory with a long path"""
        # Create a directory
        long_subdir = os.path.join(self.long_dir_name, "subdir_to_remove_" + "b" * 100)
        os.makedirs(long_subdir, exist_ok=True)
        
        # Verify it exists
        self.assertTrue(os.path.isdir(long_subdir))
        
        # Remove it
        win32file.RemoveDirectory(long_subdir)
        
        # Verify it's gone
        self.assertFalse(os.path.exists(long_subdir))

    def test_error_handling(self):
        """Test error handling with long paths"""
        # Try to open a non-existent file
        non_existent = os.path.join(self.long_dir_name, "non_existent_" + "c" * 100 + ".txt")
        
        # This should raise an exception
        with self.assertRaises(pywintypes.error) as context:
            win32file.CreateFile(
                non_existent,
                win32file.GENERIC_READ,
                0,
                None,
                win32con.OPEN_EXISTING,
                win32con.FILE_ATTRIBUTE_NORMAL,
                None
            )
        
        # Verify it's the expected error (file not found)
        self.assertEqual(context.exception.winerror, winerror.ERROR_FILE_NOT_FOUND)

    def test_unicode_long_path(self):
        """Test long paths with Unicode characters"""
        # Create a path with Unicode characters
        unicode_dir = os.path.join(self.long_dir_name, "unicode_测试_тест_" + "d" * 100)
        os.makedirs(unicode_dir, exist_ok=True)
        
        unicode_file = os.path.join(unicode_dir, "unicode_файл_文件_" + "e" * 50 + ".txt")
        
        # Create the file
        handle = win32file.CreateFile(
            unicode_file,
            win32file.GENERIC_WRITE,
            0,
            None,
            win32con.CREATE_ALWAYS,
            win32con.FILE_ATTRIBUTE_NORMAL,
            None
        )
        
        test_data = b"Unicode test content"
        win32file.WriteFile(handle, test_data)
        handle.Close()
        
        # Verify the file exists
        self.assertTrue(os.path.exists(unicode_file))
        
        # Read it back
        with open(unicode_file, "rb") as f:
            content = f.read()
        
        self.assertEqual(content, test_data)

if __name__ == '__main__':
    testmain()
