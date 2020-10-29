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

QuestionDialogInt::QuestionDialogInt(hui::Window *_parent, const string &question, const string &options) : hui::Dialog("question-dialog-int", _parent){
	result = 0;
	aborted = true;
	set_string("question", question);
	set_options("value", options);

	event("value", [=]{ result = get_int("value"); });
	event("cancel", [=]{ request_destroy(); });
	event("ok", [=]{ aborted = false; request_destroy(); });
}

int QuestionDialogInt::ask(hui::Window *parent, const string &question, const string &options) {
	auto dlg = ownify(new QuestionDialogInt(parent, question, options));
	dlg->run();
	return dlg->result;
}









QuestionDialogIntInt::QuestionDialogIntInt(hui::Window *_parent, const string &question, const Array<string> &labels, const Array<string> &options) : hui::Dialog(_("Question"), 100, 20, _parent, false){
	result1 = 0;
	result2 = 0;
	aborted = true;
	from_resource("question-dialog-intint");
	set_string("question", question);
	set_string("label1", labels[0]);
	set_string("label2", labels[1]);
	set_options("value1", options[0]);
	set_options("value2", options[1]);

	event("value1", [=]{ result1 = get_int("value1"); });
	event("value2", [=]{ result2 = get_int("value2"); });
	event("cancel", [=]{ request_destroy(); });
	event("ok", [=]{ aborted = false; request_destroy(); });
}

std::pair<int,int> QuestionDialogIntInt::ask(hui::Window *parent, const string &question, const Array<string> &labels, const Array<string> &options) {
	auto *dlg = new QuestionDialogIntInt(parent, question, labels, options);
	dlg->run();
	auto r = std::make_pair(dlg->result1, dlg->result2);
	delete dlg;
	return r;
}





int QuestionDialogMultipleChoice::ask(hui::Window *parent, const string &title, const string &text, const Array<string> &options, const Array<string> &tips, bool allow_cancel) {
	int r = -1;
	auto *dlg = new hui::Dialog(title, 200, 40, parent, false);
	dlg->add_grid("", 0, 0, "grid");
	dlg->set_target("grid");
	dlg->add_label("!margin-top=8,margin-bottom=8,center\\" + text, 0, 0, "text");
	dlg->add_grid("", 0, 1, "buttons");
	dlg->set_target("buttons");
	for (int i=0; i<options.num; i++) {
		string id = format("button-%d", i);
		dlg->add_button("!expandx,height=36\\" + options[i], i, 0, id);
		if (tips.num > i)
			dlg->set_tooltip(id, tips[i]);
		dlg->event(id, [i,&r,dlg] { r = i; dlg->request_destroy(); });
	}
	dlg->run();
	return r;
}



