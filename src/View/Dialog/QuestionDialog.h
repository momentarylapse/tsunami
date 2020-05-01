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
	QuestionDialogInt(hui::Window *_parent, const string &question, const string &options = "");
	int result;
	static int ask(hui::Window *parent, const string &question, const string &options = "");
};


class QuestionDialogIntInt : hui::Dialog {
public:
	QuestionDialogIntInt(hui::Window *_parent, const string &question, const Array<string> &labels, const Array<string> &options);
	int result1, result2;
	static std::pair<int,int> ask(hui::Window *parent, const string &question, const Array<string> &labels, const Array<string> &options);
};


class QuestionDialogMultipleChoice {
public:
	static int ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel);
};

#endif /* SRC_VIEW_DIALOG_QUESTIONDIALOG_H_ */
