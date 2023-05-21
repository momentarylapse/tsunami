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

class QuestionDialogFloat : hui::Dialog {
public:
	using Callback = std::function<void(float)>;
	QuestionDialogFloat(hui::Window *parent, const string &question, float value0, float min, float max, const string &options, Callback cb);
	float result;
	float _min, _max, _min_db, _max_db;
	enum Mode {
		DIRECT,
		VOLUME_PERCENT,
		VOLUME_DB
	};
	Mode mode;
	void set_mode(Mode m);
	void set_spin(float f);
	void set_slider(float f);
	float get_spin();
	float get_slider();
	Callback cb;
	static bool aborted;
	static void ask(hui::Window *parent, const string &question, Callback cb, float value0, float min, float max, const string& options = "");


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 12;
	static constexpr float TAN_SCALE = 13.0f;

	static float db2slider(float db);
	static float slider2db(float val);
};


class QuestionDialogMultipleChoice : hui::Dialog {
public:
	QuestionDialogMultipleChoice(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> cb);
	int result;
	std::function<void(int)> cb;
	static void ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> cb);
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
