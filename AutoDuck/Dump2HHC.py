import os
import string
import sys
import pprint
import copy

"""
Dump2HHC.py

Converts an AutoDuck dump into an HTML Help Table Of Contents file.
TODOs:
Add support for merging in non-autoduck'd comments into HTML Help files.
"""

g_dModules = {}
g_dObject = {}
g_dOverviewTopics = {}
g_dExtOverviewTopics = {}

class topic:
    def __init__(self):
        self.context = None
        self.name = None
        self.type = None
        self.contains = []
    def __str__(self):
        return str({ "context" : self.context, "name" : self.name, "contains" : self.contains })
    
    def __repr__(self):
        if len(self.contains) > 0:
            return repr({ "context" : self.context, "name" : self.name, "contains" : self.contains })
        else:
            return repr({ "context" : self.context, "name" : self.name})

def TopicCmp(a, b):
        if a.name == b.name:
            return 0
        elif a.name > b.name:
            return 1
        else:
            return -1
          
def parseOverview(input):
  # Sucks in an external overview file.
  # format:
  # topicname\t<CHM path to HTML file>\n
  # <repeat ...>
  line = input.readline()
  if line == '':
    return None
  # chop
  line = line[:-1]
  fields = string.split(line, "\t")
  while len(fields) > 0:
    assert len(fields) == 2, fields
    top = topic()
    top.name = fields[0]
    top.context = fields[1]
    top.type = "topic"
    d = g_dExtOverviewTopics
    assert not d.has_key(top.name) and not g_dOverviewTopics.has_key(top.name), \
           "Duplicate named topic detected: " + top.name
    d[top.name] = top

    # Loop...
    line = input.readline()
    if line == '':
      return
    # chop
    line = line[:-1]
    # split
    fields = string.split(line, "\t")
  
def parseTopics(input):
    # Sucks in a AutoDuck Dump file.
    # format:
    # topicname\tcontext\tTags:
    # \ttagname
    # \t\tfield1\tfield2\t......
    # repeat tag/field section until the next topicname\tcontext line.

    # tagnames we care about:
    lTags = ["module", "object", "topic"]
    line = input.readline()
    if line == '':
        return
    # chop
    line = line[:-1]
    fields = string.split(line, "\t")
    while len(fields) > 0:
        assert len(fields) == 3, fields
        top = topic()
        top.name = fields[0]
        top.context = fields[1]
        line = input.readline()
        if line == '':
            raise ValueError, "incomplete topic!"
        # chop
        line = line[:-1]
        fields = string.split(line, "\t")
        assert len(fields) == 2
        assert len(fields[0]) == 0
        top.type = fields[1]
        if top.type not in lTags:
            # Skip the property fields line for module/object
            line = input.readline()
            line = line[:-1]
            fields = string.split(line, "\t")
            assert len(fields[0]) == 0 and len(fields[1]) == 0
            if line == '':
                raise ValueError, "incomplete topic!"
            # Loop over the rest of the properties,
            # and add them appropriately. :)
            line = input.readline()
            if line == '':
                return
            # chop
            line = line[:-1]
            fields = string.split(line, "\t")
            while len(fields) > 0:
                if len(fields[0]) > 0:
                    break
                # and loop....
                line = input.readline()
                if line == '':
                    return
                # chop
                line = line[:-1]
                fields = string.split(line, "\t")
        else:
            # add to modules or object
            if top.type == "module":
              d = g_dModules
            elif top.type == "object":
              d = g_dObject
            elif top.type == "topic":
              d = g_dOverviewTopics

            assert not d.has_key(top.name), "Duplicate named module/object/topic detected: " + top.name

            # Skip the property fields line for module/object
            line = input.readline()
            line = line[:-1]
            fields = string.split(line, "\t")
            assert len(fields[0]) == 0 and len(fields[1]) == 0, "%s, %s" %(fields, top.name)
            if line == '':
                raise ValueError, "incomplete topic!"

            # Loop over the rest of the properties,
            # and add them appropriately. :)
            line = input.readline()
            if line == '':
                return
            # chop
            line = line[:-1]
            fields = string.split(line, "\t")
            while len(fields) > 0:
                if len(fields[0]) > 0:
                    break

                # Do real work here...
                assert len(fields[0]) == 0 and len(fields[1]) > 0, "Bogus fields: " + fields
                top2 = topic()
                top2.type = fields[1]

                # Read the property fields line
                line = input.readline()
                if line == '':
                    raise ValueError, "incomplete topic!"
                line = line[:-1]
                fields = string.split(line, "\t")
                assert len(fields[0]) == 0 and len(fields[1]) == 0, fields
                if top2.type == "pymeth":
                    top2.name = fields[2]
                    top2.context = "%s__%s_meth" % (top.name, top2.name)
                elif top2.type == "prop":
                    top2.name = fields[3]
                    top2.context = "%s__%s_prop" % (top.name, top2.name)
                else:
                    # and loop....
                    line = input.readline()
                    if line == '':
                        return
                    # chop
                    line = line[:-1]
                    fields = string.split(line, "\t")
                    continue
                # Add top2 into top
                top.contains.append(top2)

                # and loop....
                line = input.readline()
                if line == '':
                    return
                # chop
                line = line[:-1]
                fields = string.split(line, "\t")
            d[top.name] = top

