import os, webbrowser
qrf_name = os.path.abspath(os.path.join(os.path.dirname(__file__), 'docs', 'quick_reference.pdf'))
if os.path.exists(qrf_name):
    webbrowser.open("file:///" + qrf_name)
else:
    webbrowser.open("https://github.com/mhammond/pywin32/blob/master/adodbapi/docs/quick_reference.pdf")
