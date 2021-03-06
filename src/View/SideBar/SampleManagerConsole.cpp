/*
 * SampleManagerConsole.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Storage/Storage.h"
#include "../AudioView.h"
#include "../Painter/MidiPainter.h"
#include "../Helper/Progress.h"
#include "../Dialog/SampleScaleDialog.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include <math.h>

#include "../../Action/ActionManager.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Sample.h"
#include "../../lib/math/math.h"
#include "SampleManagerConsole.h"

#include "../../Device/Stream/AudioOutput.h"
#include "../../Module/SignalChain.h"
#include "../../Module/Audio/BufferStreamer.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../../Module/Midi/MidiEventStreamer.h"
#include "../Graph/AudioViewLayer.h"


// TODO: use BufferPainter / MidiPainter
void render_bufbox(Image &im, AudioBuffer &b, AudioView *view) {
	int w = im.width;
	int h = im.height;
	for (int x=0; x<w; x++) {
		float m = 0;
		int i0 = (b.length * x) / w;
		int i1 = (b.length * (x + 1)) / w;
		for (int i=i0; i<i1; i++)
			m = max(m, (float)fabs(b.c[0][i]));
		for (int y=h*(1-m)/2; y<=h*(1+m)/2; y++)
			im.set_pixel(x, y, theme.text);
	}
}

void render_midi(Image &im, MidiNoteBuffer &m) {
	int w = im.width;
	int h = im.height;
	for (MidiNote *n: weak(m)) {
		float y = h * clamp((80 - n->pitch) / 50.0f, 0.0f, 1.0f);
		float x0 = w * clamp((float)n->range.offset / (float)m.samples, 0.0f, 1.0f);
		float x1 = w * clamp((float)n->range.end() / (float)m.samples, 0.0f, 1.0f);
		color c = theme.pitch[(int)n->pitch % 12];
		for (int x=x0; x<=x1; x++)
			im.set_pixel(x, y, c);
	}
}

string render_sample(Sample *s, AudioView *view) {
	Image im;
	im.create(120, 36, color(0, 0, 0, 0));
	if (s->type == SignalType::AUDIO)
		render_bufbox(im, *s->buf, view);
	else if (s->type == SignalType::MIDI)
		render_midi(im, s->midi);
	string id = "image:sample-" + i2s(s->uid);
	return hui::SetImage(&im, id);
}

class SampleManagerItem : public VirtualBase {
public:
	SampleManagerItem(SampleManagerConsole *_manager, Sample *_s, AudioView *_view) {
		manager = _manager;
		s = _s;
		view = _view;
		icon = render_sample(s, view);
		s->subscribe(this, [=]{ on_delete(); }, s->MESSAGE_DELETE);
		s->subscribe(this, [=]{ on_update(); }, s->MESSAGE_CHANGE_BY_ACTION);
		s->subscribe(this, [=]{ on_update(); }, s->MESSAGE_REFERENCE);
		s->subscribe(this, [=]{ on_update(); }, s->MESSAGE_UNREFERENCE);
	}
	virtual ~SampleManagerItem() {
		zombify();
	}
	void on_delete() {
		hui::RunLater(0.001f, [=]{ manager->remove(this); });
	}
	void on_update() {
		int n = manager->get_index(s);
		if (n >= 0) {
			icon = render_sample(s, view);
			manager->change_string(manager->id_list, n, str());
		}
	}

	void zombify() {
		if (s) {
			s->unsubscribe(this);
			s = nullptr;
			hui::DeleteImage(icon);
		}
	}

	string str() {
		string e;
		if (s->auto_delete)
			e = " *";
		return format("%s\\%s\\%s%s\\%d times", icon, s->owner->get_time_str_long(s->range().length), s->name, e, s->ref_count);
	}
	string icon;
	Sample *s;
	SampleManagerConsole *manager;
	AudioView *view;
};

SampleManagerConsole::SampleManagerConsole(Session *session) :
	SideBarConsole(_("Samples"), session)
{
	from_resource("sample-manager-dialog");

	menu_samples = hui::CreateResourceMenu("popup-menu-sample-manager");

	id_list = "sample-list";
	event("import-from-file", [=]{ on_import(); });
	event("create-from-selection", [=]{ on_create_from_selection(); });
	event("sample-delete", [=]{ on_delete(); });
	event("sample-scale", [=]{ on_scale(); });
	event("sample-auto-delete", [=]{ on_auto_delete(); });
	event("sample-export", [=]{ on_export(); });
	event("sample-preview", [=]{ on_preview(); });
	event("sample-paste", [=]{ on_insert(); });
	event_x(id_list, "hui:change", [=]{ on_list_edit(); });
	event_x(id_list, "hui:right-button-down", [=]{ on_list_right_click(); });
	event("sample-list", [=]{ on_preview(); });

	event("edit_song", [=]{ session->set_mode(EditMode::DefaultSong); });

	progress = nullptr;

	update_list();

	song->subscribe(this, [=]{ on_song_update(); }, song->MESSAGE_ADD_SAMPLE);
	song->subscribe(this, [=]{ on_song_update(); }, song->MESSAGE_DELETE_SAMPLE);
	song->subscribe(this, [=]{ on_song_update(); }, song->MESSAGE_NEW);
}

SampleManagerConsole::~SampleManagerConsole() {
	song->unsubscribe(this);
}

int SampleManagerConsole::get_index(Sample *s) {
	foreachi(auto *si, items, i)
		if (si->s == s)
			return i;
	return -1;
}

void SampleManagerConsole::update_list() {
	// new samples?
	for (Sample *s: weak(song->samples))
		if (get_index(s) < 0)
			add(new SampleManagerItem(this, s, view));
}

void SampleManagerConsole::on_list_edit() {
	int sel = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (col == 2)
		song->edit_sample_name(items[sel]->s, get_cell(id_list, sel, col));
	//else if (col == 4)
	//	items[sel]->s->auto_delete = get_cell(id_list, sel, col)._bool();
}

void SampleManagerConsole::on_list_right_click() {
	int n = hui::GetEvent()->row;
	menu_samples->enable("sample-preview", n >= 0);
	menu_samples->enable("sample-export", n >= 0);
	menu_samples->enable("sample-paste", n >= 0);
	menu_samples->enable("sample-scale", n >= 0);
	menu_samples->enable("sample-delete", n >= 0);
	menu_samples->enable("sample-auto-delete", n >= 0);
	if (n >= 0)
		menu_samples->check("sample-auto-delete", items[n]->s->auto_delete);
	menu_samples->open_popup(this);
}

void SampleManagerConsole::on_auto_delete() {
	auto sel = get_selected();
	for (auto *s: sel) {
		s->auto_delete = !s->auto_delete;
		s->notify();
	}
}

void SampleManagerConsole::on_import() {
	if (!session->storage->ask_open_import(win))
		return;
	AudioBuffer buf;
	if (!session->storage->load_buffer(&buf, hui::Filename))
		return;
	song->create_sample_audio(hui::Filename.basename_no_ext(), buf);
	//setInt("sample_list", items.num - 1);
}

void SampleManagerConsole::on_export() {
	auto sel = get_selected();
	if (sel.num != 1)
		return;

	if (session->storage->ask_save_render_export(win)){
		if (sel[0]->type == SignalType::AUDIO){
			BufferStreamer rr(sel[0]->buf);
			session->storage->save_via_renderer(rr.port_out[0], hui::Filename, sel[0]->buf->length, {});
		}
	}
}

void SampleManagerConsole::on_insert() {
	auto sel = get_selected();
	for (Sample* s: sel)
		view->cur_layer()->add_sample_ref(view->cursor_pos(), s);
}

void SampleManagerConsole::on_create_from_selection() {
	if (view->sel.range().length == 0)
		return;
	for (auto *l: song->layers())
		if (view->sel.has(l)) {
			if (l->type == SignalType::AUDIO) {
				AudioBuffer buf;
				buf.resize(view->sel.range().length);
				l->read_buffers_fixed(buf, view->sel.range());
				song->create_sample_audio("-new-", buf);
			} else if (l->type == SignalType::MIDI) {
				auto buf = l->midi.get_notes(view->sel.range());
				for (auto *n: weak(buf))
					n->range.offset -= view->sel.range().offset;
				song->create_sample_midi("-new-", buf);
			}
		}
}

void SampleManagerConsole::on_delete() {
	auto sel = get_selected();

	song->action_manager->group_begin();
	for (Sample* s: sel) {
		try {
			song->delete_sample(s);
		} catch (Exception &e) {
			session->e(e.message());
		}
	}
	song->action_manager->group_end();

	// hui bug
	set_int(id_list, -1);
}


void SampleManagerConsole::on_scale() {
	auto sel = get_selected();
	for (Sample* s: sel) {
		if (s->type != SignalType::AUDIO)
			continue;
		auto dlg = ownify(new SampleScaleDialog(parent->win, s));
		dlg->run();
	}
}

void SampleManagerConsole::add(SampleManagerItem *item) {
	items.add(item);
	add_string("sample-list", item->str());
}

void SampleManagerConsole::remove(SampleManagerItem *item) {
	foreachi(auto *si, items, i)
		if (si == item) {
			msg_write("--------SampleManagerConsole erase...");
			items.erase(i);
			remove_string(id_list, i);

			// don't delete now... we're still in notify()?  nope!
			//item->zombify();
			old_items.add(item);
		}
}

Array<Sample*> SampleManagerConsole::get_selected() {
	auto indices = get_selection(id_list);
	Array<Sample*> sel;
	for (int i: indices)
		sel.add(items[i]->s);
	return sel;
}

void SampleManagerConsole::set_selection(const Array<Sample*> &samples) {
	Array<int> indices;
	for (Sample *s: samples)
		indices.add(get_index(s));
	hui::Panel::set_selection(id_list, indices);
}

void SampleManagerConsole::on_progress_cancel() {
	if (progress)
		end_preview();
}

void SampleManagerConsole::on_song_update() {
	update_list();
}

void SampleManagerConsole::on_preview_tick() {
	int pos = 0;
	if (preview.sample->type == SignalType::AUDIO) {
		pos = preview.renderer->get_pos();
	} else {
		pos = preview.midi_streamer->get_pos();
	}
	Range r = preview.sample->range();
	progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
}

void SampleManagerConsole::on_preview_stream_end() {
	hui::RunLater(0.1f, [=]{ end_preview(); });
}

void SampleManagerConsole::on_preview() {
	if (progress)
		end_preview();
	int sel = get_int(id_list);
	preview.sample = items[sel]->s;
	preview.chain = session->create_signal_chain_system("sample-preview");
	if (preview.sample->type == SignalType::AUDIO) {
		preview.renderer = new BufferStreamer(preview.sample->buf);
		preview.chain->_add(preview.renderer);
		preview.stream = (AudioOutput*)preview.chain->add(ModuleCategory::STREAM, "AudioOutput");
		preview.chain->connect(preview.renderer, 0, preview.stream, 0);
	} else { // MIDI
		preview.midi_streamer = new MidiEventStreamer(midi_notes_to_events(preview.sample->midi));
		preview.chain->_add(preview.midi_streamer);
		auto *synth = preview.chain->add(ModuleCategory::SYNTHESIZER, "");
		preview.stream = (AudioOutput*)preview.chain->add(ModuleCategory::STREAM, "AudioOutput");
		preview.chain->connect(preview.midi_streamer, 0, synth, 0);
		preview.chain->connect(synth, 0, preview.stream, 0);
	}

	progress = new ProgressCancelable(_("Preview"), win);
	progress->subscribe(this, [=]{ on_progress_cancel(); });
	preview.chain->subscribe(this, [=]{ on_preview_tick(); });
	preview.chain->subscribe(this, [=]{ on_preview_stream_end(); }, Module::MESSAGE_PLAY_END_OF_STREAM);
	preview.chain->start();
}

void SampleManagerConsole::end_preview() {
	if (progress) {
		progress->unsubscribe(this);
		progress = nullptr;
	}
	preview.chain->unsubscribe(this);
	preview.stream->unsubscribe(this);
	preview.chain->stop();
	preview.chain->unregister();
	preview.sample = nullptr;
}


class SampleSelector : public hui::Dialog {
public:
	SampleSelector(Session *_session, hui::Panel *parent, Sample *old) :
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

		event("import", [=]{ on_import(); });
		event("ok", [=]{ on_ok(); });
		event("cancel", [=]{ on_cancel(); });
		event("hui:close", [=]{ on_cancel(); });
		event_x(list_id, "hui:select", [=]{ on_select(); });
		event(list_id, [=]{ on_list(); });
	}
	virtual ~SampleSelector() {
		for (string &name: icon_names)
			hui::DeleteImage(name);
	}

	void fill_list() {
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

	void on_select() {
		int n = get_int("");
		selected = nullptr;
		if (n >= 1)
			selected = song->samples[n - 1].get();
		enable("ok", n >= 0);
	}

	void on_list() {
		int n = get_int("");
		if (n == 0) {
			selected = nullptr;
			request_destroy();
		} else if (n >= 1) {
			selected = song->samples[n - 1].get();
			request_destroy();
		}
	}

	void on_import() {
		if (!session->storage->ask_open_import(win))
			return;
		AudioBuffer buf;
		if (!session->storage->load_buffer(&buf, hui::Filename))
			return;
		song->create_sample_audio(hui::Filename.basename_no_ext(), buf);
		fill_list();

	}

	void on_ok() {
		request_destroy();
	}

	void on_cancel() {
		selected = _old;
		request_destroy();
	}

	Sample *selected;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	Session *session;
	string list_id;
};

Sample *SampleManagerConsole::select(Session *session, hui::Panel *parent, Sample *old) {
	auto s = ownify(new SampleSelector(session, parent, old));
	s->run();
	return s->selected;
}
