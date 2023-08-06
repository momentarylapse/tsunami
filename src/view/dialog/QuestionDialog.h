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
	QuestionDialogInt(hui::Window *parent, const string &question, const string &options);
	int result;
	hui::promise<int> _promise;
	static hui::future<int> ask(hui::Window *parent, const string &question, const string &options = "");
};


class QuestionDialogIntInt : hui::Dialog {
public:
	QuestionDialogIntInt(hui::Window *_parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> cb);
	int result1, result2;
	std::function<void(int,int)> cb;
	static bool aborted;
	static void ask(hui::Window *parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> cb);
};

class QuestionDialogFloat : hui::Dialog {
public:
	QuestionDialogFloat(hui::Window *parent, const string &question, float value0, float min, float max, const string &options);
	float result;
	float _min, _max;
	void set_spin(float f);
	void set_slider(float f);
	float get_spin();
	float get_slider();
	hui::promise<float> _promise;
	static hui::future<float> ask(hui::Window *parent, const string &question, float value0, float min, float max, const string& options = "");
};


class QuestionDialogMultipleChoice : hui::Dialog {
public:
	QuestionDialogMultipleChoice(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel);
	int result;
	hui::promise<int> _promise;
	static hui::future<int> ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel);
};

class QuestionDialogString : hui::Dialog {
public:
	using Callback = std::function<void(const string&)>;
	QuestionDialogString(hui::Window *parent, const string &question, const string &options, Callback cb);
	string result;
	Callback cb;
	static bool aborted;
	static void ask(hui::Window *parent, const string &question, Callback cb, const string &options = "");
};

#endif /* SRC_VIEW_DIALOG_QUESTIONDIALOG_H_ */
