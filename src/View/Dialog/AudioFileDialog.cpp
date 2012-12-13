/*
 * AudioFileDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioFileDialog.h"
#include "../../Tsunami.h"

AudioFileDialog::AudioFileDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a):
	CHuiWindow("dummy", -1, -1, 200, 200, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	audio = a;

	// dialog
	FromResource("wave_dialog");

	fx_list = new FxList(this, "fx_list", "add_effect", "configure_effect", "delete_effect", &a->fx);

	LoadData();

	EventM("wave_list", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTrackList);
	EventMX("tags", "hui:select", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTagsSelect);
	EventMX("tags", "hui:change", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnTagsEdit);
	EventM("add_tag", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnAddTag);
	EventM("delete_tag", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnDeleteTag);
	EventM("close", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&AudioFileDialog::OnClose);

	Subscribe(audio);
}

AudioFileDialog::~AudioFileDialog()
{
	Unsubscribe(audio);
	//WaveDialog = NULL;
	delete(fx_list);
}

void AudioFileDialog::LoadData()
{
	Reset("tags");
	foreach(Tag &t, audio->tag)
		AddString("tags", t.key + "\\" + t.value);
	int samples = audio->GetRange().length();
	SetString("time", audio->get_time_str(samples));
	SetInt("samples", samples);
	SetInt("frequency", audio->sample_rate);
	SetString("format", "16 bit stereo (nami)");
	RefillAudioList();
	Enable("delete_tag", false);
	fx_list->FillList();
}


string get_vol(float volume, bool muted)
{
	return muted ? _("(stumm)") : format("%.1f%%", volume * 100.0f);
}

void AudioFileDialog::RefillAudioList()
{
	msg_db_r("RefillAudioList", 1);
	Reset("wave_list");
	foreachi(Track *t, audio->track, i)
		AddString("wave_list", format("%d\\%s\\%s", i + 1, t->name.c_str(), get_vol(t->volume, t->muted).c_str()));
	audio_list.clear();
	foreachi(Track *t, audio->track, i)
		foreachi(Track *s, t->sub, j){
			AddChildString("wave_list", i, format("%d\\%s\\%s", j + 1, s->name.c_str(), get_vol(t->volume * s->volume, t->muted || s->muted).c_str()));
			audio_list.add(s);
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
		tsunami->view->ExecuteTrackDialog(this, audio->track[sel]);
		RefillAudioList();
	}else if (sel >= audio->track.num){
		//ExecuteLevelDialog(this, audio_list[sel - dlg_audio->Track.num]);
		RefillAudioList();
	}
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

void AudioFileDialog::OnUpdate(Observable *o)
{
	LoadData();
}

void AudioFileDialog::OnClose()
{
	delete(this);
}
