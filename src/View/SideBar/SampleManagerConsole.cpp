/*
 * SampleManagerConsole.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Storage/Storage.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../Painter/MidiPainter.h"
#include "../../Device/OutputStream.h"
#include "../Helper/Progress.h"
#include "../Dialog/SampleScaleDialog.h"
#include "../../Session.h"
#include <math.h>

#include "../../Action/ActionManager.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Sample.h"
#include "../../lib/math/math.h"
#include "SampleManagerConsole.h"

#include "../../Module/Audio/BufferStreamer.h"
#include "../../Module/Audio/SongRenderer.h"


// TODO: use BufferPainter / MidiPainter
void render_bufbox(Image &im, AudioBuffer &b, AudioView *view)
{
	int w = im.width;
	int h = im.height;
	for (int x=0; x<w; x++){
		float m = 0;
		int i0 = (b.length * x) / w;
		int i1 = (b.length * (x + 1)) / w;
		for (int i=i0; i<i1; i++)
			m = max(m, (float)fabs(b.c[0][i]));
		for (int y=h*(1-m)/2; y<h*(1+m)/2; y++)
			im.set_pixel(x, y, view->colors.text);
	}
}

void render_midi(Image &im, MidiNoteBuffer &m)
{
	int w = im.width;
	int h = im.height;
	Range r = Range(0, m.samples);
	MidiNoteBufferRef notes = m.get_notes(r);
	for (MidiNote *n: notes){
		float y = h * clampf((80 - n->pitch) / 50.0f, 0, 1);
		float x0 = w * clampf((float)n->range.offset / (float)r.length, 0, 1);
		float x1 = w * clampf((float)n->range.end() / (float)r.length, 0, 1);
		color c = MidiPainter::pitch_color(n->pitch);
		for (int x=x0; x<=x1; x++)
			im.set_pixel(x, y, c);
	}
}

string render_sample(Sample *s, AudioView *view)
{
	Image im;
	im.create(120, 32, color(0, 0, 0, 0));
	if (s->type == SignalType::AUDIO)
		render_bufbox(im, s->buf, view);
	else if (s->type == SignalType::MIDI)
		render_midi(im, s->midi);
	return hui::SetImage(im);
}

class SampleManagerItem : public VirtualBase
{
public:
	SampleManagerItem(SampleManagerConsole *_manager, Sample *_s, AudioView *view)
	{
		manager = _manager;
		s = _s;
		icon = render_sample(s, view);
		s->subscribe(this, std::bind(&SampleManagerItem::on_update, this));
	}
	virtual ~SampleManagerItem()
	{
		zombify();
	}
	void on_update()
	{
		//msg_write("item:  " + message);
		if (s->cur_message() == s->MESSAGE_DELETE){
			manager->remove(this);
		}else{
			int n = manager->get_index(s);
			if (n >= 0)
				manager->change_string("sample_list", n, str());
		}
	}

	void zombify()
	{
		if (s){
			s->unsubscribe(this);
			s = nullptr;
			hui::DeleteImage(icon);
		}
	}

	string str()
	{
		return icon + "\\" + /*track_type(s->type) + "\\" +*/ s->owner->get_time_str_long(s->range().length) + "\\" + s->name + "\\" + format(_("%d times"), s->ref_count) + "\\" + b2s(s->auto_delete);
	}
	string icon;
	Sample *s;
	SampleManagerConsole *manager;
};

SampleManagerConsole::SampleManagerConsole(Session *session) :
	SideBarConsole(_("Samples"), session)
{
	from_resource("sample_manager_dialog");

	event("import_from_file", std::bind(&SampleManagerConsole::on_import, this));
	event("export_sample", std::bind(&SampleManagerConsole::on_export, this));
	event("preview_sample", std::bind(&SampleManagerConsole::on_preview, this));
	event("paste_sample", std::bind(&SampleManagerConsole::on_insert, this));
	event("create_from_selection", std::bind(&SampleManagerConsole::on_create_from_selection, this));
	event("delete_sample", std::bind(&SampleManagerConsole::on_delete, this));
	event("scale_sample", std::bind(&SampleManagerConsole::on_scale, this));
	event_x("sample_list", "hui:change", std::bind(&SampleManagerConsole::on_list_edit, this));
	event_x("sample_list", "hui:select", std::bind(&SampleManagerConsole::on_list_select, this));
	event("sample_list", std::bind(&SampleManagerConsole::on_preview, this));

	event("edit_song", std::bind(&SampleManagerConsole::on_edit_song, this));

	preview_renderer = nullptr;
	preview_stream = nullptr;
	preview_sample = nullptr;

	progress = nullptr;

	update_list();

	song->subscribe(this, std::bind(&SampleManagerConsole::on_song_update, this), song->MESSAGE_ADD_SAMPLE);
	song->subscribe(this, std::bind(&SampleManagerConsole::on_song_update, this), song->MESSAGE_DELETE_SAMPLE);
	song->subscribe(this, std::bind(&SampleManagerConsole::on_song_update, this), song->MESSAGE_NEW);
}

