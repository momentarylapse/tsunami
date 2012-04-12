/*
 * AudioFileDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioFileDialog.h"
#include "../../Tsunami.h"

AudioFileDialog::AudioFileDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	audio = a;

	// dialog
	FromResource("wave_dialog");

	foreach(a->tag, t)
		AddString("tags", t.key + "\\" + t.value);
	int samples = a->GetRange().length();
	SetString("time", a->get_time_str(samples));
	SetInt("samples", samples);
	SetInt("frequency", a->sample_rate);
	SetString("format", "16 bit stereo (nami)");
	RefillAudioList();
	fx_list = new FxList(this, "fx_list", a->fx);

	EventM("wave_list", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTrackList);
	EventM("close", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnClose);
}

AudioFileDialog::~AudioFileDialog()
{
	//WaveDialog = NULL;
	delete(fx_list);
}


string get_vol(float volume, bool muted)
{
	return muted ? _("(stumm)") : format("%.1f%%", volume * 100.0f);
}

void AudioFileDialog::RefillAudioList()
{
	msg_db_r("RefillAudioList", 1);
	Reset("wave_list");
	foreachi(audio->track, t, i)
		AddString("wave_list", format("%d\\%s\\%s", i + 1, t.name.c_str(), get_vol(t.volume, t.muted).c_str()));
	audio_list.clear();
	foreachi(audio->track, t, i)
		foreachi(t.sub, s, j){
			AddChildString("wave_list", i, format("%d\\%s\\%s", j + 1, s.name.c_str(), get_vol(t.volume * s.volume, t.muted || s.muted).c_str()));
			audio_list.add(&s);
		}
	msg_db_l(1);
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
			dlg_audio->history->Change();
		case HUI_WIN_CLOSE:
		case HMM_CANCEL:
			delete(dlg);
			break;
	}
	msg_db_l(1);
}
#endif

void AudioFileDialog::OnTrackList()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel < audio->track.num)){
		audio->cur_track = sel;
		tsunami->view->ExecuteTrackDialog(this, &audio->track[sel]);
		RefillAudioList();
	}else if (sel >= audio->track.num){
		//ExecuteLevelDialog(this, audio_list[sel - dlg_audio->Track.num]);
		RefillAudioList();
	}
}

void AudioFileDialog::OnClose()
{
	delete(this);
}
