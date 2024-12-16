/*
 * SampleSelectionDialog.h
 *
 *  Created on: 6 Aug 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_
#define SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_

#include "../../lib/hui/Window.h"

namespace tsunami {

class Sample;
class Session;
class Song;

class SampleSelectionDialog : public hui::Dialog {
public:
	SampleSelectionDialog(Session *_session, hui::Panel *parent, Sample *old);
	~SampleSelectionDialog() override;

	void fill_list();
	void on_select();
	void on_right_click();
	void on_preview();
	void on_list();
	void on_import();
	void on_ok();
	void on_cancel();
	Sample* get_selected();

	Sample *selected;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	Session *session;
	string list_id;

	owned<hui::Menu> menu_samples;

	base::promise<Sample*> _promise;

	static base::future<Sample*> select(Session *session, hui::Panel *parent, Sample *old);
};

}

#endif /* SRC_VIEW_DIALOG_SAMPLESELECTIONDIALOG_H_ */
