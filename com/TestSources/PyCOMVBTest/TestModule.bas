Attribute VB_Name = "TestModule"
Option Explicit

Sub Main()
Dim o As Tester
Dim c As Collection
Dim n
    Set o = New Tester
    Set c = o.CollectionProperty
    Set n = c
    c(1) = "New Value"
    Debug.Print "c[1] = ", c(1)
End Sub
