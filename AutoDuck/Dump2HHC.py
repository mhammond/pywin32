import os
import sys
import pprint
import copy

"""
Dump2HHC.py

Converts an AutoDuck dump into an HTML Help Table Of Contents file.
TODOs:
Add support for merging in non-autoduck'd comments into HTML Help files.
"""

class category:
    def __init__(self, category_defn):
        self.category_defn = category_defn
        self.id = category_defn.id
        self.name = category_defn.label
        self.dump_file = category_defn.id + ".dump"
        self.modules = {}
        self.objects = {}
        self.overviewTopics = {}
        self.extOverviewTopics = {}
        self.constants = {}

    def process(self):
        d = self.extOverviewTopics
        for oi in self.category_defn.overviewItems.items:
            top = topic()
            top.name = oi.name
            top.context = "html/" + oi.href
            top.type = "topic"
            assert not top.name in d and not top.name in self.overviewTopics, \
               "Duplicate named topic detected: " + top.name
            d[top.name] = top

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
def TopicKey(a):
    return a.name

def parseCategories():
    # Sucks in an external category file.
    # format:
    # topicname\t<CHM path to HTML file>\n
    # <repeat ...>
    import document_object
    ret = []
    doc = document_object.GetDocument()
    for defn in doc:
        cat = category(defn)
        cat.process()
        ret.append(cat)
    return ret

def parseTopics(cat, input):
    # Sucks in a AutoDuck Dump file.
    # format:
    # topicname\tcontext\tTags:
    # \ttagname
    # \t\tfield1\tfield2\t......
    # repeat tag/field section until the next topicname\tcontext line.

    # tagnames we care about:
    lTags = ["module", "object", "topic", "const"]
    line = input.readline()
    if line == '':
        return
    # chop
    line = line[:-1]
    fields = line.split("\t")
    while len(fields) > 0:
        assert len(fields) == 3, fields
        top = topic()
        top.name = fields[0]
        top.context = fields[1] + ".html"
        line = input.readline()
        if line == '':
            raise ValueError("incomplete topic!")
        # chop
        line = line[:-1]
        fields = line.split("\t")
        assert len(fields) == 2
        assert len(fields[0]) == 0
        top.type = fields[1]
        if top.type not in lTags:
            # Skip the property fields line for module/object
            line = input.readline()
            line = line[:-1]
            fields = line.split("\t")
            assert len(fields[0]) == 0 and len(fields[1]) == 0
            if line == '':
                raise ValueError("incomplete topic!")
            # Loop over the rest of the properties,
            # and add them appropriately. :)
            line = input.readline()
            if line == '':
                return
            # chop
            line = line[:-1]
            fields = line.split("\t")
            while len(fields) > 0:
                if len(fields[0]) > 0:
                    break
                # and loop....
                line = input.readline()
                if line == '':
                    return
                # chop
                line = line[:-1]
                fields = line.split("\t")
        else:
            # add to modules or object
            if top.type == "module":
              d = cat.modules
            elif top.type == "object":
              d = cat.objects
            elif top.type == "topic":
              d = cat.overviewTopics
            elif top.type == "const":
              d = cat.constants
            else:
                raise RuntimeError("What is '%s'" % (top.type,))

            if top.name in d:
                print("Duplicate named %s detected: %s" % (top.type, top.name))

            # Skip the property fields line for module/object
            line = input.readline()
            line = line[:-1]
            fields = line.split("\t")
            assert len(fields[0]) == 0 and len(fields[1]) == 0, "%s, %s" %(fields, top.name)
            if line == '':
                raise ValueError("incomplete topic!")

            # Loop over the rest of the properties,
            # and add them appropriately. :)
            line = input.readline()
            if line == '':
                return
            # chop
            line = line[:-1]
            fields = line.split("\t")
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
                    raise ValueError("incomplete topic!")
                line = line[:-1]
                fields = line.split("\t")
                assert len(fields[0]) == 0 and len(fields[1]) == 0, fields
                if top2.type == "pymeth":
                    top2.name = fields[2]
                    top2.context = "%s__%s_meth.html" % (_urlescape(top.name), top2.name)
                elif top2.type == "prop":
                    top2.name = fields[3]
                    top2.context = "%s__%s_prop.html" % (_urlescape(top.name), top2.name)
                else:
                    # and loop....
                    line = input.readline()
                    if line == '':
                        return
                    # chop
                    line = line[:-1]
                    fields = line.split("\t")
                    continue
                # Add top2 into top
                top.contains.append(top2)

                # and loop....
                line = input.readline()
                if line == '':
                    return
                # chop
                line = line[:-1]
                fields = line.split("\t")
            d[top.name] = top

def _urlescape(name):
    """Escape the given name for inclusion in a URL.
    
    Escaping is done in the manner in which AutoDuck(?) seems to be doing
    it.
    """
    name = name.replace(' ', '_')\
               .replace('(', '.28')\
               .replace(')', '.29')
    return name

def _genCategoryHTMLFromDict(dict, output):
    keys = list(dict.keys())
    keys.sort()
    for key in keys:
        topic = dict[key]
        output.write('<LI><A HREF="%s">%s</A>\n' % (topic.context, topic.name))

def _genOneCategoryHTML(output_dir, cat, title, suffix, *dicts):
    # Overview
    fname = os.path.join(output_dir, cat.id + suffix + ".html")
    output = open(fname, "w")
    output.write("<HTML><TITLE>" + title + "</TITLE>\n")
    output.write("<BODY>\n")
    output.write("<H1>" + title + "</H1>\n")
    for dict in dicts:
        _genCategoryHTMLFromDict(dict, output)
    output.write("</BODY></HTML>\n")
    output.close()

