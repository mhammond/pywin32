# genil_con : Constants relating to genIL.

T_VOID = 'System.Void'
T_PYOBJECT = "Python.Builtins.types.PyObject"
T_PYOBJECT_ARRAY = "Python.Builtins.types.PyObject[]"
T_IPYTYPE = "Python.Builtins.types.IPyType"

T_PY_INT = 'py:int'
T_PY_STRING = 'py:string'
T_PY_NULL = 'py:null'
T_PY_LIST = "py:list"
T_PY_TUPLE = "py:tuple"
T_PY_DICT = "py:dict"
T_PY_CLASS = "py:class"
T_PY_NONE = "py:none"
T_PY_METHOD = "py:method"
T_PY_FUNCTION = "py:function"

T_COR_OBJECT = "System.Object"
T_COR_INT = "System.Int32"
T_COR_BOOL = "System.Boolean"
T_COR_TYPE = "System.Type"
T_COR_STRING = "System.String"
T_COR_CHAR = "System.Char"
T_COR_DOUBLE = "System.Double"

T_COR_NULL = "<cor object null ref>"
T_COR_INSTANCE = "<cor instance>" # special handling for self - no specific "type"

T_PY_ALL = [T_PYOBJECT, T_PY_INT,  T_PY_STRING, T_PY_NULL,       T_PY_LIST,
            T_PY_TUPLE,  T_PY_DICT, T_PY_CLASS,  T_PY_NONE,
            ]

# where the value might be
W_LOCAL = "<local>" # A local variable
W_GLOBAL = "<global>"

# did have lots more constants before we got makepy working.