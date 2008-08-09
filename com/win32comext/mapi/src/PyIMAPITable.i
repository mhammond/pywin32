/* File : PyIMAPITable.i */

%module IMAPITable // An COM interface to MAPI

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "mapilib.i"

%{

#include "PyIMAPITable.h"

PyIMAPITable::PyIMAPITable(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIMAPITable::~PyIMAPITable()
{
}

/*static*/ IMAPITable *PyIMAPITable::GetI(PyObject *self)
{
	return (IMAPITable *)PyIUnknown::GetI(self);
}


%}


// GetLastError|Returns a MAPIERROR structure containing information about the previous error on the table. 
HRESULT GetLastError(HRESULT hr, unsigned long flags, MAPIERROR **OUTPUT);

// @pyswig int|Advise|Registers to receive notification of specified events affecting the table. 
HRESULT Advise(
	unsigned long ulEventMask, // @pyparm int|eventMask||
	IMAPIAdviseSink *INPUT,	   // @pyparm <o PyIMAPIAdviseSink>|adviseSink||	
	unsigned long *OUTPUT );

// @pyswig int|SeekRow|Moves the cursor to a specific position in the table.
HRESULT SeekRow(
	unsigned long bm,	// @pyparm int|bookmark||The bookmark.
	long rowCount,  // @pyparm int|rowCount||
	long *OUTPUT
// @rdesc The result is the number of rows processed.
);

// @pyswig |SeekRowApprox|Moves the cursor to an approximate fractional position in the table. 
HRESULT SeekRowApprox(
	unsigned long ulNumerator, // @pyparm int|numerator||The numerator of the fraction representing the table position
	unsigned long ulDenominator // @pyparm int|denominator||The denominator of the fraction representing the table position. This must not be zero.
);


// @pyswig int|GetRowCount|Returns the total number of rows in the table. 
HRESULT GetRowCount(
	unsigned long ulFlags, // @pyparm int|flags||Reserved - must be zero
	unsigned long *OUTPUT
);

// @pyswig <o SRowSet>|QueryRows|Returns one or more rows from a table, beginning at the current cursor position.
HRESULT QueryRows( 
	long rowCount, // @pyparm int|rowCount||Number of rows to retrieve
	ULONG ulFlags, // @pyparm int|flags||Flags.
	SRowSet **OUTPUT); 

// @pyswig |SetColumns|Defines the particular properties and order of properties to appear as columns in the table.
HRESULT SetColumns(
	SPropTagArray *INPUT, // @pyparm <o SPropTagArray>|propTags||Sequence of property tags identifying properties to be included as columns in the table.
	unsigned long lFlags // @pyparm int|flags||
);

// @pyswig |GetStatus|Returns the table's status and type. 
// @rdesc Result is a tuple of (tableStatus, tableType)
HRESULT GetStatus(
	unsigned long *OUTPUT,
	unsigned long *OUTPUT
); 

// @pyswig |QueryPosition|Retrieves the current table row position of the cursor, based on a fractional value. 
// @rdesc Result is a tuple of (row, numerator, denominator)
HRESULT QueryPosition(
	unsigned long *OUTPUT,
	unsigned long *OUTPUT,
	unsigned long *OUTPUT
);

// @pyswig <o SPropTagArray>|QueryColumns|Returns a list of columns for the table. 
HRESULT QueryColumns(
	unsigned long lFlags, // @pyparm int|flags||
	SPropTagArray **OUTPUT
);

// @pyswig |Abort|Stops any asynchronous operations currently in progress for the table. 
HRESULT Abort();

// @pyswig |FreeBookmark|Releases the memory associated with a bookmark. 
HRESULT FreeBookmark(
	unsigned long bm // @pyparm int|bookmark||
);

// @pyswig int|CreateBookmark|Marks the table's current position. 
HRESULT CreateBookmark(
	unsigned long *OUTPUT
);

// @pyswig |Restrict|Applies a filter to a table, reducing the row set to only those rows matching the specified criteria. 
HRESULT Restrict(
	SRestriction *INPUT, // @pyparm <o PySRestriction>|restriction||
	unsigned long ulFlags // @pyparm int|flags||
);

// @pyswig |FindRow|Finds the next row in a table that matches specific search criteria. 
HRESULT FindRow(
	SRestriction *INPUT, // @pyparm <o PySRestriction>|restriction||
	BOOKMARK bkOrigin, // @pyparm int|bookmarkOrigin||
	unsigned long ulFlags // @pyparm int|flags||
);

// @pyswig |SortTable|Orders the rows of the table based on sort criteria. 
HRESULT SortTable(
	SSortOrderSet *INPUT, // @pyparm <o PySSortOrderSet>|sortOrderSet||
	unsigned long flags // @pyparm int|flags||
);

// @pyswig |Unadvise|Cancels the sending of notifications previously set up with a call to the IMAPITable::Advise method. 
HRESULT Unadvise(
	unsigned long handle); // @pyparm int|handle||Handle returned from <om PyIMAPITable.Advise>


/*

QuerySortOrder|Retrieves the current sort order for a table. 


ExpandRow|Expands a collapsed table category, adding the leaf rows belonging to the category to the table view. 

CollapseRow|Collapses an expanded table category, removing the leaf rows belonging to the category from the table view. 

WaitForCompletion|Suspends processing until one or more asynchronous operations in progress on the table have completed. 

GetCollapseState|Returns the data necessary to rebuild the current collapsed or expanded state of a categorized table. 

SetCollapseState|Rebuilds the current expanded or collapsed state of a categorized table using data that was saved by a prior call to the IMAPITable::GetCollapseState method. 
*/
