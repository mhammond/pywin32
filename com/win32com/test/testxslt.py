import os
import tempfile

expected_output = "The jscript test worked.\nThe Python test worked"

def TestAll():
    output_name = tempfile.mktemp("-pycom-test")
    cmd = "cscript //nologo testxslt.js doesnt_matter.xml testxslt.xsl " + output_name
    pipe = os.popen(cmd)
    try:
        output = pipe.read()
        if pipe.close():
            print "*** WSH failed executing XSLT command %r" % (cmd,)
            print output
            return
        f=open(output_name)
        try:
            got = f.read()
            if got != expected_output:
                print "ERROR: XSLT expected output of %r" % (expected_output,)
                print "but got %r" % (got,)
        finally:
            f.close()
    finally:
        try:
            os.unlink(output_name)
        except os.error:
            pass
    print "The MS-XSLT test seemed to work."

if __name__=='__main__':
    TestAll()
