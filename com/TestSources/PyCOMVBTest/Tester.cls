VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Tester"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Ext_KEY = "SavedWithClassBuilder" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Option Explicit
'local variable(s) to hold property value(s)
Private mvarIntProperty As Integer 'local copy
Private mvarLongProperty As Long 'local copy
Private mvarVariantProperty As Variant 'local copy
Private mvarStringProperty As String 'local copy
Private mvarCollectionProperty As Collection 'local copy
'local variable(s) to hold property value(s)
Private mvarVariantPutref As Variant 'local copy
'local variable(s) to hold property value(s)
' I cant seem to declare properties as returning native arrays.
' I can only declare them as variant which hopefully holds an array.
Private mvarArrayProperty As Variant 'local copy

Public EnumerableCollectionProperty As New EnumerableCollection 'local copy

Public Type SubSubStruct
    int_val As Integer
    str_val As String
End Type

Public Type SubStruct
    int_val As Integer
    str_val As String
    array_val(3) As SubSubStruct
    ob_val As Object
End Type

Public Type VBStruct
    int_val As Integer
    str_val As String
    ob_val As Object
    sub_val As SubStruct
End Type

Dim mvarStructProperty As VBStruct
Dim mvarStructArrayProperty() As VBStruct

Public Function PassSAFEARRAY(arrayData() As String) As Variant
    PassSAFEARRAY = arrayData
End Function

Public Function PassSAFEARRAYVariant(arrayData() As Variant) As Variant
    PassSAFEARRAYVariant = arrayData
End Function

Public Property Let ArrayProperty(ByRef vData As Variant)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.ArrayProperty = 5
    mvarArrayProperty = vData
End Property


Public Property Get ArrayProperty() As Variant
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.ArrayProperty
    ArrayProperty = mvarArrayProperty
End Property


Public Property Set VariantPutref(ByVal vData As Variant)
'used when assigning an Object to the property, on the left side of a Set statement.
'Syntax: Set x.VariantPutref = Form1
    Set mvarVariantPutref = vData
End Property


Public Property Get VariantPutref() As Variant
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.VariantPutref
    If IsObject(mvarVariantPutref) Then
        Set VariantPutref = mvarVariantPutref
    Else
        VariantPutref = mvarVariantPutref
    End If
End Property

Public Function PassIntByRef(intVal As Integer) As Integer
    PassIntByRef = intVal
    intVal = intVal + 1
End Function
Public Function PassIntByVal(ByVal intVal As Integer) As Integer
    PassIntByVal = intVal + 1
End Function

Public Property Set CollectionProperty(ByVal vData As Object)
'used when assigning an Object to the property, on the left side of a Set statement.
'Syntax: Set x.CollectionProperty = Form1
    Set mvarCollectionProperty = vData
End Property


Public Property Get CollectionProperty() As Collection
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.CollectionProperty
    Set CollectionProperty = mvarCollectionProperty
End Property

Public Property Let StringProperty(ByVal vData As String)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.StringProperty = 5
    mvarStringProperty = vData
End Property


Public Property Get StringProperty() As String
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.StringProperty
    StringProperty = mvarStringProperty
End Property

Public Property Get StructProperty() As VBStruct
    StructProperty = mvarStructProperty
End Property
Public Property Let StructProperty(s As VBStruct)
    mvarStructProperty = s
End Property

Public Function GetStructFunc() As VBStruct
    GetStructFunc = mvarStructProperty
End Function
Public Sub SetStructSub(s As VBStruct)
    mvarStructProperty = s
End Sub

Public Property Get StructArrayProperty() As VBStruct()
    StructArrayProperty = mvarStructArrayProperty
End Property

Public Property Let StructArrayProperty(s() As VBStruct)
    mvarStructArrayProperty = s
End Property

Public Property Let VariantProperty(ByVal vData As Variant)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.VariantProperty = 5
    mvarVariantProperty = vData
End Property


Public Property Set VariantProperty(ByVal vData As Object)
'used when assigning an Object to the property, on the left side of a Set statement.
'Syntax: Set x.VariantProperty = Form1
    Set mvarVariantProperty = vData
End Property


Public Property Get VariantProperty() As Variant
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.VariantProperty
    If IsObject(mvarVariantProperty) Then
        Set VariantProperty = mvarVariantProperty
    Else
        VariantProperty = mvarVariantProperty
    End If
End Property

Public Property Let IntProperty(ByVal vData As Long)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.IntProperty = 5
    mvarIntProperty = vData
End Property


Public Property Get IntProperty() As Long
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.IntProperty
    IntProperty = mvarIntProperty
End Property

Public Property Let LongProperty(ByVal vData As Long)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.LongProperty = 5
    mvarLongProperty = vData
End Property

Public Property Get LongProperty() As Long
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.LongProperty
    LongProperty = mvarLongProperty
End Property

Private Sub Class_Initialize()
    Set mvarCollectionProperty = New Collection
    mvarCollectionProperty.Add (1)
    mvarCollectionProperty.Add ("Two")
    mvarCollectionProperty.Add ("3")
    
    mvarStructProperty.int_val = 99
    mvarStructProperty.str_val = "hello"
    mvarStructProperty.sub_val.int_val = 66
    mvarStructProperty.sub_val.str_val = "sub hello"
    mvarStructProperty.sub_val.array_val(0).int_val = 0
    mvarStructProperty.sub_val.array_val(0).str_val = "zero"
    mvarStructProperty.sub_val.array_val(1).int_val = 1
    mvarStructProperty.sub_val.array_val(1).str_val = "one"
    mvarStructProperty.sub_val.array_val(2).int_val = 2
    mvarStructProperty.sub_val.array_val(2).str_val = "two"
  
