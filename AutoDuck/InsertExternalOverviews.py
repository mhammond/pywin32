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

def processFile(input, out, extLinksHTML, extTopicHTML, importantHTML):
  while 1:
    line = input.readline()
    if not line:
      break
    line = line.replace("<!--index:exlinks-->", extLinksHTML)
    line = line.replace("<!--index:extopics-->", extTopicHTML)
    line = line.replace("<!--index:eximportant-->", importantHTML)
    out.write(line + "\n")
    
def genHTML(doc):
  s = ""
  for cat in doc:
      s = s + "<H3>%s</H3>\n" % (cat.label,)
      dict = {}
      for item in  cat.overviewItems.items:
          dict[item.name] = item.href
      keys = list(dict.keys())
      keys.sort()
      for k in keys:
        s = s + '<LI><A HREF="html/%s">%s</A>\n' % (dict[k], k)
  return s

def genLinksHTML(links):
  s = ""
  for link in links:
    s = s + '<LI><A HREF="%s">%s</A>\n' % (link.href, link.name)
  return s

import document_object

def main():
  if len(sys.argv) != 2:
    print("Invalid args")
    sys.exit(1)
  file = sys.argv[1]
  input = open(file, "r")
  out = open(file + ".2", "w")
  doc = document_object.GetDocument()
  linksHTML = genLinksHTML(doc.links)
  extTopicHTML = genHTML(doc)
  importantHTML = genLinksHTML(doc.important)
  processFile(input, out, linksHTML, extTopicHTML, importantHTML)
  input.close()
  out.close()
  sCmd = 'del "%s"' % file
  os.unlink(file)
  os.rename(file+".2", file)

if __name__ == "__main__":
  main()
