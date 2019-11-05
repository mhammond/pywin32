# Imported by pywin32.pth to bootstrap the pywin32 environment in "portable"
# environments or any other case where the post-install script isn't run.
#
# In short, there's a directory installed by pywin32 named 'pywin32_system32'
# with some important DLLs which need to be found by Python when some pywin32
# modules are imported.
# If Python has `os.add_dll_directory()`, we need to call it with this path.
# Otherwise, we add this path to PATH.
import os
import site
import sys

# The directory should be installed under site-packages.
for maybe in site.getsitepackages():
	pywin32_system32=os.path.join(maybe,"pywin32_system32")
	if os.path.isdir(pywin32_system32):
		if hasattr(os, "add_dll_directory"):
			os.add_dll_directory(pywin32_system32)
		else:
			os.environ["PATH"] = pywin32_system32 + ";" + os.environ["PATH"]

