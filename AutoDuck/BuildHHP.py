import os
import sys

"""
BuildHHP.py

Build HTML Help project file.
"""

sHHPFormat = """
[OPTIONS]
Binary TOC=Yes
Compatibility=1.1 or later
Compiled file=%(output)s.chm
Contents file=%(output)s.hhc
Default Window=Home
Default topic=%(target)s.HTML
Display compile progress=Yes
Full-text search=Yes
Index file=%(output)s.hhk
Language=0x409 English (United States)

[WINDOWS]
Home="%(target)s","%(target)s.hhc","%(target)s.hhk","%(target)s.HTML","%(target)s.HTML",,,,,0x63520,,0x387e,,,,,,2,,0


[FILES]
%(output)s.HTML

[INFOTYPES]
"""

def main():
    output = os.path.abspath(sys.argv[1])
    target = sys.argv[2]
    f = open(output + ".hhp", "w")
    f.write(sHHPFormat % { "output" : output, "target" : target })
    f.close()

if __name__ == "__main__":
    main()

