/*
 * SampleManagerConsole.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Storage/Storage.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Device/OutputStream.h"
#include "../Helper/Progress.h"
#include "../Dialog/SampleScaleDialog.h"
#include "../../Session.h"
#include <math.h>

#include "../../Data/Song.h"
#include "../../lib/math/math.h"
#include "SampleManagerConsole.h"

#include "../../Module/Audio/BufferStreamer.h"
#include "../../Module/Audio/SongRenderer.h"


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
			im.setPixel(x, y, view->colors.text);
	}
}

void render_midi(Image &im, MidiNoteBuffer &m)
{
	int w = im.width;
	int h = im.height;
	Range r = Range(0, m.samples);
	MidiNoteBufferRef notes = m.getNotes(r);
	for (MidiNote *n: notes){
		float y = h * clampf((80 - n->pitch) / 50.0f, 0, 1);
		float x0 = w * clampf((float)n->range.offset / (float)r.length, 0, 1);
		float x1 = w * clampf((float)n->range.end() / (float)r.length, 0, 1);
		color c = AudioViewTrack::getPitchColor(n->pitch);
		for (int x=x0; x<=x1; x++)
			im.setPixel(x, y, c);
	}
}

string render_sample(Sample *s, AudioView *view)
{
	Image im;
	im.create(150, 32, color(0, 0, 0, 0));
	if (s->type == Track::Type::AUDIO)
		render_bufbox(im, s->buf, view);
	else if (s->type == Track::Type::MIDI)
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
		s->subscribe(this, std::bind(&SampleManagerItem::onUpdate, this));
	}
	virtual ~SampleManagerItem()
	{
		zombify();
	}
	virtual void onUpdate()
	{
		//msg_write("item:  " + message);
		if (s->cur_message() == s->MESSAGE_DELETE){
			manager->remove(this);
		}else{
			int n = manager->getIndex(s);
			if (n >= 0)
				manager->changeString("sample_list", n, str());
		}
	}

	void zombify()
	{
		if (s){
			s->unsubscribe(this);
			s = NULL;
			hui::DeleteImage(icon);
		}
	}

	string str()
	{
		return icon + "\\" + /*track_type(s->type) + "\\" +*/ s->name + "\\" + s->owner->get_time_str_long(s->range().length) + "\\" + format(_("%d times"), s->ref_count) + "\\" + b2s(s->auto_delete);
	}
	string icon;
	Sample *s;
	SampleManagerConsole *manager;
};

SampleManagerConsole::SampleManagerConsole(Session *session) :
	SideBarConsole(_("Samples"), session)
{
	fromResource("sample_manager_dialog");

	event("import_from_file", std::bind(&SampleManagerConsole::onImport, this));
	event("export_sample", std::bind(&SampleManagerConsole::onExport, this));
	event("preview_sample", std::bind(&SampleManagerConsole::onPreview, this));
	event("paste_sample", std::bind(&SampleManagerConsole::onInsert, this));
	event("create_from_selection", std::bind(&SampleManagerConsole::onCreateFromSelection, this));
	event("delete_sample", std::bind(&SampleManagerConsole::onDelete, this));
	event("scale_sample", std::bind(&SampleManagerConsole::onScale, this));
	eventX("sample_list", "hui:change", std::bind(&SampleManagerConsole::onListEdit, this));
	eventX("sample_list", "hui:select", std::bind(&SampleManagerConsole::onListSelect, this));
	event("sample_list", std::bind(&SampleManagerConsole::onPreview, this));

	event("edit_song", std::bind(&SampleManagerConsole::onEditSong, this));

	preview_renderer = NULL;
	preview_stream = NULL;
	preview_sample = NULL;

	progress = NULL;

	updateList();

	song->subscribe(this, std::bind(&SampleManagerConsole::onSongUpdate, this), song->MESSAGE_ADD_SAMPLE);
	song->subscribe(this, std::bind(&SampleManagerConsole::onSongUpdate, this), song->MESSAGE_DELETE_SAMPLE);
	song->subscribe(this, std::bind(&SampleManagerConsole::onSongUpdate, this), song->MESSAGE_NEW);
}

SampleManagerConsole::~SampleManagerConsole()
{
	for (SampleManagerItem *si: items)
		delete(si);
	items.clear();

	song->unsubscribe(this);
}

int SampleManagerConsole::getIndex(Sample *s)
{
	foreachi(SampleManagerItem *si, items, i)
		if (si->s == s)
			return i;
	return -1;
}

void SampleManagerConsole::updateList()
{
	// new samples?
	for (Sample *s: song->samples)
		if (getIndex(s) < 0)
			add(new SampleManagerItem(this, s, view));

	onListSelect();
}

void SampleManagerConsole::onListSelect()
{
	Array<Sample*> sel = getSelected();

	enable("export_sample", sel.num == 1);
	enable("preview_sample", sel.num == 1);
	enable("delete_sample", sel.num > 0);
	enable("paste_sample", sel.num == 1);
	enable("scale_sample", sel.num == 1);
}

void SampleManagerConsole::onListEdit()
{
	int sel = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (col == 1)
		song->editSampleName(items[sel]->s, getCell("sample_list", sel, 1));
	else if (col == 4)
		items[sel]->s->auto_delete = getCell("sample_list", sel, 4)._bool();
}

void SampleManagerConsole::onImport()
{
	if (session->storage->askOpenImport(win)){
		AudioBuffer buf;
		session->storage->loadBufferBox(song, &buf, hui::Filename);
		song->addSample(hui::Filename.basename(), buf);
		//setInt("sample_list", items.num - 1);
		onListSelect();
	}
}

