function print(msg)
{
  WScript.Echo(msg) ;
}

var thisScriptEngine = ScriptEngine() ;

var majorVersion = ScriptEngineMajorVersion() ;
var minorVersion = ScriptEngineMinorVersion() ;
var buildVersion = ScriptEngineBuildVersion() ;

WScript.Echo(thisScriptEngine + " Version " + majorVersion + "." + minorVersion + " Build " + buildVersion) ;

var scriptlet = new  ActiveXObject("TestPys.Scriptlet") ;

print("Getting PyProp1");
var m = scriptlet.PyProp1 ;
print("PyProp1 = " + m) ;
m = scriptlet.PyProp2 ;
print("PyProp2 = " + m) ;

scriptlet.PyProp1=scriptlet.PyMethod1() ;
var m = scriptlet.PyProp1 ;
print("PyProp1 = " + m) ;

scriptlet.PyProp2=scriptlet.PyMethod2() ;
m = scriptlet.PyProp2 ;
print("PyProp2 = " + m) ;
