import os
import sys
import string

"""
Replace: <!--index:extopics-->
With:    <LI><A HREF="<context>">Topic Name</A>
         <LI><A HREF="<context2>">Topic Name2</A>
Note: The replacement string must be on one line.
Usage:
      AdExtTopics.py htmlfile ext_overviewfile
"""

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
    assert not d.has_key(top.name), \
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


def processFile(input, out, extTopicHTML):
  while 1:
    line = input.readline()
    if not line:
      break
    line = string.replace(line, "<!--index:extopics-->", extTopicHTML)
    out.write(line + "\n")
    
def genHTML():
  s = ""
  d = g_dExtOverviewTopics
  keys = d.keys()
  keys.sort()
  for k in keys:
    s = s + '<LI><A HREF="%s">%s</A>\n' % (d[k].context, d[k].name)
  return s
    
def main():
  if len(sys.argv) > 2:
    overview = sys.argv[2]
    file = sys.argv[1]
    input = open(file, "r")
    out = open(file + ".2", "w")
    parseOverview(open(sys.argv[2], "r"))
    extTopicHTML = genHTML()
    processFile(input, out, extTopicHTML)
    input.close()
    out.close()
    sCmd = 'del "%s"' % file
    os.system(sCmd)
    sCmd = 'move "%s.2" "%s"' % (file, file)
    os.system(sCmd)
    
if __name__ == "__main__":
  main()