End Sub

Public Sub IncrementIntegerParam(intVal As Integer)
    intVal = intVal + 1
End Sub

Public Sub IncrementVariantParam(ByRef varVal As Variant)
    varVal = varVal + 1
End Sub

Public Function TakeByRefObject(ByRef ob As Object) As Object
    Set TakeByRefObject = ob
End Function

Public Function TakeByValObject(ByVal ob As Object) As Object
    Set TakeByValObject = ob
End Function

Public Sub MakeStructArrayProperty(n As Integer)
    ReDim mvarStructArrayProperty(n)
    Dim i As Integer
    For i = 0 To n
        mvarStructArrayProperty(i).int_val = i
        mvarStructArrayProperty(i).sub_val.int_val = i
        mvarStructArrayProperty(i).sub_val.array_val(0).int_val = i
        mvarStructArrayProperty(i).sub_val.array_val(1).int_val = i + 1
        mvarStructArrayProperty(i).sub_val.array_val(2).int_val = i + 2
    Next
End Sub

Public Sub DoSomeCallbacks(ob As Object)
    Dim i, i2 As Integer
    Dim s As String
    Dim ret As Integer
    Dim variantResult As Variant
    Dim rg(1) As Integer
    Dim rgv(1) As Variant
    Dim rg_new As Variant
    Rem On Error GoTo failed    ' Dont really need this - error is correctly propogated back to Python!
    
    Rem Can check byrefs for gateways here, as VB passes all params byref!
    i = 99
    ret = ob.CallbackResultOneByRef(i)
    If i <> 100 Then MsgBox "After CallbackResultOneByRef, the param was in error - was " & i & " but expected 100"
    If ret <> 99 Then MsgBox "After CallbackResultOneByRef, the result was in error - was " & ret & " but expected 99"
    ob.CallbackVoidOneByRef i
    If i <> 101 Then MsgBox "After CallbackVoidOneByRef, the param was in error - was " & i & " but expected 101"
    i = 100
    i2 = 50
    ob.CallbackVoidTwoByRef i, i2
    If i <> 150 And i2 <> 50 Then MsgBox "After CallbackVoidTwoByRef, the param was in error - was " & i & "/" & i2 & " but expected 150/50"
    i = 100
    variantResult = ob.CallbackResultOneByRefButReturnNone(i)
    ' i should remain the same
    If i <> 100 Then MsgBox "After CallbackResultOneByRefButReturnNone, the param was in error - was " & i & " but expected 100"
    If variantResult <> Null Then MsgBox "After CallbackResultOneByRefButReturnNone, the param was in error - was " & variantResult & " but expected Null"
    ' See what happens to a ByRef with a void function!
    ' XXX - was expecting this to fail - VB passes as VT_VARIANT|VT_BYREF
    ' and Python sets the Variant to NULL - but VB doesnt complain!
    ob.CallbackVoidOneByRefButReturnNone i
    If i <> 100 Then MsgBox "After CallbackVoidOneByRefButReturnNone, the param was in error - was " & i & " but expected 100"
    rgv(0) = 0
    rgv(1) = 1
    ' NOTE - MUST use subroutine syntax to get byrefs passed!
    ob.CallbackArrayResult rgv
    If rgv(0) <> 1 Or rgv(1) <> 2 Then MsgBox "CallbackArrayResult (sub, variant) failed - (" & rgv(0) & "," & rgv(1) & ")!!"
    ' Try it with doubles, just for the sake of testing!
    rgv(0) = 0#
    rgv(1) = 1#
    ' NOTE - MUST use subroutine syntax to get byrefs passed!
    ob.CallbackArrayResult rgv
    If Abs(rgv(0) - 1#) > 0.01 Or Abs(rgv(1) - 2#) > 0.01 Then MsgBox "CallbackArrayResult (sub, variant) failed - (" & rgv(0) & "," & rgv(1) & ")!!"
    rg(0) = 0
    rg(1) = 1
    ob.CallbackArrayResult rg
    If rg(0) <> 1 Or rg(1) <> 2 Then MsgBox "CallbackArrayResult (sub) failed - (" & rg(0) & "," & rg(1) & ")!!"
    rg_new = ob.CallbackArrayResultOneArrayByRef(rg)
    ' rg_new should now be rg before the call.
    If rg_new(0) <> 1 Or rg_new(1) <> 2 Then MsgBox "CallbackArrayResultOneArrayByRef - result wrong - - (" & rg_new(0) & "," & rg_new(1) & ")!!"
    ' The byref array gets incremented.
    ' EEEK - this doesnt work.
    ' Cos we are using function call syntax, the array is _not_ passed byref!!!
    'If rg(0) <> 1 Or rg(1) <> 2 Then MsgBox "CallbackArrayResultOneArrayByRef - byref param wrong - - (" & rg(0) & "," & rg(1) & ")!!"
    Exit Sub
failed:
    MsgBox ("Calling back into Python failed" & Chr(11) & Err.Description)
    Err.Raise Err.Number
End Sub

Public Sub DoCallbackSafeArraySizeFail(ob As Object)
    Dim rg(1) As Integer
    rg(0) = 0
    rg(1) = 1
    ob.CallbackArrayResultWrongSize rg
'    Err.Raise Err.Message = "Expecting Python have failed!"
End Sub

Public Function GetInterfaceTester(ByVal initval As Integer) As IPyComTestInterface
    Dim n As IPyComTestInterface
    Set n = New PyComTestImplementation
    n.setn initval
    Set GetInterfaceTester = n
    Set n = Nothing
End Function