void SampleManagerConsole::onExport()
{
	Array<Sample*> sel = getSelected();
	if (sel.num != 1)
		return;

	if (session->storage->askSaveExport(win)){
		if (sel[0]->type == Track::Type::AUDIO){
			BufferStreamer rr(&sel[0]->buf);
			session->storage->saveViaRenderer(rr.out, hui::Filename, sel[0]->buf.length, Array<Tag>());
		}
	}
}

void SampleManagerConsole::onInsert()
{
	Array<Sample*> sel = getSelected();
	for (Sample* s: sel)
		view->cur_track->addSampleRef(view->sel.range.start(), s);
}

void SampleManagerConsole::onCreateFromSelection()
{
	song->createSamplesFromSelection(view->sel);
}

void SampleManagerConsole::onDelete()
{
	Array<Sample*> sel = getSelected();

	song->action_manager->beginActionGroup();
	for (Sample* s: sel)
		song->deleteSample(s);
	song->action_manager->endActionGroup();

	// hui bug
	setInt("sample_list", -1);
}


void SampleManagerConsole::onScale()
{
	Array<Sample*> sel = getSelected();
	for (Sample* s: sel){
		if (s->type != Track::Type::AUDIO)
			continue;
		SampleScaleDialog *dlg = new SampleScaleDialog(parent->win, s);
		dlg->run();
		delete(dlg);
	}
}

void SampleManagerConsole::add(SampleManagerItem *item)
{
	items.add(item);
	addString("sample_list", item->str());
}

void SampleManagerConsole::remove(SampleManagerItem *item)
{
	foreachi(SampleManagerItem *si, items, i)
		if (si == item){
			items.erase(i);
			removeString("sample_list", i);

			// don't delete now... we're still in notify()?
			item->zombify();
			old_items.add(item);
		}
}

Array<Sample*> SampleManagerConsole::getSelected()
{
	Array<int> indices = getSelection("sample_list");
	Array<Sample*> sel;
	for (int i: indices)
		sel.add(items[i]->s);
	return sel;
}

void SampleManagerConsole::setSelection(const Array<Sample*> &samples)
{
	Array<int> indices;
	for (Sample *s: samples)
		indices.add(getIndex(s));
	hui::Panel::setSelection("sample_list", indices);
}

void SampleManagerConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void SampleManagerConsole::onProgressCancel()
{
	if (progress)
		endPreview();
}

void SampleManagerConsole::onSongUpdate()
{
	updateList();
}

void SampleManagerConsole::onPreviewStreamUpdate()
{
	int pos = preview_stream->get_pos();
	Range r = preview_sample->range();
	progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
}

void SampleManagerConsole::onPreviewStreamEnd()
{
	endPreview();
}

void SampleManagerConsole::onPreview()
{
	if (progress)
		endPreview();
	int sel = getInt("sample_list");
	preview_sample = items[sel]->s;
	preview_renderer = new BufferStreamer(&preview_sample->buf);
	preview_stream = new OutputStream(session, preview_renderer->out);

	progress = new ProgressCancelable(_("Preview"), win);
	progress->subscribe(this, std::bind(&SampleManagerConsole::onProgressCancel, this));
	preview_stream->subscribe(this, std::bind(&SampleManagerConsole::onPreviewStreamUpdate, this));
	preview_stream->subscribe(this, std::bind(&SampleManagerConsole::onPreviewStreamEnd, this), preview_stream->MESSAGE_PLAY_END_OF_STREAM);
	preview_stream->play();
}

void SampleManagerConsole::endPreview()
{
	if (progress){
		progress->unsubscribe(this);
		delete(progress);
		progress = NULL;
	}
	preview_stream->unsubscribe(this);
	preview_stream->stop();
	delete(preview_stream);
	delete(preview_renderer);
	preview_sample = NULL;
}


class SampleSelector : public hui::Dialog
{
public:
	SampleSelector(Session *session, hui::Panel *parent, Sample *old) :
		hui::Dialog("", 300, 400, parent->win, false)
	{
		song = session->song;
		ret = NULL;;
		_old = old;

		fromResource("sample_selection_dialog");

		list_id = "sample_selection_list";

		setString(list_id, _("\\- none -\\"));
		setInt(list_id, 0);
		foreachi(Sample *s, song->samples, i){
			icon_names.add(render_sample(s, session->view));
			setString(list_id, icon_names[i] + "\\" + s->name + "\\" + song->get_time_str_long(s->buf.length));
			if (s == old)
				setInt(list_id, i + 1);
		}

		event("ok", std::bind(&SampleSelector::onOk, this));
		event("cancel", std::bind(&SampleSelector::onCancel, this));
		event("hui:close", std::bind(&SampleSelector::onCancel, this));
		eventX(list_id, "hui:select", std::bind(&SampleSelector::onSelect, this));
		event(list_id, std::bind(&SampleSelector::onList, this));
	}
	virtual ~SampleSelector()
	{
		for (string &name: icon_names)
			hui::DeleteImage(name);
	}

	void onSelect()
	{
		int n = getInt("");
		ret = NULL;
		if (n >= 1)
			ret = song->samples[n - 1];
		enable("ok", n >= 0);
	}

	void onList()
	{
		int n = getInt("");
		if (n == 0){
			ret = NULL;
			destroy();
		}else if (n >= 1){
			ret = song->samples[n - 1];
			destroy();
		}
	}

	void onOk()
	{
		destroy();
	}

	void onCancel()
	{
		ret = _old;
		destroy();
	}

	Sample *ret;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	string list_id;
};

Sample *SampleManagerConsole::select(Session *session, hui::Panel *parent, Sample *old)
{
	SampleSelector *s = new SampleSelector(session, parent, old);
	s->run();
	Sample *r = s->ret;
	delete(s);
	return r;
}
