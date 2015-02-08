/*
 * SampleManager.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleManager.h"
#include "../../Data/AudioFile.h"
#include "../../Storage/Storage.h"
#include "../../View/AudioView.h"
#include "../../View/AudioViewTrack.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/AudioRenderer.h"
#include "../Helper/Progress.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include <math.h>
#include "../../lib/math/math.h"

SampleManager::SampleManager(AudioFile *a) :
	BottomBarConsole(_("Samples")),
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

	preview_audio = new AudioFile;
	preview_renderer = new AudioRenderer;
	preview_stream = new AudioStream(preview_renderer);
	preview_sample = NULL;

	audio = a;
	selected_uid = -1;
	fillList();

	subscribe(audio, audio->MESSAGE_CHANGE);
}

SampleManager::~SampleManager()
{
	unsubscribe(audio);
	delete(preview_stream);
	delete(preview_renderer);
	delete(preview_audio);
}

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
			im.setPixel(x, y, Black);
	}
}

void render_midi(Image &im, MidiData &m)
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

void SampleManager::fillList()
{
	reset("sample_list");
	foreach(string &name, icon_names)
		HuiDeleteImage(name);
	icon_names.clear();
	foreachi(Sample *s, audio->sample, i){
		icon_names.add(render_sample(s));
		setString("sample_list", icon_names[i] + "\\" + track_type(s->type) + "\\" + s->name + "\\" + audio->get_time_str_long(s->getRange().num) + "\\" + format(_("%d mal"), s->ref_count) + "\\" + b2s(s->auto_delete));
	}
	int sel = audio->get_sample_by_uid(selected_uid);
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
		selected_uid = audio->sample[sel]->uid;
	enable("export_sample", sel >= 0);
	enable("preview_sample", sel >= 0);
	enable("delete_sample", sel >= 0);
	enable("paste_sample", sel >= 0);
}

void SampleManager::onListEdit()
{
	int sel = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 2)
		audio->editSampleName(sel, getCell("sample_list", sel, 2));
	else if (col == 5)
		audio->sample[sel]->auto_delete = getCell("sample_list", sel, 5)._bool();
}

void SampleManager::onImport()
{
	if (tsunami->storage->askOpenImport(win)){
		BufferBox buf;
		tsunami->storage->loadBufferBox(audio, &buf, HuiFilename);
		Sample *s = audio->addSample(HuiFilename.basename(), buf);
		selected_uid = s->uid;
		setInt("sample_list", audio->sample.num - 1);
		enable("delete_sample", true);
		enable("paste_sample", true);
	}
}

void SampleManager::onExport()
{
	if (tsunami->storage->askSaveExport(win)){
		int sel = getInt("sample_list");
		Sample *s = audio->sample[sel];
		if (s->type == Track::TYPE_AUDIO){
			tsunami->storage->saveBufferBox(audio, &s->buf, HuiFilename);
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
	audio->createSamplesFromSelection(tsunami->win->view->cur_level, tsunami->win->view->sel_range);
	if (audio->sample.num > 0){
		selected_uid = audio->sample.back()->uid;
		enable("delete_sample", true);
		enable("paste_sample", true);
	}
}

void SampleManager::onDelete()
{
	int n = getInt("sample_list");
	if (n >= 0)
		audio->deleteSample(n);
}

void SampleManager::onUpdate(Observable *o, const string &message)
{
	if (o == tsunami->progress){
		if (message == tsunami->progress->MESSAGE_CANCEL)
			endPreview();
	}else if (o == preview_stream){
		int pos = preview_stream->getPos();
		Range r = preview_sample->getRange();
		tsunami->progress->set(_("Vorschau"), (float)(pos - r.offset) / r.length());
		if (!preview_stream->isPlaying())
			endPreview();
	}else{
		fillList();
	}
}

void SampleManager::onPreview()
{
	int sel = getInt("sample_list");
	preview_sample = audio->sample[sel];
	preview_audio->reset();
	preview_audio->addTrack(preview_sample->type);
	preview_audio->track[0]->level[0].buffer.add(preview_sample->buf);
	preview_audio->track[0]->midi = preview_sample->midi;
	preview_renderer->prepare(preview_audio, preview_audio->getRange(), false);

	tsunami->progress->startCancelable(_("Vorschau"), 0);
	subscribe(tsunami->progress);
	subscribe(preview_stream);
	preview_stream->play();
}

void SampleManager::endPreview()
{
	unsubscribe(preview_stream);
	unsubscribe(tsunami->progress);
	preview_stream->stop();
	tsunami->progress->end();
	preview_sample = NULL;
}


class SampleSelector : public HuiDialog
{
public:
	SampleSelector(HuiPanel *root, AudioFile *a, Sample *old) :
		HuiDialog(_("Sample w&ahlen"), 300, 400, root->win, false)
	{
		audio = a;
		ret = NULL;;
		_old = old;

		fromSource("Grid ? '' 1 2\n"\
					"	ListView list 'Vorschau\\Name\\Dauer' format=itt\n"\
					"	Grid ? '' 2 1 buttonbar\n"\
					"		Button cancel 'Abbrechen'\n"\
					"		Button ok 'Ok'");

		setString("list", _("\\- keines -\\"));
		setInt("list", 0);
		foreachi(Sample *s, audio->sample, i){
			icon_names.add(render_sample(s));
			setString("list", icon_names[i] + "\\" + s->name + "\\" + audio->get_time_str_long(s->buf.num));
			if (s == old)
				setInt("list", i + 1);
		}

		event("ok", this, &SampleSelector::onOk);
		event("cancel", this, &SampleSelector::onCancel);
		event("hui:close", this, &SampleSelector::onCancel);
		eventX("list", "hui:select", this, &SampleSelector::onSelect);
		event("list", this, &SampleSelector::onList);
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
			ret = audio->sample[n - 1];
		enable("ok", n >= 0);
	}

	void onList()
	{
		int n = getInt("");
		if (n == 0){
			ret = NULL;
			delete(this);
		}else if (n >= 1){
			ret = audio->sample[n - 1];
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
	AudioFile *audio;
};

Sample *SampleSelector::ret;

Sample *SampleManager::select(HuiPanel *root, AudioFile *a, Sample *old)
{
	SampleSelector *s = new SampleSelector(root, a, old);
	s->run();
	return SampleSelector::ret;
}
