/*
 * SampleManager.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleManager.h"
#include "../../Storage/Storage.h"
#include "../../View/AudioView.h"
#include "../../View/AudioViewTrack.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/Renderer/BufferRenderer.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../Helper/Progress.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include <math.h>

#include "../../Data/Song.h"
#include "../../lib/math/math.h"


void render_bufbox(Image &im, BufferBox &b)
{
	int w = im.width;
	int h = im.height;
	for (int x=0; x<w; x++){
		float m = 0;
		int i0 = (b.num * x) / w;
		int i1 = (b.num * (x + 1)) / w;
		for (int i=i0; i<i1; i++)
			m = max(m, fabs(b.r[i]));
		for (int y=h*(1-m)/2; y<h*(1+m)/2; y++)
			im.setPixel(x, y, tsunami->_view->colors.text);
	}
}

void render_midi(Image &im, MidiNoteData &m)
{
	int w = im.width;
	int h = im.height;
	Range r = Range(0, m.samples);
	Array<MidiNote> notes = m.getNotes(r);
	foreach(MidiNote &n, notes){
		float y = h * clampf((80 - n.pitch) / 50.0f, 0, 1);
		float x0 = w * clampf((float)n.range.offset / (float)r.num, 0, 1);
		float x1 = w * clampf((float)n.range.end() / (float)r.num, 0, 1);
		color c = AudioViewTrack::getPitchColor(n.pitch);
		for (int x=x0; x<=x1; x++)
			im.setPixel(x, y, c);
	}
}

string render_sample(Sample *s)
{
	Image im;
	im.create(150, 32, color(0, 0, 0, 0));
	if (s->type == Track::TYPE_AUDIO)
		render_bufbox(im, s->buf);
	else if (s->type == Track::TYPE_MIDI)
		render_midi(im, s->midi);
	return HuiSetImage(im);
}

class SampleManagerItem : public Observer
{
public:
	SampleManagerItem(SampleManager *_manager, Sample *_s) : Observer("SampleManagerItem")
	{
		manager = _manager;
		s = _s;
		icon = render_sample(s);
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
		return icon + "\\" + /*track_type(s->type) + "\\" +*/ s->name + "\\" + s->owner->get_time_str_long(s->getRange().num) + "\\" + format(_("%d mal"), s->ref_count) + "\\" + b2s(s->auto_delete);
	}
	string icon;
	Sample *s;
	SampleManager *manager;
};

SampleManager::SampleManager(Song *s) :
	SideBarConsole(_("Samples")),
	Observer("SampleManager")
{
	fromResource("sample_manager_dialog");
	setTooltip("import_from_file", _("aus Datei importieren"));
	setTooltip("export", _("in Datei exportieren"));
	setTooltip("preview_sample", _("Vorschau"));
	setTooltip("delete_sample", _("l&oschen"));
	setTooltip("paste_sample", _("f&ugt am Cursor der aktuellen Spur ein"));
	setTooltip("create_from_selection", _("aus Auswahl erzeugen"));

	event("import_from_file", this, &SampleManager::onImport);
	event("export_sample", this, &SampleManager::onExport);
	event("preview_sample", this, &SampleManager::onPreview);
	event("paste_sample", this, &SampleManager::onInsert);
	event("create_from_selection", this, &SampleManager::onCreateFromSelection);
	event("delete_sample", this, &SampleManager::onDelete);
	eventX("sample_list", "hui:change", this, &SampleManager::onListEdit);
	eventX("sample_list", "hui:select", this, &SampleManager::onListSelect);
	event("sample_list", this, &SampleManager::onPreview);

	event("edit_song", this, &SampleManager::onEditSong);

	preview_audio = new Song;
	preview_renderer = new SongRenderer(preview_audio);
	preview_stream = new AudioStream(preview_renderer);
	preview_sample = NULL;

	progress = NULL;

	song = s;
	selected_uid = -1;
	updateList();

	subscribe(song, song->MESSAGE_ADD_SAMPLE);
	subscribe(song, song->MESSAGE_DELETE_SAMPLE);
	subscribe(song, song->MESSAGE_NEW);
}

SampleManager::~SampleManager()
{
	foreach(SampleManagerItem *si, items)
		delete(si);
	items.clear();

	unsubscribe(song);
	delete(preview_stream);
	delete(preview_renderer);
	delete(preview_audio);
}

int SampleManager::getIndex(Sample *s)
{
	foreachi(SampleManagerItem *si, items, i)
		if (si->s == s)
			return i;
	return -1;
}

void SampleManager::updateList()
{
	// new samples?
	foreach(Sample *s, song->samples)
		if (getIndex(s) < 0)
			add(new SampleManagerItem(this, s));

	if ((selected_uid < 0) and (song->samples.num > 0))
		selected_uid = song->samples.back()->uid;

	int sel = song->get_sample_by_uid(selected_uid);
	setInt("sample_list", sel);
	enable("export_sample", sel >= 0);
	enable("preview_sample", sel >= 0);
	enable("delete_sample", sel >= 0);
	enable("paste_sample", sel >= 0);
}

