/*
 * SampleManagerConsole.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleManagerConsole.h"
#include "../audioview/AudioView.h"
#include "../painter/MidiPainter.h"
#include "../dialog/SampleScaleDialog.h"
#include "../helper/SamplePreviewPlayer.h"
#include "../ColorScheme.h"
#include "../../action/ActionManager.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/TrackLayer.h"
#include "../../data/Sample.h"
#include "../../data/SampleRef.h"
#include "../../module/audio/BufferStreamer.h"
#include "../../storage/Storage.h"
#include "../../lib/base/iter.h"
#include "../../lib/math/math.h"
#include "../../lib/image/image.h"
#include "../../lib/os/msg.h"
#include "../../lib/hui/language.h"
#include "../../lib/hui/Menu.h"
#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include <cmath>

namespace tsunami {


static const int SAMPLE_PREVIEW_WIDTH = 120;
static const int SAMPLE_PREVIEW_HEIGHT = 36;

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

// TODO find a good strategy to delete obsolete images
// for now, assume we only have a limited number of samples/uids...
string get_sample_preview(Sample *s, AudioView *view) {
	Image im;
	im.create(SAMPLE_PREVIEW_WIDTH, SAMPLE_PREVIEW_HEIGHT, color(0, 0, 0, 0));
	if (s->type == SignalType::Audio)
		render_bufbox(im, *s->buf, view);
	else if (s->type == SignalType::Midi)
		render_midi(im, s->midi);
	string id = "image:sample-" + i2s(s->uid);
	return hui::set_image(&im, id);
}

class SampleManagerItem : public obs::Node<VirtualBase> {
public:
	SampleManagerItem(SampleManagerConsole *_manager, Sample *_s, AudioView *_view) {
		manager = _manager;
		s = _s;
		view = _view;
		icon = get_sample_preview(s, view);
		s->out_death >> create_sink([this] { on_delete(); });
		s->out_changed_by_action >> create_sink([this] { on_update(); });
		s->out_reference >> create_sink([this] { on_update(); });
		s->out_unreference >> create_sink([this] { on_update(); });
	}
	~SampleManagerItem() override {
		zombify();
	}
	void on_delete() {
		hui::run_later(0.001f, [this] { manager->remove(this); });
	}
	void on_update() {
		if (manager->editing_cell)
			return;
		int n = manager->get_index(s);
		if (n >= 0) {
			icon = get_sample_preview(s, view);
			manager->change_string(manager->id_list, n, str());
		}
	}

	void zombify() {
		if (s) {
			s->unsubscribe(this);
			s = nullptr;
			hui::delete_image(icon);
		}
	}

	string str() {
		string e;
		if (s->auto_delete)
			e = " *";
		string usage = format("%dx used", s->ref_count);
		if (s->ref_count == 0)
			usage = "unused";
		return format("%s\\%s\\%s%s\\<span alpha=\"50%%\"><i>%s</i></span>", s->name, icon, s->owner->get_time_str_long(s->range().length), e, usage);
	}
	string icon;
	Sample *s;
	SampleManagerConsole *manager;
	AudioView *view;
};

SampleManagerConsole::SampleManagerConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Samples"), "sample-manager-console", session, bar),
	in_song_update{this, [this] { on_song_update(); }}
{
	from_resource("sample-manager-dialog");

	menu_samples = hui::create_resource_menu("popup-menu-sample-manager", this);

	id_list = "sample-list";
	event("import-from-file", [this] { on_import(); });
	event("create-from-selection", [this] { on_create_from_selection(); });
	event("sample-delete", [this] { on_delete(); });
	event("sample-scale", [this] { on_scale(); });
	event("sample-auto-delete", [this] { on_auto_delete(); });
	event("sample-export", [this] { on_export(); });
	event("sample-preview", [this] { on_preview(); });
	event("sample-paste", [this] { on_insert(); });
	event_x(id_list, "hui:change", [this] { on_list_edit(); });
	event_x(id_list, "hui:right-button-down", [this] { on_list_right_click(); });
	event(id_list, [this] { on_preview(); });
}

void SampleManagerConsole::on_enter() {
	update_list();

	song->out_sample_list_changed >> in_song_update;
	song->out_new >> in_song_update;
}

void SampleManagerConsole::on_leave() {
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
	editing_cell = true;
	int sel = hui::get_event()->row;
	int col = hui::get_event()->column;
	if (col == 0)
		song->edit_sample_name(items[sel]->s, get_cell(id_list, sel, col));
	//else if (col == 4)
	//	items[sel]->s->auto_delete = get_cell(id_list, sel, col)._bool();
	editing_cell = false;
}

void SampleManagerConsole::on_list_right_click() {
	int n = hui::get_event()->row;
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
		s->out_changed.notify();
	}
}

void SampleManagerConsole::on_import() {
	session->storage->ask_open_import(win).then([this] (const Path &filename) {
		session->storage->load_buffer(filename).then([this, filename] (const AudioBuffer& buf) {
			song->create_sample_audio(filename.basename_no_ext(), buf);
			//setInt("sample_list", items.num - 1);
		});
	});
}

void SampleManagerConsole::on_export() {
	auto sel = get_selected();
	if (sel.num != 1)
		return;

	session->storage->ask_save_render_export(win).then([this, sel] (const Path &filename) {
		if (sel[0]->type == SignalType::Audio) {
			BufferStreamer rr(sel[0]->buf);
			session->storage->save_via_renderer(rr.out, filename, sel[0]->buf->length, {});
		}
	});
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
			if (l->type == SignalType::Audio) {
				AudioBuffer buf;
				buf.resize(view->sel.range().length);
				l->read_buffers_fixed(buf, view->sel.range());
				song->create_sample_audio("-new-", buf);
			} else if (l->type == SignalType::Midi) {
				auto buf = l->midi.get_notes(view->sel.range());
				for (auto *n: weak(buf))
					n->range.offset -= view->sel.range().offset;
				song->create_sample_midi("-new-", buf);
			}
		}
}

void SampleManagerConsole::on_delete() {
	auto sel = get_selected();

	song->begin_action_group(_("delete samples"));
	for (Sample* s: sel) {
		try {
			song->delete_sample(s);
		} catch (Exception &e) {
			session->e(e.message());
		}
	}
	song->end_action_group();

	// hui bug
	set_int(id_list, -1);
}


void SampleManagerConsole::on_scale() {
	auto sel = get_selected();
	for (Sample* s: sel) {
		if (s->type != SignalType::Audio)
			continue;
		hui::fly(new SampleScaleDialog(parent->win, s));
	}
}

void SampleManagerConsole::add(SampleManagerItem *item) {
	items.add(item);
	add_string("sample-list", item->str());
}

void SampleManagerConsole::remove(SampleManagerItem *item) {
	for (auto&& [i, si]: enumerate(items))
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

void SampleManagerConsole::on_song_update() {
	if (!editing_cell)
		update_list();
}

void SampleManagerConsole::on_preview() {
	int sel = get_int(id_list);
	if (sel < 0)
		return;
	SamplePreviewPlayer::play(win, session, items[sel]->s);
}

}

