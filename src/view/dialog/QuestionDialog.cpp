/*
 * QuestionDialog.cpp
 *
 *  Created on: May 1, 2020
 *      Author: michi
 */

#include "QuestionDialog.h"
#include "../../lib/base/pointer.h"


bool QuestionDialogInt::aborted;
bool QuestionDialogIntInt::aborted;
bool QuestionDialogFloat::aborted;
bool QuestionDialogString::aborted;


QuestionDialogInt::QuestionDialogInt(hui::Window *_parent, const string &question, const string &options, std::function<void(int)> _cb)
: hui::Dialog("question-dialog-int", _parent) {
	cb = _cb;
	result = 0;
	aborted = true;
	set_string("question", question);
	set_options("value", options);

	event("value", [this] {
		result = get_int("value");
	});
	event("cancel", [this] {
		cb(-1);
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		cb(result);
		request_destroy();
	});
}

void QuestionDialogInt::ask(hui::Window *parent, const string &question, std::function<void(int)> cb, const string &options) {
	hui::fly(new QuestionDialogInt(parent, question, options, cb));
}







QuestionDialogIntInt::QuestionDialogIntInt(hui::Window *_parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> _cb)
		: hui::Dialog(_("Question"), 100, 20, _parent, false) {
	cb = _cb;
	result1 = 0;
	result2 = 0;
	aborted = true;
	from_resource("question-dialog-intint");
	set_string("question", question);
	set_string("label1", labels[0]);
	set_string("label2", labels[1]);
	set_options("value1", options[0]);
	set_options("value2", options[1]);

	event("value1", [this] { result1 = get_int("value1"); });
	event("value2", [this] { result2 = get_int("value2"); });
	event("cancel", [this] {
		cb(-1,-1);
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		cb(result1, result2);
		request_destroy();
	});
}

void QuestionDialogIntInt::ask(hui::Window *parent, const string &question, const Array<string> &labels, const Array<string> &options, std::function<void(int,int)> cb) {
	hui::fly(new QuestionDialogIntInt(parent, question, labels, options, cb));
}



QuestionDialogFloat::QuestionDialogFloat(hui::Window *_parent, const string &question, float value0, float min, float max, const string &options, Callback _cb)
: hui::Dialog("question-dialog-float", _parent) {
	cb = _cb;
	result = value0;
	aborted = true;
	_min = min;
	_max = max;
	set_string("question", question);

	set_spin(result);
	set_slider(result);

	event("value", [this] {
		result = get_spin();
		set_slider(result);
	});
	event("slider", [this] {
		result = get_slider();
		set_spin(result);
	});
	event("cancel", [this] {
		cb(-1);
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		cb(result);
		request_destroy();
	});
}

void QuestionDialogFloat::set_spin(float f) {
	set_float("value", f);
}

void QuestionDialogFloat::set_slider(float f) {
	set_float("slider", (f - _min) / (_max - _min));
}

float QuestionDialogFloat::get_spin() {
	return get_float("value");
}

float QuestionDialogFloat::get_slider() {
	return _min + get_float("slider") * (_max - _min);
}

void QuestionDialogFloat::ask(hui::Window *parent, const string &question, Callback cb, float value0, float min, float max, const string &options) {
	hui::fly(new QuestionDialogFloat(parent, question, value0, min, max, options, cb));
}




QuestionDialogMultipleChoice::QuestionDialogMultipleChoice(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> _cb)
		: hui::Dialog(title, 100, 40, parent, false) {
	cb = _cb;
	result = -1;
	add_grid("", 0, 0, "grid");
	set_target("grid");
	add_label("!margin-top=8,margin-bottom=8,center\\" + text, 0, 0, "text");
	add_grid("!expandx", 0, 1, "buttons");
	set_target("buttons");
	for (int i=0; i<options.num; i++) {
		string id = format("button-%d", i);
		add_button("!expandx,height=36\\" + options[i], i, 0, id);
		if (tips.num > i)
			set_tooltip(id, tips[i]);
		event(id, [i,this] {
			result = i;
			cb(i);
			request_destroy();
		});
	}
}

void QuestionDialogMultipleChoice::ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel, std::function<void(int)> cb) {
	hui::fly(new QuestionDialogMultipleChoice(parent, title, text, options, tips, allow_cancel, cb));
}



QuestionDialogString::QuestionDialogString(hui::Window *_parent, const string &question, const string &options, Callback _cb)
: hui::Dialog("question-dialog-text", _parent) {
	cb = _cb;
	aborted = true;
	set_string("question", question);
	set_options("value", options);
	enable("ok", false);

	event("value", [this] {
		result = get_string("value");
		enable("ok", result.num > 0);
	});
	event("cancel", [this] {
		cb("");
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		cb(result);
		request_destroy();
	});
}

void QuestionDialogString::ask(hui::Window *parent, const string &question, Callback cb, const string &options) {
	hui::fly(new QuestionDialogString(parent, question, options, cb));
}

