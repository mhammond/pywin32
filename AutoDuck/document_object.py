import sys

from xml.sax import make_parser, handler

class categoryHandler(handler.ContentHandler):
    def __init__(self):
        self.document = None
        self.in_importants = False
    def startElement(self, name, attrs):
        if name=="document":
            self.document = Document(attrs)
        if name=="category":
            self.document.categories.append( Category(attrs) )
        elif name=="overviews":
            category = self.document.categories[-1]
            assert category.overviewItems is None, "category %r already has overviews" % (category,)
            category.overviewItems = OverviewItems(attrs)
        elif name=="item":
            item = Item(attrs)
            if self.in_importants:
                self.document.important.append(item)
            elif self.document.categories:
                category = self.document.categories[-1]
                category.overviewItems.items.append(item)
            else:
                self.document.links.append(item)
        elif name=="important":
            self.in_importants = True

    def endElement(self, name):
        if name=="important":
            self.in_importants = False
    def endDocument(self):
        pass

class Document:
    def __init__(self, attrs):
        self.__dict__.update(attrs)
        self.categories = []
        self.links = []
        self.important = []
    def __iter__(self):
        return iter(self.categories)
    
class Category:
    def __init__(self, attrs):
        self.__dict__.update(attrs)
        self.overviewItems = None

class OverviewItems:
    def __init__(self, attrs):
        self.__dict__.update(attrs)
        self.items = []
    def __iter__(self):
        return iter(self.items)

class Item:
    def __init__(self, attrs):
        self.__dict__.update(attrs)

def GetDocument(fname="pywin32-document.xml"):
    parser = make_parser()
    handler=categoryHandler()
    parser.setContentHandler(handler)
    parser.parse(fname)
    return handler.document

if __name__=='__main__':
    doc = GetDocument()
    print("Important Notes")
    for link in doc.important:
        print(" ", link.name, link.href)
    
    print("Doc links")
    for link in doc.links:
        print(" ", link.name, link.href)

    print("Doc categories")
    for c in doc:
        print(" ", c.id, c.label)