SampleManagerConsole::~SampleManagerConsole()
{
	for (SampleManagerItem *si: items)
		delete(si);
	items.clear();

	song->unsubscribe(this);
}

int SampleManagerConsole::get_index(Sample *s)
{
	foreachi(SampleManagerItem *si, items, i)
		if (si->s == s)
			return i;
	return -1;
}

void SampleManagerConsole::update_list()
{
	// new samples?
	for (Sample *s: song->samples)
		if (get_index(s) < 0)
			add(new SampleManagerItem(this, s, view));

	on_list_select();
}

void SampleManagerConsole::on_list_select()
{
	Array<Sample*> sel = get_selected();

	enable("export_sample", sel.num == 1);
	enable("preview_sample", sel.num == 1);
	enable("delete_sample", sel.num > 0);
	enable("paste_sample", sel.num == 1);
	enable("scale_sample", sel.num == 1);
}

void SampleManagerConsole::on_list_edit()
{
	int sel = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (col == 1)
		song->edit_sample_name(items[sel]->s, get_cell("sample_list", sel, 1));
	else if (col == 4)
		items[sel]->s->auto_delete = get_cell("sample_list", sel, 4)._bool();
}

void SampleManagerConsole::on_import()
{
	if (session->storage->ask_open_import(win)){
		AudioBuffer buf;
		session->storage->load_buffer(song, &buf, hui::Filename);
		song->add_sample(hui::Filename.basename(), buf);
		//setInt("sample_list", items.num - 1);
		on_list_select();
	}
}

void SampleManagerConsole::on_export()
{
	Array<Sample*> sel = get_selected();
	if (sel.num != 1)
		return;

	if (session->storage->ask_save_export(win)){
		if (sel[0]->type == SignalType::AUDIO){
			BufferStreamer rr(&sel[0]->buf);
			session->storage->save_via_renderer(rr.out, hui::Filename, sel[0]->buf.length, {});
		}
	}
}

void SampleManagerConsole::on_insert()
{
	Array<Sample*> sel = get_selected();
	for (Sample* s: sel)
		view->cur_layer()->add_sample_ref(view->sel.range.start(), s);
}

void SampleManagerConsole::on_create_from_selection()
{
	song->create_samples_from_selection(view->sel, false);
}

void SampleManagerConsole::on_delete()
{
	Array<Sample*> sel = get_selected();

	song->action_manager->group_begin();
	for (Sample* s: sel)
		song->delete_sample(s);
	song->action_manager->group_end();

	// hui bug
	set_int("sample_list", -1);
}


void SampleManagerConsole::on_scale()
{
	Array<Sample*> sel = get_selected();
	for (Sample* s: sel){
		if (s->type != SignalType::AUDIO)
			continue;
		SampleScaleDialog *dlg = new SampleScaleDialog(parent->win, s);
		dlg->run();
		delete(dlg);
	}
}

void SampleManagerConsole::add(SampleManagerItem *item)
{
	items.add(item);
	add_string("sample_list", item->str());
}

void SampleManagerConsole::remove(SampleManagerItem *item)
{
	foreachi(SampleManagerItem *si, items, i)
		if (si == item){
			items.erase(i);
			remove_string("sample_list", i);

			// don't delete now... we're still in notify()?
			item->zombify();
			old_items.add(item);
		}
}

Array<Sample*> SampleManagerConsole::get_selected()
{
	Array<int> indices = get_selection("sample_list");
	Array<Sample*> sel;
	for (int i: indices)
		sel.add(items[i]->s);
	return sel;
}

