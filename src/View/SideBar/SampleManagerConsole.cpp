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
#include "../../Audio/Renderer/BufferRenderer.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../Helper/Progress.h"
#include "../Dialog/SampleScaleDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include <math.h>

#include "../../Data/Song.h"
#include "../../lib/math/math.h"
#include "SampleManagerConsole.h"


void render_bufbox(Image &im, BufferBox &b, AudioView *view)
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

void render_midi(Image &im, MidiData &m)
{
	int w = im.width;
	int h = im.height;
	Range r = Range(0, m.samples);
	MidiDataRef notes = m.getNotes(r);
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
	if (s->type == Track::TYPE_AUDIO)
		render_bufbox(im, s->buf, view);
	else if (s->type == Track::TYPE_MIDI)
		render_midi(im, s->midi);
	return hui::SetImage(im);
}

class SampleManagerItem : public Observer
{
public:
	SampleManagerItem(SampleManagerConsole *_manager, Sample *_s, AudioView *view) :
		Observer("SampleManagerItem")
	{
		manager = _manager;
		s = _s;
		icon = render_sample(s, view);
		subscribe(s);
	}
	virtual ~SampleManagerItem()
	{
		zombify();
	}
	virtual void onUpdate(Observable *o, const string &message)
	{
		//msg_write("item:  " + message);
		if (message == s->MESSAGE_DELETE){
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
			unsubscribe(s);
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

SampleManagerConsole::SampleManagerConsole(Song *s, AudioView *_view) :
	SideBarConsole(_("Samples")),
	Observer("SampleManagerConsole")
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

	song = s;
	view = _view;
	updateList();

	subscribe(song, song->MESSAGE_ADD_SAMPLE);
	subscribe(song, song->MESSAGE_DELETE_SAMPLE);
	subscribe(song, song->MESSAGE_NEW);
}

SampleManagerConsole::~SampleManagerConsole()
{
	for (SampleManagerItem *si: items)
		delete(si);
	items.clear();

	unsubscribe(song);
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
	if (tsunami->storage->askOpenImport(win)){
		BufferBox buf;
		tsunami->storage->loadBufferBox(song, &buf, hui::Filename);
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

	if (tsunami->storage->askSaveExport(win)){
		if (sel[0]->type == Track::TYPE_AUDIO){
			BufferRenderer rr(&sel[0]->buf);
			tsunami->storage->saveViaRenderer(&rr, hui::Filename);
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
	song->createSamplesFromSelection(view->sel, view->cur_layer);
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
		if (s->type != Track::TYPE_AUDIO)
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

void SampleManagerConsole::onUpdate(Observable *o, const string &message)
{
	if (progress and (o == progress)){
		if (message == progress->MESSAGE_CANCEL)
			endPreview();
	}else if (o == preview_stream){
		int pos = preview_stream->getPos();
		Range r = preview_sample->range();
		progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
		if (!preview_stream->isPlaying())
			endPreview();
	}else if (o == song){
		//msg_write(o->getName() + " / " + message);
		updateList();
	}
}

void SampleManagerConsole::onPreview()
{
	if (progress)
		endPreview();
	int sel = getInt("sample_list");
	preview_sample = items[sel]->s;
	preview_renderer = new BufferRenderer(&preview_sample->buf);
	preview_stream = new OutputStream(preview_renderer);

	progress = new ProgressCancelable(_("Preview"), win);
	subscribe(progress);
	subscribe(preview_stream);
	preview_stream->play();
}

void SampleManagerConsole::endPreview()
{
	if (!progress)
		return;
	unsubscribe(preview_stream);
	unsubscribe(progress);
	preview_stream->stop();
	delete(progress);
	progress = NULL;
	delete(preview_stream);
	delete(preview_renderer);
	preview_sample = NULL;
}


class SampleSelector : public hui::Dialog
{
public:
	SampleSelector(hui::Panel *root, Song *a, Sample *old, AudioView *view) :
		hui::Dialog("", 300, 400, root->win, false)
	{
		song = a;
		ret = NULL;;
		_old = old;

		fromResource("sample_selection_dialog");

		list_id = "sample_selection_list";

		setString(list_id, _("\\- none -\\"));
		setInt(list_id, 0);
		foreachi(Sample *s, song->samples, i){
			icon_names.add(render_sample(s, view));
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

Sample *SampleManagerConsole::select(hui::Panel *root, Song *a, Sample *old)
{
	SampleSelector *s = new SampleSelector(root, a, old, tsunami->_view);
	s->run();
	Sample *r = s->ret;
	delete(s);
	return r;
}