void SampleManager::onListSelect()
{
	int sel = getInt("");
	selected_uid = -1;
	if (sel >= 0)
		selected_uid = song->samples[sel]->uid;
	enable("export_sample", sel >= 0);
	enable("preview_sample", sel >= 0);
	enable("delete_sample", sel >= 0);
	enable("paste_sample", sel >= 0);
}

void SampleManager::onListEdit()
{
	int sel = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 1)
		song->editSampleName(sel, getCell("sample_list", sel, 1));
	else if (col == 4)
		song->samples[sel]->auto_delete = getCell("sample_list", sel, 4)._bool();
}

void SampleManager::onImport()
{
	if (tsunami->storage->askOpenImport(win)){
		BufferBox buf;
		tsunami->storage->loadBufferBox(song, &buf, HuiFilename);
		Sample *s = song->addSample(HuiFilename.basename(), buf);
		selected_uid = s->uid;
		setInt("sample_list", song->samples.num - 1);
		enable("delete_sample", true);
		enable("paste_sample", true);
	}
}

void SampleManager::onExport()
{
	if (tsunami->storage->askSaveExport(win)){
		int sel = getInt("sample_list");
		Sample *s = song->samples[sel];
		if (s->type == Track::TYPE_AUDIO){
			BufferRenderer rr(&s->buf);
			tsunami->storage->saveViaRenderer(&rr, HuiFilename);
		}
	}
}

void SampleManager::onInsert()
{
	int n = getInt("sample_list");
	if (n >= 0)
		tsunami->win->view->cur_track->addSample(tsunami->win->view->sel_range.start(), n);
}

void SampleManager::onCreateFromSelection()
{
	song->createSamplesFromSelection(tsunami->win->view->cur_level, tsunami->win->view->sel_range);
	if (song->samples.num > 0){
		selected_uid = song->samples.back()->uid;
		enable("delete_sample", true);
		enable("paste_sample", true);
	}
}

void SampleManager::onDelete()
{
	int n = getInt("sample_list");
	if (n >= 0)
		song->deleteSample(n);
}

void SampleManager::add(SampleManagerItem *item)
{
	//msg_write("add");
	items.add(item);
	addString("sample_list", item->str());
}

void SampleManager::remove(SampleManagerItem *item)
{
	//msg_write("remove");
	foreachi(SampleManagerItem *si, items, i)
		if (si == item){
			//msg_write(i);
			items.erase(i);
			removeString("sample_list", i);

			// don't delete now... we're still in notify()?
			item->zombify();
			old_items.add(item);
		}
}

void SampleManager::onEditSong()
{
	tsunami->win->side_bar->open(SideBar::SONG_CONSOLE);
}

void SampleManager::onUpdate(Observable *o, const string &message)
{
	if ((progress) and (o == progress)){
		if (message == progress->MESSAGE_CANCEL)
			endPreview();
	}else if (o == preview_stream){
		int pos = preview_stream->getPos();
		Range r = preview_sample->getRange();
		progress->set(_("Vorschau"), (float)(pos - r.offset) / r.length());
		if (!preview_stream->isPlaying())
			endPreview();
	}else if (o == song){
		//msg_write(o->getName() + " / " + message);
		updateList();
	}
}

void SampleManager::onPreview()
{
	if (progress)
		endPreview();
	int sel = getInt("sample_list");
	preview_sample = song->samples[sel];
	preview_audio->reset();
	preview_audio->addTrack(preview_sample->type);
	preview_audio->tracks[0]->levels[0].buffers.add(preview_sample->buf);
	preview_audio->tracks[0]->midi = preview_sample->midi;
	preview_renderer->prepare(preview_audio->getRange(), false);

	progress = new ProgressCancelable(_("Vorschau"), win);
	subscribe(progress);
	subscribe(preview_stream);
	preview_stream->play();
}

void SampleManager::endPreview()
{
	if (!progress)
		return;
	unsubscribe(preview_stream);
	unsubscribe(progress);
	preview_stream->stop();
	delete(progress);
	progress = NULL;
	preview_sample = NULL;
}


class SampleSelector : public HuiDialog
{
public:
	SampleSelector(HuiPanel *root, Song *a, Sample *old) :
		HuiDialog("", 300, 400, root->win, false)
	{
		song = a;
		ret = NULL;;
		_old = old;

		fromResource("sample_selection_dialog");

		list_id = "sample_selection_list";

		setString(list_id, _("\\- keines -\\"));
		setInt(list_id, 0);
		foreachi(Sample *s, song->samples, i){
			icon_names.add(render_sample(s));
			setString(list_id, icon_names[i] + "\\" + s->name + "\\" + song->get_time_str_long(s->buf.num));
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
		foreach(string &name, icon_names)
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
			delete(this);
		}else if (n >= 1){
			ret = song->samples[n - 1];
			delete(this);
		}
	}

	void onOk()
	{
		delete(this);
	}

	void onCancel()
	{
		ret = _old;
		delete(this);
	}

	static Sample *ret;
	Sample *_old;
	Array<string> icon_names;
	Song *song;
	string list_id;
};

Sample *SampleSelector::ret;

Sample *SampleManager::select(HuiPanel *root, Song *a, Sample *old)
{
	SampleSelector *s = new SampleSelector(root, a, old);
	s->run();
	return SampleSelector::ret;
}
