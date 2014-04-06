/*
 * AudioFileDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioFileDialog.h"
#include "../../Tsunami.h"
#include "../../Stuff/Observer.h"
#include "../../Data/AudioFile.h"
#include "../Helper/BarList.h"
#include "../AudioView.h"
#include "../../Stuff/Log.h"

AudioFileDialog::AudioFileDialog(AudioView *v, AudioFile *a) :
	SideBarConsole(_("Datei-Eigenschaften")),
	Observer("AudioFileDialog")
{
	audio = a;
	view = v;

	// dialog
//	SetTarget("audio_dialog_table", 0);
	SetBorderWidth(5);
	EmbedDialog("audio_file_dialog", 0, 0);
	SetDecimals(1);
	bar_list = new BarList(this, "audio_bar_list", "audio_add_bar", "audio_add_bar_pause", "audio_delete_bar");
	HideControl("ad_t_bars", true);

	Expand("ad_t_tags", 0, true);

	LoadData();

	EventMX("tags", "hui:select", this, &AudioFileDialog::OnTagsSelect);
	EventMX("tags", "hui:change", this, &AudioFileDialog::OnTagsEdit);
	EventM("add_tag", this, &AudioFileDialog::OnAddTag);
	EventM("delete_tag", this, &AudioFileDialog::OnDeleteTag);
	EventMX("levels", "hui:select", this, &AudioFileDialog::OnLevelsSelect);
	EventMX("levels", "hui:change", this, &AudioFileDialog::OnLevelsEdit);
	EventM("add_level", this, &AudioFileDialog::OnAddLevel);
	EventM("delete_level", this, &AudioFileDialog::OnDeleteLevel);

	Subscribe(audio);
	Subscribe(view, view->MESSAGE_CUR_LEVEL_CHANGE);
}

AudioFileDialog::~AudioFileDialog()
{
	Unsubscribe(audio);
	Unsubscribe(view);
	delete(bar_list);
}

void AudioFileDialog::LoadData()
{
	// tags
	Reset("tags");
	foreach(Tag &t, audio->tag)
		AddString("tags", t.key + "\\" + t.value);
	Enable("delete_tag", false);

	// data
	Reset("data_list");
	int samples = audio->GetRange().length();
	AddString("data_list", _("Anfang\\") + audio->get_time_str_long(audio->GetRange().start()));
	AddString("data_list", _("Ende\\") + audio->get_time_str_long(audio->GetRange().end()));
	AddString("data_list", _("Dauer\\") + audio->get_time_str_long(samples));
	AddString("data_list", _("Samples\\") + i2s(samples));
	AddString("data_list", _("Abtastrate\\") + i2s(audio->sample_rate) + " Hz");
	AddString("data_list", _("Format\\16 bit stereo (nami)"));

	Reset("levels");
	foreachi(string &n, audio->level_name, i)
		AddString("levels", i2s(i + 1) + "\\" + n);
	if (audio->level_name.num > 0)
		SetInt("levels", view->cur_level);
}


string get_vol(float volume, bool muted)
{
	return muted ? _("(stumm)") : format("%.1f%%", volume * 100.0f);
}

void AudioFileDialog::OnTagsSelect()
{
	int s = GetInt("tags");
	Enable("delete_tag", s >= 0);
}

void AudioFileDialog::OnTagsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	Tag t = audio->tag[r];
	if (HuiGetEvent()->column == 0)
		t.key = GetCell("tags", r, 0);
	else
		t.value = GetCell("tags", r, 1);
	audio->EditTag(r, t.key, t.value);
}

void AudioFileDialog::OnAddTag()
{
	audio->AddTag("key", "value");
}

void AudioFileDialog::OnDeleteTag()
{
	int s = GetInt("tags");
	if (s >= 0)
		audio->DeleteTag(s);
}

void AudioFileDialog::OnLevelsSelect()
{
	int s = GetInt("levels");
	view->SetCurLevel(s);
}

void AudioFileDialog::OnLevelsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	audio->RenameLevel(r, GetCell("levels", r, 1));
}

void AudioFileDialog::OnAddLevel()
{
	audio->AddLevel();
}

void AudioFileDialog::OnDeleteLevel()
{
	int s = GetInt("levels");
	if (s >= 0)
		audio->DeleteLevel(s, false);
}

void AudioFileDialog::OnUpdate(Observable *o, const string &message)
{
	LoadData();
}
