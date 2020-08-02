/*----------------------------------------------------------------------------*\
| Hui common dialogs                                                           |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _COMMON_DLG_EXISTS_
#define _COMMON_DLG_EXISTS_


class color;

namespace hui {

class Window;

// file dialogs
bool _cdecl FileDialogOpen(Window *win, const string &title, const Path &dir, const string &show_filter, const string &filter);
bool _cdecl FileDialogSave(Window *win, const string &title, const Path &dir, const string &show_filter, const string &filter);
bool _cdecl FileDialogDir(Window *win, const string &title, const Path &dir/*, string &root_dir*/);
extern Path Filename;
bool _cdecl SelectColor(Window *win, const color &c);
extern color Color;
bool _cdecl SelectFont(Window *win, const string &title);
extern string Fontname;

// message dialogs
string _cdecl QuestionBox(Window *win, const string &title, const string &text, bool allow_cancel = false);
void _cdecl InfoBox(Window *win, const string &title, const string &text);
void _cdecl ErrorBox(Window *win, const string &title, const string &text);


void _cdecl AboutBox(Window *win);

};


#endif
