/* File : PyIMessage.i */

%module IMessage // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPIProp.h"
#include "PyIMessage.h"

PyIMessage::PyIMessage(IUnknown *pDisp) :
	PyIMAPIProp(pDisp)
{
	ob_type = &type;
}

PyIMessage::~PyIMessage()
{
}

/*static*/ IMessage *PyIMessage::GetI(PyObject *self)
{
	return (IMessage *)PyIUnknown::GetI(self);
}

%}


// @pyswig |SetReadFlag|Sets the read flags for a message
HRESULT SetReadFlag(
	ULONG ulFlags // @pyparm int|flag||Bitmask of flags that controls the setting of a message's read flag - that is, the message's MSGFLAG_READ flag in its PR_MESSAGE_FLAGS property and the processing of read reports.
);


// @pyswig <o PyIMAPITable>|GetAttachmentTable|Returns the message's attachment table.
HRESULT GetAttachmentTable(
	ULONG ulFlags, // @pyparm int|flags||Bitmask of flags that relate to the creation of the table.
	IMAPITable **OUTPUT
);

// @pyswig <o PyIAttach>|OpenAttach|Opens an attachment
HRESULT OpenAttach(
	ULONG ulAttachmentNum, // @pyparm int|attachmentNum||
	IID *INPUT_NULLOK, // @pyparm <o PyIID>|interface||The interface to use, or None
	ULONG ulFlags, // @pyparm int|flags||Bitmask of flags that controls how the attachment is opened.
	IAttach **OUTPUT
);

// @pyswig int, <o PyIAttach>|CreateAttach|Creates an attachment
// @rdesc The result is a tuple of (attachmentNum, attachmentObject)
HRESULT CreateAttach(
	IID *INPUT_NULLOK, // @pyparm <o PyIID>|interface||The interface to use, or None
	ULONG ulFlags, // @pyparm int|flags||Bitmask of flags that controls how the attachment is created.
	unsigned long *OUTPUT, // lpulAttachmentNum
	IAttach **OUTPUT
);

// @pyswig |DeleteAttach|Deletes an attachment
HRESULT DeleteAttach( 
	ULONG ulAttachmentNum, // @pyparm int|attachmentNum||
	ULONG ulUIParam, // @pyparm int|ulUIParam||
	IMAPIProgress *INPUT_NULLOK, // @pyparm <o PyIMAPIProgress>|interface||The interface to use, or None
	ULONG ulFlags // @pyparm int|flags||Bitmask of flags that controls the display of a user interface.
);

// @pyswig |ModifyRecipients|adds, deletes, or modifies message recipients.
HRESULT ModifyRecipients(
	unsigned long flags, // @pyparm int|flags||Bitmask of flags that controls the recipient changes. If zero is passed for the ulFlags parameter, ModifyRecipients replaces all existing recipients with the recipient list in the mods parameter. 
	ADRLIST *INPUT // @pyparm object|mods||The list of recipients.
);

// @pyswig <o PyIMAPITable>|GetRecipientTable|Returns the message's recipient table.
HRESULT GetRecipientTable(
	ULONG ulFlags, // @pyparm int|flags||Bitmask of flags that relate to the creation of the table.
	IMAPITable **OUTPUT
);

// @pyswig |SubmitMessage|Saves all of the message's properties and marks the message as ready to be sent.
HRESULT SubmitMessage(
	unsigned long flags // @pyparm int|flags||Flags which specify how the message is submitted.
);

