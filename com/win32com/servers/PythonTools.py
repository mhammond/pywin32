
import sys
import time

class Tools:
  _public_methods_ = [ 'reload', 'adddir', 'echo', 'sleep' ]

  def reload(self, module):
    if type(module) == type('') and sys.modules.has_key(module):
      reload(sys.modules[module])
      return "reload succeeded."
    return "no reload performed."

  def adddir(self, dir):
    if type(dir) == type(''):
      sys.path.append(dir)
    return str(sys.path)

  def echo(self, arg):
    return `arg`

  def sleep(self, t):
    time.sleep(t)


if __name__=='__main__':
	print "Registering COM server..."
	from win32com.server.register import RegisterServer
	RegisterServer("{06ce7630-1d81-11d0-ae37-c2fa70000000}",
                       "win32com.servers.PythonTools.Tools",
                       "Python Tools",
                       "Python.Tools",
                       "Python.Tools.1")
	print "Class registered."