void SampleManagerConsole::set_selection(const Array<Sample*> &samples)
{
	Array<int> indices;
	for (Sample *s: samples)
		indices.add(get_index(s));
	hui::Panel::set_selection("sample_list", indices);
}

void SampleManagerConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void SampleManagerConsole::on_progress_cancel()
{
	if (progress)
		end_preview();
}

void SampleManagerConsole::on_song_update()
{
	update_list();
}

void SampleManagerConsole::on_preview_stream_update()
{
	int pos = preview_stream->get_pos();
	Range r = preview_sample->range();
	progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
}

void SampleManagerConsole::on_preview_stream_end()
{
	end_preview();
}

void SampleManagerConsole::on_preview()
{
	if (progress)
		end_preview();
	int sel = get_int("sample_list");
	preview_sample = items[sel]->s;
	preview_renderer = new BufferStreamer(&preview_sample->buf);
	preview_stream = new OutputStream(session);
	preview_stream->plug(0, preview_renderer, 0);

	progress = new ProgressCancelable(_("Preview"), win);
	progress->subscribe(this, std::bind(&SampleManagerConsole::on_progress_cancel, this));
	preview_stream->subscribe(this, std::bind(&SampleManagerConsole::on_preview_stream_update, this));
	preview_stream->subscribe(this, std::bind(&SampleManagerConsole::on_preview_stream_end, this), preview_stream->MESSAGE_PLAY_END_OF_STREAM);
	preview_stream->play();
}

void SampleManagerConsole::end_preview()
{
	if (progress){
		progress->unsubscribe(this);
		delete(progress);
		progress = nullptr;
	}
	preview_stream->unsubscribe(this);
	preview_stream->stop();
	delete(preview_stream);
	delete(preview_renderer);
	preview_sample = nullptr;
}


class SampleSelector : public hui::Dialog
{
public:
	SampleSelector(Session *_session, hui::Panel *parent, Sample *old) :
		hui::Dialog("", 300, 400, parent->win, false)
	{
		session = _session;
		song = session->song;
		selected = nullptr;
		_old = old;
		for (Sample *s: song->samples)
			if (s == old)
				selected = s;


		from_resource("sample_selection_dialog");

		list_id = "sample_selection_list";

		fill_list();

		event("import", std::bind(&SampleSelector::on_import, this));
		event("ok", std::bind(&SampleSelector::on_ok, this));
		event("cancel", std::bind(&SampleSelector::on_cancel, this));
		event("hui:close", std::bind(&SampleSelector::on_cancel, this));
		event_x(list_id, "hui:select", std::bind(&SampleSelector::on_select, this));
		event(list_id, std::bind(&SampleSelector::on_list, this));
	}
	virtual ~SampleSelector()
	{
		for (string &name: icon_names)
			hui::DeleteImage(name);
	}

	void fill_list()
	{
		reset(list_id);

		set_string(list_id, _("\\- none -\\"));
		set_int(list_id, 0);
		foreachi(Sample *s, song->samples, i){
			icon_names.add(render_sample(s, session->view));
			set_string(list_id, icon_names[i] + "\\" + song->get_time_str_long(s->buf.length) + "\\" + s->name);
			if (s == selected)
				set_int(list_id, i + 1);
		}
	}

	void on_select()
	{
		int n = get_int("");
		selected = nullptr;
		if (n >= 1)
			selected = song->samples[n - 1];
		enable("ok", n >= 0);
	}

	void on_list()
	{
		int n = get_int("");
		if (n == 0){
			selected = nullptr;
			destroy();
		}else if (n >= 1){
			selected = song->samples[n - 1];
			destroy();
		}
	}

	void on_import()
	{
		if (session->storage->ask_open_import(win)){
			AudioBuffer buf;
			session->storage->load_buffer(song, &buf, hui::Filename);
			song->add_sample(hui::Filename.basename(), buf);
			fill_list();
		}

	}

	void on_ok()
	{
		destroy();
	}

	void on_cancel()
	{
		selected = _old;
		destroy();
	}

	Sample *selected;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	Session *session;
	string list_id;
};

Sample *SampleManagerConsole::select(Session *session, hui::Panel *parent, Sample *old)
{
	SampleSelector *s = new SampleSelector(session, parent, old);
	s->run();
	Sample *r = s->selected;
	delete(s);
	return r;
}
