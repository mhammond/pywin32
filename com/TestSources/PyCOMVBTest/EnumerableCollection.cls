VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "EnumerableCollection"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Private col As New Collection

Public Property Get Count() As Integer
    Count = col.Count
End Property

Public Sub Add(ByVal Item As Variant)
    col.Add Item
End Sub

' Default property
Public Property Get Item(v As Variant) As Variant
Attribute Item.VB_UserMemId = 0
    Set Item = col(v)
End Property

' NewEnum must have the procedure ID -4 in Procedure Attributes dialog
' This procedure is *not* marked as hidden
Public Function NewEnum() As IEnumVARIANT
Attribute NewEnum.VB_UserMemId = -4
    Set NewEnum = col.[_NewEnum]
End Function

