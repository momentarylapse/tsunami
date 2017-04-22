/*
 * SampleManagerConsole.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Storage/Storage.h"
#include "../../View/AudioView.h"
#include "../../View/AudioViewTrack.h"
#include "../../Device/OutputStream.h"
#include "../../Audio/Renderer/BufferRenderer.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../Helper/Progress.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include <math.h>

#include "../../Data/Song.h"
#include "../../Data/BufferInterpolator.h"
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
	return HuiSetImage(im);
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
			HuiDeleteImage(icon);
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

	event("import_from_file", this, &SampleManagerConsole::onImport);
	event("export_sample", this, &SampleManagerConsole::onExport);
	event("preview_sample", this, &SampleManagerConsole::onPreview);
	event("paste_sample", this, &SampleManagerConsole::onInsert);
	event("create_from_selection", this, &SampleManagerConsole::onCreateFromSelection);
	event("delete_sample", this, &SampleManagerConsole::onDelete);
	event("scale_sample", this, &SampleManagerConsole::onScale);
	eventX("sample_list", "hui:change", this, &SampleManagerConsole::onListEdit);
	eventX("sample_list", "hui:select", this, &SampleManagerConsole::onListSelect);
	event("sample_list", this, &SampleManagerConsole::onPreview);

	event("edit_song", this, &SampleManagerConsole::onEditSong);

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
	int sel = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 1)
		song->editSampleName(items[sel]->s, getCell("sample_list", sel, 1));
	else if (col == 4)
		items[sel]->s->auto_delete = getCell("sample_list", sel, 4)._bool();
}

void SampleManagerConsole::onImport()
{
	if (tsunami->storage->askOpenImport(win)){
		BufferBox buf;
		tsunami->storage->loadBufferBox(song, &buf, HuiFilename);
		song->addSample(HuiFilename.basename(), buf);
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
			tsunami->storage->saveViaRenderer(&rr, HuiFilename);
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
	song->createSamplesFromSelection(view->sel, view->cur_level);
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
		BufferBox out;
		int new_size = (int)(((long long)s->buf.length * (long long)44100) / (long long)48000);
		msg_write(format("%d  ->  %d", s->buf.length, new_size));
		BufferInterpolator::interpolate(s->buf, out, new_size, BufferInterpolator::METHOD_LINEAR);
		msg_write("ok");
		s->buf = out;
		song->notify();
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
	HuiPanel::setSelection("sample_list", indices);
}

void SampleManagerConsole::onEditSong()
{
	tsunami->win->side_bar->open(SideBar::SONG_CONSOLE);
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


class SampleSelector : public HuiDialog
{
public:
	SampleSelector(HuiPanel *root, Song *a, Sample *old, AudioView *view) :
		HuiDialog("", 300, 400, root->win, false)
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

		event("ok", this, &SampleSelector::onOk);
		event("cancel", this, &SampleSelector::onCancel);
		event("hui:close", this, &SampleSelector::onCancel);
		eventX(list_id, "hui:select", this, &SampleSelector::onSelect);
		event(list_id, this, &SampleSelector::onList);
	}
	virtual ~SampleSelector()
	{
		for (string &name: icon_names)
			HuiDeleteImage(name);
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

Sample *SampleManagerConsole::select(HuiPanel *root, Song *a, Sample *old)
{
	SampleSelector *s = new SampleSelector(root, a, old, tsunami->_view);
	s->run();
	Sample *r = s->ret;
	delete(s);
	return r;
}
