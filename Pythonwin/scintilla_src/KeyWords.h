// SciTE Neil Hodgson, December 1998
// KeyWords.h
// Colourise for particular languages

#ifdef GTK
#undef HWND
#define HWND GtkWidget*
#endif
void ColouriseDoc(char *cdoc, int startPos, int lengthDoc, int initStyle, const char *language, char **keywords, HWND hwnd);
void TabTimmy(char *cdoc, int lengthDoc, HWND hwnd);
