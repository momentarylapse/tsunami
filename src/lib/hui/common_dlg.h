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

typedef std::function<void(const Path&)> FileDialogCallback;
typedef std::function<void(const color&)> ColorDialogCallback;
typedef std::function<void(const string&)> FontDialogCallback;
typedef std::function<void(const string&)> QuestionDialogCallback;

// file dialogs
void _cdecl file_dialog_open(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb);
void _cdecl file_dialog_save(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb);
void _cdecl file_dialog_dir(Window *win, const string &title, const Path &dir, const Array<string> &params, const FileDialogCallback &cb);
void _cdecl select_color(Window *win, const string &title, const color &c, const ColorDialogCallback &cb);
void _cdecl select_font(Window *win, const string &title, const Array<string> &params, const FontDialogCallback &cb);

// message dialogs
void _cdecl question_box(Window *win, const string &title, const string &text, const QuestionDialogCallback &cb, bool allow_cancel = false);
void _cdecl info_box(Window *win, const string &title, const string &text);
void _cdecl error_box(Window *win, const string &title, const string &text);


void _cdecl about_box(Window *win);

};


#endif
