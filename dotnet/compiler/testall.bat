rem @echo off
call make suite\test_misc
call make suite\test_builtins
call make suite\test_methods
call make suite\test_exceptions
call make suite\test_signatures
call make suite\com_imports
call make suite\pystone
call make suite\test_func
call make suite\test_cor_conversions

echo All tests done!