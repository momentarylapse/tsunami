/*
 * SampleSelectionDialog.h
 *
 *  Created on: 6 Aug 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_
#define SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_

#include "../../lib/hui/hui.h"

class Sample;
class Session;
class Song;

class SampleSelectionDialog : public hui::Dialog {
public:
	SampleSelectionDialog(Session *_session, hui::Panel *parent, Sample *old);
	virtual ~SampleSelectionDialog();

	void fill_list();
	void on_select();
	void on_list();
	void on_import();
	void on_ok();
	void on_cancel();

	Sample *selected;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	Session *session;
	string list_id;

	base::promise<Sample*> _promise;

	static base::future<Sample*> select(Session *session, hui::Panel *parent, Sample *old);
};

#endif /* SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_ */
