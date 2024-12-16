/*
 * SampleSelectionDialog.cpp
 *
 *  Created on: 6 Aug 2023
 *      Author: michi
 */

#include "SampleSelectionDialog.h"

#include <os/msg.h>

#include "../../storage/Storage.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../Session.h"
#include "../../lib/base/iter.h"
#include "../../lib/hui/language.h"
#include "../../lib/hui/hui.h"
#include "../../lib/hui/Menu.h"
#include "../helper/SamplePreviewPlayer.h"

namespace tsunami {

string get_sample_preview(Sample *s, AudioView *view);

SampleSelectionDialog::SampleSelectionDialog(Session *_session, hui::Panel *parent, Sample *old) :
	hui::Dialog("sample_selection_dialog", parent->win)
{
	session = _session;
	song = session->song.get();
	selected = nullptr;
	_old = old;
	for (Sample *s: weak(song->samples))
		if (s == old)
			selected = s;

	list_id = "sample_selection_list";

	menu_samples = hui::create_resource_menu("popup-menu-sample-selection", this);

	fill_list();

	event("import", [this] { on_import(); });
	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_cancel(); });
	event("hui:close", [this] { on_cancel(); });
	event_x(list_id, "hui:select", [this] { on_select(); });
	event_x(list_id, "hui:right-button-down", [this] { on_right_click(); });
	event(list_id, [this] { on_list(); });
	event("sample-preview", [this] { on_preview(); });
}

SampleSelectionDialog::~SampleSelectionDialog() {
	// no, keep them forever! ...
	//for (string &name: icon_names)
	//	hui::delete_image(name);
}

void SampleSelectionDialog::fill_list() {
	reset(list_id);

	set_string(list_id, _("\\- none -\\"));
	set_int(list_id, 0);
	for (const auto& [i, s]: enumerate(weak(song->samples))) {
		icon_names.add(get_sample_preview(s, session->view));
		set_string(list_id, icon_names[i] + "\\" + song->get_time_str_long(s->buf->length) + "\\" + s->name);
		if (s == selected)
			set_int(list_id, i + 1);
	}
}

void SampleSelectionDialog::on_select() {
	selected = get_selected();
	enable("ok", selected);
}

void SampleSelectionDialog::on_right_click() {
	int n = hui::get_event()->row;
	menu_samples->enable("sample-preview", n >= 1);
	menu_samples->open_popup(this);
}


Sample* SampleSelectionDialog::get_selected() {
	int n = get_int("");
	if (n >= 1)
		return song->samples[n - 1].get();
	return nullptr;
}

void SampleSelectionDialog::on_list() {
	selected = get_selected();
	_promise(selected);
	request_destroy();
}

void SampleSelectionDialog::on_import() {
	session->storage->ask_open_import(win).then([this] (const Path &filename) {
		session->storage->load_buffer(filename).then([this, filename] (const AudioBuffer &buf) {
			song->create_sample_audio(filename.basename_no_ext(), buf);
			fill_list();
		});
	});
}

void SampleSelectionDialog::on_preview() {
	int n = hui::get_event()->row;
	if (n >= 1)
		SamplePreviewPlayer::play(this, session, song->samples[n - 1].get());
}


void SampleSelectionDialog::on_ok() {
	_promise(selected);
	request_destroy();
}

void SampleSelectionDialog::on_cancel() {
	selected = _old;
	_promise.fail();
	request_destroy();
}

base::future<Sample*> SampleSelectionDialog::select(Session *session, hui::Panel *parent, Sample *old) {
	auto s = new SampleSelectionDialog(session, parent, old);
	hui::fly(s);
	return s->_promise.get_future();
}

}
