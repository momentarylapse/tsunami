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
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include <math.h>
#include "../../lib/math/math.h"

SampleManager::SampleManager(AudioFile *a) :
	BottomBarConsole(_("Samples")),
	Observer("SampleManager")
{
	FromResource("sample_manager_dialog");
	SetTooltip("import_from_file", _("aus Datei importieren"));
	SetTooltip("delete_sample", _("l&oschen"));
	SetTooltip("paste_sample", _("f&ugt am Cursor der aktuellen Spur ein"));
	SetTooltip("create_from_selection", _("aus Auswahl erzeugen"));

	EventM("import_from_file", this, &SampleManager::onImportFromFile);
	EventM("paste_sample", this, &SampleManager::onInsert);
	EventM("create_from_selection", this, &SampleManager::onCreateFromSelection);
	EventM("delete_sample", this, &SampleManager::onDelete);
	EventMX("sample_list", "hui:change", this, &SampleManager::onListEdit);
	EventMX("sample_list", "hui:select", this, &SampleManager::onListSelect);

	audio = a;
	selected_uid = -1;
	fillList();

	subscribe(audio, audio->MESSAGE_CHANGE);
}

SampleManager::~SampleManager()
{
	unsubscribe(audio);
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
	Range r = m.GetRange();
	foreach(MidiNote &n, m){
		float y = h * clampf((80 - n.pitch) / 50.0f, 0, 1);
		float x0 = w * clampf((float)n.range.offset / (float)r.num, 0, 1);
		float x1 = w * clampf((float)n.range.end() / (float)r.num, 0, 1);
		color c = AudioViewTrack::GetPitchColor(n.pitch);
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
	Reset("sample_list");
	foreach(string &name, icon_names)
		HuiDeleteImage(name);
	icon_names.clear();
	foreachi(Sample *s, audio->sample, i){
		icon_names.add(render_sample(s));
		SetString("sample_list", icon_names[i] + "\\" + track_type(s->type) + "\\" + s->name + "\\" + audio->get_time_str_long(s->GetRange().num) + "\\" + format(_("%d mal"), s->ref_count) + "\\" + b2s(s->auto_delete));
	}
	int sel = audio->get_sample_by_uid(selected_uid);
	SetInt("sample_list", sel);
	Enable("delete_sample", sel >= 0);
	Enable("paste_sample", sel >= 0);
}

void SampleManager::onListSelect()
{
	int sel = GetInt("");
	selected_uid = -1;
	if (sel >= 0)
		selected_uid = audio->sample[sel]->uid;
	Enable("delete_sample", sel >= 0);
	Enable("paste_sample", sel >= 0);
}

void SampleManager::onListEdit()
{
	int sel = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 2)
		audio->EditSampleName(sel, GetCell("sample_list", sel, 2));
	else if (col == 5)
		audio->sample[sel]->auto_delete = GetCell("sample_list", sel, 5)._bool();
}

void SampleManager::onImportFromFile()
{
	if (tsunami->storage->askOpenImport(win)){
		BufferBox buf;
		tsunami->storage->loadBufferBox(audio, &buf, HuiFilename);
		Sample *s = audio->AddSample(HuiFilename.basename(), buf);
		selected_uid = s->uid;
		SetInt("sample_list", audio->sample.num - 1);
		Enable("delete_sample", true);
		Enable("paste_sample", true);
	}
}

void SampleManager::onInsert()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		tsunami->win->view->cur_track->AddSample(tsunami->win->view->sel_range.start(), n);
}

void SampleManager::onCreateFromSelection()
{
	audio->CreateSamplesFromSelection(tsunami->win->view->cur_level, tsunami->win->view->sel_range);
	if (audio->sample.num > 0){
		selected_uid = audio->sample.back()->uid;
		Enable("delete_sample", true);
		Enable("paste_sample", true);
	}
}

void SampleManager::onDelete()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		audio->DeleteSample(n);
}

void SampleManager::onUpdate(Observable *o, const string &message)
{
	fillList();
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

		FromSource("Grid ? '' 1 2\n"\
					"	ListView list 'Vorschau\\Name\\Dauer' format=itt\n"\
					"	Grid ? '' 2 1 buttonbar\n"\
					"		Button cancel 'Abbrechen'\n"\
					"		Button ok 'Ok'");

		SetString("list", _("\\- keines -\\"));
		SetInt("list", 0);
		foreachi(Sample *s, audio->sample, i){
			icon_names.add(render_sample(s));
			SetString("list", icon_names[i] + "\\" + s->name + "\\" + audio->get_time_str_long(s->buf.num));
			if (s == old)
				SetInt("list", i + 1);
		}

		EventM("ok", this, &SampleSelector::onOk);
		EventM("cancel", this, &SampleSelector::onCancel);
		EventM("hui:close", this, &SampleSelector::onCancel);
		EventMX("list", "hui:select", this, &SampleSelector::onSelect);
		EventM("list", this, &SampleSelector::onList);
	}
	virtual ~SampleSelector()
	{
		foreach(string &name, icon_names)
			HuiDeleteImage(name);
	}

	void onSelect()
	{
		int n = GetInt("");
		ret = NULL;
		if (n >= 1)
			ret = audio->sample[n - 1];
		Enable("ok", n >= 0);
	}

	void onList()
	{
		int n = GetInt("");
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
	s->Run();
	return SampleSelector::ret;
}
