/*----------------------------------------------------------------------------*\
| Hui common dialogs                                                           |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _COMMON_DLG_EXISTS_
#define _COMMON_DLG_EXISTS_

#include "../base/future.h"

struct color;

namespace hui {

class Window;

using FileFuture = base::future<Path>;
using ColorFuture = base::future<color>;
using FontFuture = base::future<string>;
using QuestionFuture = base::future<bool>;

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
