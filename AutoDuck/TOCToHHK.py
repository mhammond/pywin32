import os
import os.path
import sys

"""
TOCToHHK.py

Converts an AutoDuck .IDX file into a HTML Help index file.
"""

def main():
    file = sys.argv[1]
    output = sys.argv[2]
    input = open(file, "r")
    out = open(output, "w")
    line = input.readline()
    out.write("""
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="Python AutoDuck TOCToHHK.py">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
""")
    while line != "":
        # chop line
        line = line[:-1]
        fields = line.split("\t")
        if "." in fields[1]:
            keyword = fields[1].split(".")[-1]
        else:
            keyword = fields[1]
        context = fields[0]
        if " " in context:
            context = context.replace(" ", "_")
        out.write("""    <LI><OBJECT type="text/sitemap">
        <param name="Keyword" value="%s">
        <param name="Name" value="%s">
        <param name="Local" value="%s.html">
        </OBJECT>
""" % (keyword, fields[1], context))
        line = input.readline()
    out.write("""
</UL>
</BODY></HTML>
""")
    
if __name__ == "__main__":
    main()
