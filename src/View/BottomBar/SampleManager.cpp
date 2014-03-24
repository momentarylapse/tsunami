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
#include "../../Tsunami.h"
#include <math.h>

SampleManager::SampleManager(AudioFile *a)
{
	title = _("Sample Manager");
	FromResource("sample_manager_dialog");
	SetTooltip("insert_sample", _("f&ugt am Cursor der aktuellen Spur ein"));

	EventM("import_from_file", this, &SampleManager::OnImportFromFile);
	EventM("insert_sample", this, &SampleManager::OnInsert);
	EventM("create_from_selection", this, &SampleManager::OnCreateFromSelection);
	EventM("delete_sample", this, &SampleManager::OnDelete);
	EventMX("sample_list", "hui:select", this, &SampleManager::OnListSelect);

	audio = a;
	selected = 0;
	FillList();

	Subscribe(audio);
}

SampleManager::~SampleManager()
{
	Unsubscribe(audio);
}

string render_bufbox(BufferBox &b, int w, int h)
{
	Image im;
	im.create(w, h, color(0, 0, 0, 0));
	for (int x=0; x<w; x++){
		float m = 0;
		int i0 = (b.num * x) / w;
		int i1 = (b.num * (x + 1)) / w;
		for (int i=i0; i<i1; i++)
			m = max(m, fabs(b.r[i]));
		for (int y=h*(1-m)/2; y<h*(1+m)/2; y++)
			im.setPixel(x, y, Black);
	}
	return HuiSetImage(im);
}

void SampleManager::FillList()
{
	Reset("sample_list");
	foreach(string &name, icon_names)
		HuiDeleteImage(name);
	icon_names.clear();
	foreachi(Sample *s, audio->sample, i){
		icon_names.add(render_bufbox(s->buf, 80, 32));
		SetString("sample_list", icon_names[i] + "\\" + s->name + "\\" + audio->get_time_str(s->buf.num) + "\\" + format(_("%d mal"), s->ref_count) + "\\" + b2s(s->auto_delete));
	}
	if (audio->sample.num > 0){
		selected = min(selected, audio->sample.num - 1);
		SetInt("sample_list", selected);
	}
	Enable("delete_sample", audio->sample.num > 0);
	Enable("insert_sample", audio->sample.num > 0);
}

void SampleManager::OnListSelect()
{
	selected = GetInt("");
}

void SampleManager::OnImportFromFile()
{
	if (tsunami->storage->AskOpenImport(win)){
		BufferBox buf;
		tsunami->storage->LoadBufferBox(audio, &buf, HuiFilename);
		selected = audio->sample.num;
		audio->AddSample(HuiFilename.basename(), buf);
	}
}

void SampleManager::OnInsert()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		tsunami->view->cur_track->AddSample(audio->selection.start(), n);
}

void SampleManager::OnCreateFromSelection()
{
	selected = audio->sample.num;
	audio->CreateSubsFromSelection(tsunami->view->cur_level);
}

void SampleManager::OnDelete()
{
	int n = GetInt("sample_list");
	if (n >= 0)
		audio->DeleteSample(n);
}

void SampleManager::OnUpdate(Observable *o, const string &message)
{
	FillList();
}
