# A version of the ActiveScripting engine that enables rexec support
# This version supports hosting by IE - however, due to Python's
# rexec module being neither completely trusted nor private, it is
# *not* enabled by default.

import pythoncom
from win32com.axscript import axscript
import winerror
import pyscript

INTERFACE_USES_DISPEX = 0x00000004	# Object knows to use IDispatchEx
INTERFACE_USES_SECURITY_MANAGER = 0x00000008 # Object knows to use IInternetHostSecurityManager

class PyScriptRExec(pyscript.PyScript):
	# Setup the auto-registration stuff...
	_reg_verprogid_ = "Python.AXScript-rexec.2"
	_reg_progid_ = "Python" # Same ProgID as the standard engine.
#	_reg_policy_spec_ = default
	_reg_catids_ = [axscript.CATID_ActiveScript,axscript.CATID_ActiveScriptParse]
	_reg_desc_ = "Python ActiveX Scripting Engine (with rexec support)"
	_reg_clsid_ = "{69c2454b-efa2-455b-988c-c3651c4a2f69}"
	_reg_class_spec_ = "win32com.axscript.client.pyscript_rexec.PyScriptRExec"
	_reg_remove_keys_ = [(".pys",), ("pysFile",)]
	_reg_threading_ = "Apartment"

	def _GetSupportedInterfaceSafetyOptions(self):
		# print "**** calling", pyscript.PyScript._GetSupportedInterfaceSafetyOptions, "**->", pyscript.PyScript._GetSupportedInterfaceSafetyOptions(self)
		return INTERFACE_USES_DISPEX | \
		       INTERFACE_USES_SECURITY_MANAGER | \
		       axscript.INTERFACESAFE_FOR_UNTRUSTED_DATA | \
		       axscript.INTERFACESAFE_FOR_UNTRUSTED_CALLER

if __name__=='__main__':
	pyscript.Register(PyScriptRExec)
