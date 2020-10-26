import string
import re

###$ event <<expand-word>>
###$ win <Alt-slash>
###$ unix <Alt-slash>


class AutoExpand:

    keydefs = {
        "<<expand-word>>": ["<Alt-slash>"],
    }

    unix_keydefs = {
        "<<expand-word>>": ["<Meta-slash>"],
    }

    menudefs = [
        (
            "edit",
            [
                ("E_xpand word", "<<expand-word>>"),
            ],
        ),
    ]

    wordchars = string.ascii_letters + string.digits + "_"

    def __init__(self, editwin):
        self.text = editwin.text
        self.text.wordlist = None  # XXX what is this?
        self.state = None

    def expand_word_event(self, event):
        curinsert = self.text.index("insert")
        curline = self.text.get("insert linestart", "insert lineend")
        if not self.state:
            words = self.getwords()
            index = 0
        else:
            words, index, insert, line = self.state
            if insert != curinsert or line != curline:
                words = self.getwords()
                index = 0
        if not words:
            self.text.bell()
            return "break"
        word = self.getprevword()
        self.text.delete("insert - %d chars" % len(word), "insert")
        newword = words[index]
        index = (index + 1) % len(words)
        if index == 0:
            self.text.bell()  # Warn we cycled around
        self.text.insert("insert", newword)
        curinsert = self.text.index("insert")
        curline = self.text.get("insert linestart", "insert lineend")
        self.state = words, index, curinsert, curline
        return "break"

    def getwords(self):
        word = self.getprevword()
        if not word:
            return []
        before = self.text.get("1.0", "insert wordstart")
        wbefore = re.findall(r"\b" + word + r"\w+\b", before)
        del before
        after = self.text.get("insert wordend", "end")
        wafter = re.findall(r"\b" + word + r"\w+\b", after)
        del after
        words = []
        dict = {}
        # search backwards through words before
        wbefore.reverse()
        for w in wbefore:
            if dict.get(w):
                continue
            words.append(w)
            dict[w] = w
        # search onwards through words after
        for w in wafter:
            if dict.get(w):
                continue
            words.append(w)
            dict[w] = w

        # add words from interactive context
        prevexpr = self.getprevword(
            self.wordchars + "."
        )  # faster may be: self.get_prevexpr()
        if "." in prevexpr:
            ppos = prevexpr.rfind(".")
            prevexpr, word = prevexpr[:ppos], prevexpr[ppos + 1 :]  ## .rsplit('.', 1)
        else:
            prevexpr, word = "", prevexpr
        ns = self.get_auto_namespace(prevexpr)  # dict or list
        if ns:
            for w in ns:  # sorted() ?
                if w.startswith(word) and w not in dict:
                    words.append(w)  # TODO: prepend when prevexpr found?
                    dict[w] = w

        words.append(word)  # fall back to word itself finally

        return words

    def getprevword(self, chars=wordchars):
        line = self.text.get("insert linestart", "insert")
        i = len(line)
        while i > 0 and line[i - 1] in chars:
            i = i - 1
        return line[i:]

    def get_auto_namespace(self, predot=""):
        # return namespace dict/list with potential expansion candidates
        from pywin.framework import scriptutils

        o = scriptutils.GetXNamespace(predot)
        if predot:
            return dir(o)
        else:
            nslist = list(o)
            nslist.sort()
            return nslist
