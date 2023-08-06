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

using FileFuture = future<const Path&>;
using ColorFuture = future<const color&>;
using FontFuture = future<const string&>;
using QuestionFuture = future<bool>;

// file dialogs
FileFuture file_dialog_open(Window *win, const string &title, const Path &dir, const Array<string> &params);
FileFuture file_dialog_save(Window *win, const string &title, const Path &dir, const Array<string> &params);
FileFuture file_dialog_dir(Window *win, const string &title, const Path &dir, const Array<string> &params);
ColorFuture select_color(Window *win, const string &title, const color &c);
FontFuture select_font(Window *win, const string &title, const Array<string> &params);

// message dialogs
QuestionFuture question_box(Window *win, const string &title, const string &text, bool allow_cancel = false);
void _cdecl info_box(Window *win, const string &title, const string &text);
void _cdecl error_box(Window *win, const string &title, const string &text);


void _cdecl about_box(Window *win);

};


#endif
