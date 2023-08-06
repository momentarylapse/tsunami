/*
 * SampleSelectionDialog.cpp
 *
 *  Created on: 6 Aug 2023
 *      Author: michi
 */

#include "SampleSelectionDialog.h"
#include "../../storage/Storage.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../Session.h"

string render_sample(Sample *s, AudioView *view);

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

	fill_list();

	event("import", [this] { on_import(); });
	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_cancel(); });
	event("hui:close", [this] { on_cancel(); });
	event_x(list_id, "hui:select", [this] { on_select(); });
	event(list_id, [this] { on_list(); });
}

SampleSelectionDialog::~SampleSelectionDialog() {
	for (string &name: icon_names)
		hui::delete_image(name);
}

void SampleSelectionDialog::fill_list() {
	reset(list_id);

	set_string(list_id, _("\\- none -\\"));
	set_int(list_id, 0);
	foreachi(Sample *s, weak(song->samples), i) {
		icon_names.add(render_sample(s, session->view));
		set_string(list_id, icon_names[i] + "\\" + song->get_time_str_long(s->buf->length) + "\\" + s->name);
		if (s == selected)
			set_int(list_id, i + 1);
	}
}

void SampleSelectionDialog::on_select() {
	int n = get_int("");
	selected = nullptr;
	if (n >= 1)
		selected = song->samples[n - 1].get();
	enable("ok", n >= 0);
}

void SampleSelectionDialog::on_list() {
	int n = get_int("");
	if (n == 0) {
		selected = nullptr;
		_promise.set_value(selected);
		request_destroy();
	} else if (n >= 1) {
		selected = song->samples[n - 1].get();
		_promise.set_value(selected);
		request_destroy();
	}
}

void SampleSelectionDialog::on_import() {
	session->storage->ask_open_import(win).on([this] (const Path &filename) {
		AudioBuffer buf;
		if (!session->storage->load_buffer(&buf, filename))
			return;
		song->create_sample_audio(filename.basename_no_ext(), buf);
		fill_list();
	});
}

void SampleSelectionDialog::on_ok() {
	_promise.set_value(selected);
	request_destroy();
}

void SampleSelectionDialog::on_cancel() {
	selected = _old;
	_promise.fail();
	request_destroy();
}

hui::future<Sample*> SampleSelectionDialog::select(Session *session, hui::Panel *parent, Sample *old) {
	auto s = new SampleSelectionDialog(session, parent, old);
	hui::fly(s);
	return s->_promise.get_future();
}