def genTOC(output, title, target):
    CHM = "mk:@MSITStore:%s.chm::/" % target
    output.write('''
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<OBJECT type="text/site properties">
    <param name="ImageType" value="Folder">
</OBJECT>
<UL>
    <LI> <OBJECT type="text/sitemap">
        <param name="Name" value="%(title)s">
        <param name="ImageNumber" value="1">
        <param name="Local" value="%(CHM)s%(target)s.html">
        </OBJECT>
    <UL>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Overviews">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)soverviews.html">
         </OBJECT>
    <UL>
''' % {"title" : title, "target" : target, "CHM" : CHM})
    keys = g_dOverviewTopics.keys()
    keys.sort()
    for k in keys:
      context = g_dOverviewTopics[k].context
      output.write('''
        <LI> <OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="1">
             <param name="Local" value="%s%s.html">
             </OBJECT>
      ''' % (g_dOverviewTopics[k].name, CHM, g_dOverviewTopics[k].context))
      if len(g_dOverviewTopics[k].contains) > 0:
        output.write("<UL>")
      containees = copy.copy(g_dOverviewTopics[k].contains)
      containees.sort(TopicCmp)
      for m in containees:
        context = m.context
        output.write('''
        <LI><OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="11">
             <param name="Local" value="%s%s.html">
            </OBJECT>''' % (m.name, CHM, m.context))
      if len(g_dOverviewTopics[k].contains) > 0:
        output.write('''
        </UL>''')
    keys = g_dExtOverviewTopics.keys()
    keys.sort()
    for k in keys:
      context = g_dExtOverviewTopics[k].context
      output.write('''
        <LI> <OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="1">
             <param name="Local" value="%s%s">
             </OBJECT>
      ''' % (g_dExtOverviewTopics[k].name, CHM, g_dExtOverviewTopics[k].context))
      if len(g_dExtOverviewTopics[k].contains) > 0:
        output.write("<UL>")
      containees = copy.copy(g_dExtOverviewTopics[k].contains)
      containees.sort(TopicCmp)
      for m in containees:
        context = m.context
        output.write('''
        <LI><OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="11">
             <param name="Local" value="%s%s">
            </OBJECT>''' % (m.name, CHM, m.context))
      if len(g_dExtOverviewTopics[k].contains) > 0:
        output.write('''
        </UL>''')
    output.write('''
    </UL>
    <LI> <OBJECT type="text/sitemap">
        <param name="Name" value="Modules">
        <param name="ImageNumber" value="1">
        <param name="Local" value="%(CHM)smodules.html">
        </OBJECT>
    <UL>
''' % {"title" : title, "target" : target, "CHM" : CHM})
    keys = g_dModules.keys()
    keys.sort()
    for k in keys:
        context = g_dModules[k].context
        output.write('''
        <LI> <OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="1">
             <param name="Local" value="%s%s.html">
             </OBJECT>
        ''' % (g_dModules[k].name, CHM, g_dModules[k].context))
        if len(g_dModules[k].contains) > 0:
            output.write("<UL>")
        containees = copy.copy(g_dModules[k].contains)
        containees.sort(TopicCmp)
        for m in containees:
            context = m.context
            output.write('''
            <LI> <OBJECT type="text/sitemap">
                 <param name="Name" value="%s">
                 <param name="ImageNumber" value="11">
                 <param name="Local" value="%s%s.html">
                 </OBJECT>''' % (m.name, CHM, m.context))
        if len(g_dModules[k].contains) > 0:
            output.write('''
        </UL>''')
    output.write('''
    </UL>
    <LI> <OBJECT type="text/sitemap">
        <param name="Name" value="Objects">
        <param name="ImageNumber" value="1">
        <param name="Local" value="%sobjects.html">
        </OBJECT>
    <UL>''' % CHM)
    keys = g_dObject.keys()
    keys.sort()
    for k in keys:
        context = g_dObject[k].context
        output.write('''
        <LI> <OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="1">
             <param name="Local" value="%s%s.html">
             </OBJECT>
        ''' % (g_dObject[k].name, CHM, context))
        if len(g_dObject[k].contains) > 0:
            output.write("<UL>")
        containees = copy.copy(g_dObject[k].contains)
        containees.sort(TopicCmp)
        for m in containees:
            if m.type == "prop":
                context = g_dObject[k].context
            else:
                context = m.context
            output.write('''
            <LI> <OBJECT type="text/sitemap">
                 <param name="Name" value="%s">
                 <param name="Local" value="%s%s.html">
                 <param name="ImageNumber" value="11">
                 </OBJECT>''' % (m.name, CHM, context))
        if len(g_dObject[k].contains) > 0:
            output.write('''
        </UL>''')
    output.write('''
    </UL>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Constants">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)sconstants.html">
         </OBJECT>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Classes and class members">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)sclassesandcmember.html">
         </OBJECT>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Functions">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)sfunctions.html">
         </OBJECT>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Messages">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)smessages.html">
         </OBJECT>
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Structures and Enumerations">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)sstructsnenum.html">
         </OBJECT>
</UL>
</UL>
</BODY></HTML>
''' % { "CHM" : CHM})

# Dump2HHC.py
# Usage:
#   Dump2HHC.py autoduck.DUMP output.hhc
#               <CHM Title> <Generated CHM name without CHM ext.>
#               <non-autoduck overview list file>
#

def main():
    file = sys.argv[1]
    input = open(file, "r")
    parseTopics(input)
    del input
    output = open(sys.argv[2], "w")
    if len(sys.argv) > 5:
      # parse non-autoduck overview list file.
      parseOverview(open(sys.argv[5], "r"))
    genTOC(output, sys.argv[3], sys.argv[4])
    #pprint.pprint(g_dModules["win32lz"].contains)
    #pprint.pprint(g_dObject["connection"].contains)
    
    
if __name__ == "__main__":
    main()
