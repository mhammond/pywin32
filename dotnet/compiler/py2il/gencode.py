#!/usr/local/bin/python
#
# Copyright 1997-1999 Greg Stein and Bill Tutt
#
# Portions Copyright 1999-2000 Microsoft Corporation.
# Portions Copyright 1997-1999 Greg Stein and Bill Tutt.
#
# This source code may be freely distributed, as long as all
# copyright information remains in place.
#
# See http://www.ActiveState.com/.NET for the latest versions.


import sys, os
import string
import traceback
import linecache
import parser # for the error
import pythoncom # for the error
import options

def usage(extra = None):
    if extra:
        print "***", extra
        print
    print "gencode.py [/opt:val ...] source_file"
    print "Options:"
    for line in options.GetOptionDescriptions():
        print " " + line
    sys.exit(9)

def print_where(msg, compiler, filename):
    if compiler is not None and compiler.gen is not None:
        gen = compiler.gen
        print "Error: %s (%s) : %s" % (filename, gen.lineno, msg)
        print ">%s" % (linecache.getline(filename, gen.lineno),)
    else:
        print "Error: %s" % (msg,)

def profile(fn, *args):
    import profile
    prof = profile.Profile()
    try:
        apply(prof.runcall, (fn,) + args )
    except SystemExit:
        pass
    import pstats
    # Damn - really want to send this to Excel!
    #      width, list = pstats.Stats(prof).strip_dirs().get_print_list([])
    pstats.Stats(prof).strip_dirs().sort_stats("time").print_stats()

def compile_code(compiler, file_name):
    import genil
    options = compiler.options
    if options.source_file is None:
        options.source_file = file_name
    try: # Catch errors to be reported to user.
        try: # Catch errors so verbose level can show location in compiler.
            if options.profile_compiler:
                profile(compiler.createModule, file_name)
            else:
                compiler.createModule(file_name)
            compiler = None
            return 0
        except (EnvironmentError, KeyboardInterrupt, parser.ParserError):
            # IO Errors or KeyboardInterrupt exceptions dont get an "internal traceback"!
            raise
        except: # (StandardError, pythoncom.com_error):
            if options.verbose_level>1:
                print "Internal Traceback:"
                traceback.print_exc()
                print "----"
            raise

    # Non source-code related errors raised by the compiler
    except genil.emit_error, err:
        print_where("Error compiling file: " + err.msg, compiler, file_name)
    # Compiler (not syntax) errors that directly involve user source code.
    except genil.source_error, err:
        print_where(err.msg, compiler, file_name)
    # Syntax errors in the Python code.
    except parser.ParserError:
        # Use compile to get the specific details:
        try:
            compile(open(file_name).read(), file_name, "exec")
            # Shouldnt get here - expecting a syntax error!
            print "Some strange syntax error in source file", file_name
            print "Sorry - but I can't be more specific about the problem :-("
        except SyntaxError, details:
            if details.filename is None: details.filename = file_name
            print "Syntax error in %s, line %d" % (details.filename, details.lineno)
            print linecache.getline(details.filename, details.lineno)[:-1]
            print " " * (details.offset-1) + "^"
    # User interrupt.
    except KeyboardInterrupt:
        print "* Cancelled *"
    # General IO Error
    except EnvironmentError, exc:
        print "Error compiling -", exc
    # COM Exceptions
    except pythoncom.com_error, exc_info:
        hr, msg, exc, argerr = exc_info
        if exc: msg = exc[2]
        msg = "Internal compiler error: " + str(msg)
        print_where(msg, compiler, file_name)
    # Every other exception!
    except:
        t, v, tb = sys.exc_info()
        msg = "Internal compiler error: "
        msg = msg + string.join(traceback.format_exception_only(t, v), '\r\n')
        print_where(msg, compiler, file_name)
    return 1

def main():
    if len(sys.argv[1:])==0:
        usage()
    num_source = 0
    opts = options.Options()
    import genil
    compiler = genil.Compiler(opts)
    have_opt_since_source = 0
    for arg in sys.argv[1:]:
        if arg[0] in '-/':
            # Appears to be an option - set it.
            have_opt_since_source = 1
            arg = arg[1:]
            pos = string.find(arg, ':')
            if pos>0:
                opt = arg[:pos]
                val = arg[pos+1:]
            else:
                opt = arg
                val = None
            try:
                opts.SetOption(opt, val)
            except options.OptionsException, details:
                usage(str(details))
        else:
            have_opt_since_source = 0
            num_source = num_source + 1
            # It must be a source file.
            if opts.print_tree:
                ast = parser.suite(open(arg).read())
                tree = parser.ast2tuple(ast,1)
                gen_python(tree)
            elif opts.transformed_tree:
                from compiler import parseFile
                import pprint
                pprint.pprint(parseFile(arg).asList())
            else:
                # Do the real compile.
                err_no = compile_code(compiler, arg)
                if err_no:
                    compiler.finalize()
                    return err_no
                # Reset all the file-specific options.
                opts.ResetOptions('output-file', 'source-file', 'module-name')
    if num_source==0:
        usage("No source files specified!")
    if have_opt_since_source:
        print "WARNING: Compiler options specified after the last source file name have been ignored!"

    try:
        compiler.save()
        compiler.finalize()
    except EnvironmentError, exc:
        print "Error saving outfile file:", exc
        return 1
    return 0

if __name__=='__main__':
    rc = main()
    # The rest of this code drops all the COM objects, and ensures none are left alive.
    # There is no real problem with _not_ doing this, but it does ensure we are nice and clean
    # and can run OK in a long-living of embedded environment.
    import genil, funcsigs
    import pythoncom
    genil.__dict__.clear()
    funcsigs.__dict__.clear()
    if pythoncom._GetInterfaceCount():
        print "Warning - compiler terminating with with", pythoncom._GetInterfaceCount(), "COM objects still alive"
    sys.exit(rc)