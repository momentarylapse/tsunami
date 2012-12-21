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
#include "../Helper/FxList.h"
#include "../Helper/BarList.h"

AudioFileDialog::AudioFileDialog(CHuiWindow *win, AudioFile *a):
	EmbeddedDialog(win)
{
	audio = a;

	// dialog
	win->SetTarget("audio_dialog_table", 0);
	win->SetBorderWidth(8);
	win->EmbedDialog("audio_file_dialog", 0, 0);
	win->SetDecimals(1);
	//volume_slider = new Slider(win, "audio_volume_slider", "audio_volume", 0, 2, 100, (void(HuiEventHandler::*)())&TrackDialog::OnVolume, 0, this);
	fx_list = new FxList(win, "audio_fx_list", "audio_add_effect", "audio_configure_effect", "audio_delete_effect");
	bar_list = new BarList(win, "audio_bar_list", "audio_add_bar", "audio_add_bar_pause", "audio_delete_bar");
	fx_list->SetAudio(audio);

	LoadData();

	win->EventMX("tags", "hui:select", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTagsSelect);
	win->EventMX("tags", "hui:change", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTagsEdit);
	win->EventM("add_tag", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnAddTag);
	win->EventM("delete_tag", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnDeleteTag);
	win->EventM("audio_close", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnClose);

	Subscribe(audio);
}

AudioFileDialog::~AudioFileDialog()
{
	Unsubscribe(audio);
	delete(fx_list);
	delete(bar_list);
}

void AudioFileDialog::LoadData()
{
	Reset("tags");
	foreach(Tag &t, audio->tag)
		AddString("tags", t.key + "\\" + t.value);
	Reset("data_list");
	int samples = audio->GetRange().length();
	AddString("data_list", _("Anfang\\") + audio->get_time_str(audio->GetRange().start()));
	AddString("data_list", _("Ende\\") + audio->get_time_str(audio->GetRange().end()));
	AddString("data_list", _("Dauer\\") + audio->get_time_str(samples));
	AddString("data_list", _("Samples\\") + i2s(samples));
	AddString("data_list", _("Abtastrate\\") + i2s(audio->sample_rate) + " Hz");
	AddString("data_list", _("Format\\16 bit stereo (nami)"));
	Enable("delete_tag", false);
	fx_list->FillList();
}


string get_vol(float volume, bool muted)
{
	return muted ? _("(stumm)") : format("%.1f%%", volume * 100.0f);
}

#if 0
void WaveDialogFunction(int message)
{
	msg_db_r("WaveDialogFunction", 1);
	CHuiWindow *dlg = WaveDialog;
	int sel;
	DoEffectList(dlg_audio->FX, dlg, HMM_FX_LIST, message);
	switch(message){
		case HMM_WAVE_LIST:
			sel = dlg->GetInt(HMM_WAVE_LIST);
			if ((sel >= 0) && (sel < dlg_audio->Track.num)){
				dlg_audio->CurTrack = sel;
				ExecuteTrackDialog(dlg, dlg_audio);
				RefillAudioList();
			}else if (sel >= dlg_audio->Track.num){
				//ExecuteLevelDialog(dlg, audio_list[sel - dlg_audio->Track.num]);
				RefillAudioList();
			}
			break;
		case HMM_OK:
			/*dlg_audio->Title = WaveDialog->GetCell(HMM_TAGS, 0, 1);
			dlg_audio->Album = WaveDialog->GetCell(HMM_TAGS, 1, 1);
			dlg_audio->Artist = WaveDialog->GetCell(HMM_TAGS, 2, 1);*/
		case HUI_WIN_CLOSE:
		case HMM_CANCEL:
			delete(dlg);
			break;
	}
	msg_db_l(1);
}
#endif

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

void AudioFileDialog::OnUpdate(Observable *o)
{
	LoadData();
}

void AudioFileDialog::OnClose()
{
	win->HideControl("audio_dialog_table", true);
}
