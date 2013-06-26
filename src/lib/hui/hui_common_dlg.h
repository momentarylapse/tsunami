/*----------------------------------------------------------------------------*\
| Hui common dialogs                                                           |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_COMMON_DLG_EXISTS_
#define _HUI_COMMON_DLG_EXISTS_



// file dialogs
bool _cdecl HuiFileDialogOpen(HuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter);
bool _cdecl HuiFileDialogSave(HuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter);
bool _cdecl HuiFileDialogDir(HuiWindow *win, const string &title, const string &dir/*, string &root_dir*/);
extern string HuiFilename;
bool _cdecl HuiSelectColor(HuiWindow *win, int r, int g, int b);
extern int HuiColor[4];
bool _cdecl HuiSelectFont(HuiWindow *win, const string &title);
extern string HuiFontname;

// additional application properties
void _cdecl HuiAboutBox(HuiWindow *win);
void _cdecl HuiSetProperty(const string &name, const string &value);
string HuiGetProperty(const string &name);
extern Array<string> HuiPropAuthors;

// message dialogs
string _cdecl HuiQuestionBox(HuiWindow *win, const string &title, const string &text, bool allow_cancel = false);
void _cdecl HuiInfoBox(HuiWindow *win, const string &title, const string &text);
void _cdecl HuiErrorBox(HuiWindow *win, const string &title, const string &text);


#endif
