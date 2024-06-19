/*
 * QuestionDialog.h
 *
 *  Created on: May 1, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_QUESTIONDIALOG_H_
#define SRC_VIEW_DIALOG_QUESTIONDIALOG_H_

#include "../../lib/hui/hui.h"

namespace tsunami {

class QuestionDialogInt : hui::Dialog {
public:
	QuestionDialogInt(hui::Window *parent, const string &question, const string &options);
	int result;
	base::promise<int> _promise;
	static base::future<int> ask(hui::Window *parent, const string &question, const string &options = "");
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
	base::promise<float> _promise;
	static base::future<float> ask(hui::Window *parent, const string &question, float value0, float min, float max, const string& options = "");
};


class QuestionDialogMultipleChoice : hui::Dialog {
public:
	QuestionDialogMultipleChoice(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel);
	int result;
	base::promise<int> _promise;
	static base::future<int> ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel);
};

class QuestionDialogString : hui::Dialog {
public:
	QuestionDialogString(hui::Window *parent, const string &question, const string &options);
	string result;
	base::promise<string> _promise;
	static base::future<string> ask(hui::Window *parent, const string &question, const string &options = "");
};

}

#endif /* SRC_VIEW_DIALOG_QUESTIONDIALOG_H_ */
