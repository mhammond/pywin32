// Scintilla source code edit control
// LineMarker.cxx - defines the look of a line marker in the margin 
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include "Platform.h"

#include "Scintilla.h"
#include "LineMarker.h"

void LineMarker::Draw(Surface *surface, PRectangle &rc) {
	int minDim = Platform::Minimum(rc.Width(), rc.Height());
	int centreX = (rc.right + rc.left) / 2;
	int centreY = (rc.bottom + rc.top) / 2;
	int dimOn2 = minDim / 2;
	int dimOn4 = minDim / 4;
	if (markType == SC_MARK_ROUNDRECT) {
		PRectangle rcRounded = rc;
		rcRounded.left = rc.left + 1;
		rcRounded.right = rc.right - 1;
		surface->RoundedRectangle(rcRounded, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_CIRCLE) {
		PRectangle rcCircle;
		rcCircle.left = centreX - minDim / 2;
		rcCircle.top = centreY - minDim / 2;
		rcCircle.right = centreX + minDim / 2;
		rcCircle.bottom = centreY + minDim / 2;
		surface->Ellipse(rcCircle, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_ARROW) {
		Point pts[] = {
    		Point(centreX, centreY - minDim / 2),
    		Point(centreX, centreY + minDim / 2),
    		Point(centreX + minDim / 2, centreY),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
                 		fore.allocated, back.allocated);

	} else if (markType == SC_MARK_SMALLRECT) {
		PRectangle rcSmall;
		rcSmall.left = rc.left + 1;
		rcSmall.top = rc.top + 2;
		rcSmall.right = rc.right - 1;
		rcSmall.bottom = rc.bottom - 2;
		surface->RectangleDraw(rcSmall, fore.allocated, back.allocated);
	} else if (markType == SC_MARK_EMPTY) {
		// An invisible marker so don't draw anything
	} else { // SC_MARK_SHORTARROW
		Point pts[] = {
			Point(centreX, centreY + dimOn2),
			Point(centreX + dimOn2, centreY),
			Point(centreX, centreY - dimOn2),
			Point(centreX, centreY - dimOn4),
			Point(centreX - dimOn4, centreY - dimOn4),
			Point(centreX - dimOn4, centreY + dimOn4),
			Point(centreX, centreY + dimOn4),
			Point(centreX, centreY + dimOn2),
		};
		surface->Polygon(pts, sizeof(pts) / sizeof(pts[0]),
				fore.allocated, back.allocated);
	}
}
