/*
 * QuestionDialog.h
 *
 *  Created on: May 1, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_QUESTIONDIALOG_H_
#define SRC_VIEW_DIALOG_QUESTIONDIALOG_H_

#include "../../lib/hui/hui.h"

class QuestionDialogInt : hui::Dialog {
public:
	QuestionDialogInt(hui::Window *parent, const string &question, const string &options, std::function<void(int)> cb);
	int result;
	std::function<void(int)> cb;
	static bool aborted;
	static void ask(hui::Window *parent, const string &question, std::function<void(int)> cb, const string &options = "");
};


class QuestionDialogIntInt : hui::Dialog {
public:
	QuestionDialogIntInt(hui::Window *_parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> cb);
	int result1, result2;
	std::function<void(int,int)> cb;
	static bool aborted;
	static void ask(hui::Window *parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> cb);
};


class QuestionDialogMultipleChoice : hui::Dialog {
public:
	QuestionDialogMultipleChoice(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> cb);
	int result;
	std::function<void(int)> cb;
	static void ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> cb);
};

#endif /* SRC_VIEW_DIALOG_QUESTIONDIALOG_H_ */