def _genCategoryTopic(output_dir, cat, title):
    fname = os.path.join(output_dir, cat.id + ".html")
    output = open(fname, "w")
    output.write("<HTML><TITLE>" + title + "</TITLE>\n")
    output.write("<BODY>\n")
    output.write("<H1>" + title + "</H1>\n")
    for subtitle, suffix in ("Overviews", "_overview"), ("Modules", "_modules"), ("Objects", "_objects"):
        output.write('<LI><A HREF="%s%s.html">%s</A>\n' % (cat.id, suffix, subtitle))
    output.write("</BODY></HTML>\n")
    output.close()

def genCategoryHTML(output_dir, cats):
    for cat in cats:
        _genCategoryTopic(output_dir, cat, cat.name)
        _genOneCategoryHTML(output_dir, cat, "Overviews", "_overview", cat.extOverviewTopics, cat.overviewTopics)
        _genOneCategoryHTML(output_dir, cat, "Modules", "_modules", cat.modules)
        _genOneCategoryHTML(output_dir, cat, "Objects", "_objects", cat.objects)
        _genOneCategoryHTML(output_dir, cat, "Constants", "_constants", cat.constants)

def _genItemsFromDict(dict, cat, output, target, do_children = 1):
    CHM = "mk:@MSITStore:%s.chm::/" % target
    keys = list(dict.keys())
    keys.sort()
    for k in keys:
      context = dict[k].context
      name = dict[k].name
      output.write('''
        <LI> <OBJECT type="text/sitemap">
             <param name="Name" value="%(name)s">
             <param name="ImageNumber" value="1">
             <param name="Local" value="%(CHM)s%(context)s">
             </OBJECT>
      ''' % locals())
      if not do_children:
          continue
      if len(dict[k].contains) > 0:
        output.write("<UL>")
      containees = copy.copy(dict[k].contains)
      containees.sort(key=TopicKey)
      for m in containees:
        output.write('''
        <LI><OBJECT type="text/sitemap">
             <param name="Name" value="%s">
             <param name="ImageNumber" value="11">
             <param name="Local" value="%s%s">
            </OBJECT>''' % (m.name, CHM, m.context))
      if len(dict[k].contains) > 0:
        output.write('''
        </UL>''')

def genTOC(cats, output, title, target):
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
''' % locals())

    for cat in cats:
        cat_name = cat.name; cat_id = cat.id
        output.write('''\
            <LI> <OBJECT type="text/sitemap">
                 <param name="Name" value="%(cat_name)s">
                 <param name="ImageNumber" value="1">
                 <param name="Local" value="%(CHM)s%(cat_id)s.html">
                 </OBJECT>
            <UL>
        ''' % locals())
        # Next write the overviews for this category
        output.write('''\
                <LI> <OBJECT type="text/sitemap">
                     <param name="Name" value="Overviews">
                     <param name="ImageNumber" value="1">
                     <param name="Local" value="%(CHM)s%(cat_id)s_overview.html">
                     </OBJECT>
                <UL>
        ''' % locals())
        _genItemsFromDict(cat.overviewTopics, cat, output, target)
        _genItemsFromDict(cat.extOverviewTopics, cat, output, target)
        output.write('''
                </UL>''')
        # Modules
        output.write('''
                <LI> <OBJECT type="text/sitemap">
                    <param name="Name" value="Modules">
                    <param name="ImageNumber" value="1">
                    <param name="Local" value="%(CHM)s%(cat_id)s_modules.html">
                    </OBJECT>
                <UL>
''' % locals())
        _genItemsFromDict(cat.modules, cat, output, target)
        output.write('''
                </UL>''')
        # Objects
        output.write('''
                <LI> <OBJECT type="text/sitemap">
                    <param name="Name" value="Objects">
                    <param name="ImageNumber" value="1">
                    <param name="Local" value="%(CHM)s%(cat_id)s_objects.html">
                    </OBJECT>
                <UL>''' % locals())
        # Dont show 'children' for objects - params etc don't need their own child nodes!
        _genItemsFromDict(cat.objects, cat, output, target, do_children=0)
        output.write('''
                </UL>''')
        # Constants
        output.write('''
    <LI> <OBJECT type="text/sitemap">
         <param name="Name" value="Constants">
         <param name="ImageNumber" value="1">
         <param name="Local" value="%(CHM)s%(cat_id)s_constants.html">
         </OBJECT>
           <UL>
''' % locals())
        _genItemsFromDict(cat.constants, cat, output, target)
        output.write("""
           </UL>""")
        # Finish this category
        output.write('''
        </UL>''')
    
    # Finished dumping categories - finish up
    output.write('''
</UL>
</BODY></HTML>
''')

# Dump2HHC.py
# Usage:
#   Dump2HHC.py dirname output.hhc
#

def main():
    gen_dir = sys.argv[1]
    cats = parseCategories()
    for cat in cats:
        file = os.path.join(gen_dir, cat.dump_file)
        input = open(file, "r")
        parseTopics(cat, input)
        input.close()

    output = open(sys.argv[2], "w")
    genTOC(cats, output, sys.argv[3], sys.argv[4])
    genCategoryHTML(gen_dir, cats)
    #pprint.pprint(g_dModules["win32lz"].contains)
    #pprint.pprint(g_dObject["connection"].contains)

if __name__ == "__main__":
    main()
