// Scintilla source code edit control
// Editor.h - defines the main editor class
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EDITOR_H
#define EDITOR_H

class Caret {
public:
	bool active;
	bool on;
	int period;
		
	Caret();
};

class Timer {
public:
	bool ticking;
	int ticksToWait;
	enum {tickSize = 100};
	int tickerID;
		
	Timer();
};

class LineLayout {
public:
	// Drawing is only performed for maxLineLength characters on each line.
	enum {maxLineLength = 4000};
	int numCharsInLine;
	char chars[maxLineLength];
	char styles[maxLineLength];
	char indicators[maxLineLength];
	int positions[maxLineLength];
};

class Editor : public DocWatcher {
protected:	// ScintillaBase subclass needs access to much of Editor

	// On GTK+, Scintilla is a container widget holding two scroll bars and a drawing area
	// whereas on Windows there is just one window with both scroll bars turned on.
	// Therefore, on GTK+ the following are separate windows but only one window on Windows.
	Window wMain;	// The Scintilla parent window
	Window wDraw;	// The text drawing area

	// Style resources may be expensive to allocate so are cached between uses.
	// When a style attribute is changed, this cache is flushed.
	bool stylesValid;	
	ViewStyle vs;
	Palette palette;
	
	bool hideSelection;
	bool inOverstrike;

	// In bufferedDraw mode, graphics operations are drawn to a pixmap and then copied to 
	// the screen. This avoids flashing but is about 30% slower.
	bool bufferedDraw;

	int xOffset;

	Surface pixmapLine;
	Surface pixmapSelMargin;
	Surface pixmapSelPattern;
	// Intellimouse support - currently only implemented for Windows
	unsigned int ucWheelScrollLines;
	short cWheelDelta; //wheel delta from roll

	KeyMap kmap;

	Caret caret;
	Timer timer;

	Point lastClick;
	unsigned int lastClickTime;
	enum { selChar, selWord, selLine } selectionType;
	Point ptMouseLast;
	bool firstExpose;
	bool inDragDrop;
	bool dropWentOutside;
	int posDrag;
	int posDrop;
	int lastXChosen;
	int lineAnchor;
	int originalAnchorPos;
	int currentPos;
	int anchor;
	int topLine;
	
        Position braces[2];
	int bracesMatchStyle;
        
	enum { notPainting, painting, paintAbandoned } paintState;
	PRectangle rcPaint;

	char *dragChars;
	int lenDrag;
	
	Document doc;

	Editor();
	virtual ~Editor();
	virtual void Initialise() = 0;
	virtual void Finalise();

	void InvalidateStyleData();
	void InvalidateStyleRedraw();
	virtual void RefreshColourPalette(Palette &pal, bool want);
	void RefreshStyleData();
	void DropGraphics();

	PRectangle GetClientRectangle();
	PRectangle GetTextRectangle();
	
	int LinesOnScreen();
	int LinesToScroll();
	int MaxScrollPos();
	Point LocationFromPosition(unsigned int pos);
	int PositionFromLocation(Point pt);
	int LineFromLocation(Point pt);

	void RedrawRect(PRectangle rc);
	void Redraw();
	void RedrawSelMargin();
	PRectangle RectangleFromRange(int start, int end);
	void InvalidateRange(int start, int end);
	
	int CurrentPosition();
	bool SelectionEmpty();
	int SelectionStart();
	int SelectionEnd();
	void SetSelection(int currentPos_, int anchor_);
	void SetSelection(int currentPos_);
	void SetEmptySelection(int currentPos_);
	void SetPosition(int pos, bool shift=false);
	int MovePositionTo(int newPos, bool extend = false);
	void SetLastXChosen();

	void ScrollTo(int line);
	virtual void ScrollText(int linesToMove);
	void HorizontalScrollTo(int xPos);
	void EnsureCaretVisible();
	void ShowCaretAtCurrentPosition();
	void DropCaret();
	void InvalidateCaret();

	void PaintSelMargin(Surface *surface, PRectangle &rc);
        void LayoutLine(int line, Surface *surface, ViewStyle &vstyle, LineLayout &ll);
	void DrawLine(Surface *surface, ViewStyle &vsDraw, int line, int xStart, 
		PRectangle rcLine, LineLayout &ll);
	void Paint(Surface *surfaceWindow, PRectangle rcArea);
	long FormatRange(bool draw, FORMATRANGE *pfr);

	virtual void SetVerticalScrollPos() = 0;
	virtual void SetHorizontalScrollPos() = 0;
	virtual bool ModifyScrollBars(int nMax, int nPage) = 0;
	void SetScrollBarsTo(PRectangle rsClient);
	void SetScrollBars();

	virtual void AddChar(char ch);
	void ClearSelection();
	void ClearAll();
	void Cut();
	virtual void Copy() = 0;
	virtual void Paste() = 0;
	void Clear();
	void SelectAll();
	void Undo();
	void Redo();
	void DelChar();
	void DelCharBack();
	virtual void ClaimSelection() = 0;

	virtual void NotifyChange() = 0;
	virtual void NotifyParent(SCNotification scn) = 0;
	void NotifyStyleNeeded(int endStyleNeeded);
	void NotifyChar(char ch);
	void NotifySavePoint(bool isSavePoint);
	void NotifyModifyAttempt();
	virtual void NotifyDoubleClick(Point pt, bool shift);
        void NotifyCheckBrace();

	void NotifyModifyAttempt(Document *doc, void *userData);
	void NotifySavePoint(Document *doc, void *userData, bool atSavePoint);
	void NotifyModified(Document *doc, void *userData);
	
	void PageMove(int direction, bool extend=false);
	virtual int KeyCommand(UINT iMessage);
	virtual int KeyDefault(int /* key */, int /*modifiers*/);
	int KeyDown(int key, bool shift, bool ctrl, bool alt);

	bool GetWhitespaceVisible();
	void SetWhitespaceVisible(bool view);

	void Indent(bool forwards);

	long FindText(UINT iMessage,WPARAM wParam,LPARAM lParam);
	void GoToLine(int lineNo);

	char *CopyRange(int start, int end);
	char *CopySelectionRange();
	void CopySelectionIntoDrag();
	void SetDragPosition(int newPos);
	virtual void StartDrag();
	void DropAt(int position, const char *value, bool moving);
	bool PositionInSelection(int pos);
	bool PointInSelection(Point pt);
	bool PointInSelMargin(Point pt);
	virtual void ButtonDown(Point pt, unsigned int curTime, bool shift, bool ctrl);
	void ButtonMove(Point pt);
	void ButtonUp(Point pt, unsigned int curTime, bool ctrl);

	void Tick();
	virtual void SetTicking(bool on) = 0;
	virtual void SetMouseCapture(bool on) = 0;
	virtual bool HaveMouseCapture() = 0;

	Range RangeFromRectangle(PRectangle rc);
	void CheckForChangeOutsidePaint(Range r);
        int BraceMatch(int position, int maxReStyle);
	void SetBraceHighlight(Position pos0, Position pos1, int matchStyle);

	virtual LRESULT DefWndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) = 0;
	
public:
	// Public so scintilla_send_message can use it
	virtual LRESULT WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam);
	// Public so scintilla_set_id can use it
	int ctrlID;	
};

#define STYLE_MASK	0x1F	// Mask to get the style number from the styling byte

#endif
