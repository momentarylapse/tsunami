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
#include "../Helper/Slider.h"
#include "../Helper/BarList.h"

AudioFileDialog::AudioFileDialog(AudioFile *a) :
	SideBarConsole(_("Datei-Eigenschaften")),
	Observer("AudioFileDialog")
{
	audio = a;

	// dialog
//	SetTarget("audio_dialog_table", 0);
	SetBorderWidth(5);
	EmbedDialog("audio_file_dialog", 0, 0);
	SetDecimals(1);
	volume_slider = new Slider(this, "audio_volume_slider", "audio_volume", 0, 1, 100, (void(HuiEventHandler::*)())&AudioFileDialog::OnVolume, audio->volume, this);
	bar_list = new BarList(this, "audio_bar_list", "audio_add_bar", "audio_add_bar_pause", "audio_delete_bar");
	HideControl("ad_t_bars", true);

	Expand("ad_t_tags", 0, true);

	LoadData();

	EventMX("tags", "hui:select", this, &AudioFileDialog::OnTagsSelect);
	EventMX("tags", "hui:change", this, &AudioFileDialog::OnTagsEdit);
	EventM("add_tag", this, &AudioFileDialog::OnAddTag);
	EventM("delete_tag", this, &AudioFileDialog::OnDeleteTag);

	Subscribe(audio);
}

AudioFileDialog::~AudioFileDialog()
{
	Unsubscribe(audio);
	delete(bar_list);
	delete(volume_slider);
}

void AudioFileDialog::LoadData()
{
	volume_slider->Set(audio->volume);
	Reset("tags");
	foreach(Tag &t, audio->tag)
		AddString("tags", t.key + "\\" + t.value);
	Reset("data_list");
	int samples = audio->GetRange().length();
	AddString("data_list", _("Anfang\\") + audio->get_time_str_long(audio->GetRange().start()));
	AddString("data_list", _("Ende\\") + audio->get_time_str_long(audio->GetRange().end()));
	AddString("data_list", _("Dauer\\") + audio->get_time_str_long(samples));
	AddString("data_list", _("Samples\\") + i2s(samples));
	AddString("data_list", _("Abtastrate\\") + i2s(audio->sample_rate) + " Hz");
	AddString("data_list", _("Format\\16 bit stereo (nami)"));
	Enable("delete_tag", false);
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

void AudioFileDialog::OnVolume()
{
	audio->SetVolume(volume_slider->Get());
}

void AudioFileDialog::OnUpdate(Observable *o, const string &message)
{
	LoadData();
}
