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
bool _cdecl HuiFileDialogOpen(CHuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter);
bool _cdecl HuiFileDialogSave(CHuiWindow *win, const string &title, const string &dir, const string &show_filter, const string &filter);
bool _cdecl HuiFileDialogDir(CHuiWindow *win, const string &title, const string &dir/*, string &root_dir*/);
extern string HuiFilename;
bool _cdecl HuiSelectColor(CHuiWindow *win, int r, int g, int b);
extern int HuiColor[4];
void _cdecl HuiSetProperty(const string &name, const string &value);
void _cdecl HuiAboutBox(CHuiWindow *win);

// additional application properties
extern string HuiPropName;
extern string HuiPropVersion;
extern string HuiPropComment;
extern string HuiPropLogo;
extern string HuiPropCopyright;
extern string HuiPropWebsite;
extern string HuiPropLicense;
extern Array<string> HuiPropAuthors;

// message dialogs
string _cdecl HuiQuestionBox(CHuiWindow *win, const string &title, const string &text, bool allow_cancel = false);
void _cdecl HuiInfoBox(CHuiWindow *win, const string &title, const string &text);
void _cdecl HuiErrorBox(CHuiWindow *win, const string &title, const string &text);


#endif
