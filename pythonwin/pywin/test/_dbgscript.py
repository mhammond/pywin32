# -*- coding: latin-1 -*-
#
# Script for testing pywin.debugger & interactive exec from test_pywin

# Umlauts for encoding test: áéúäöü

aa = 11
aa = 22


class CC:
    cc = 44


def ff(bb=55):
    global aa
    aa = 77
    return aa + bb


ff()
aa = 33
