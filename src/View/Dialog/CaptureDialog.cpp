/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "CaptureDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/AudioInput.h"
#include "../../Audio/AudioOutput.h"
#include "../AudioView.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

CaptureDialog::CaptureDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	audio = a;
	capturing = false;
	type = Track::TYPE_AUDIO;


	int sample_rate = (a->used) ? a->sample_rate : DEFAULT_SAMPLE_RATE;
	//CapturingByDialog = true;

	if (!tsunami->input->Start(type, sample_rate)){
		/*HuiErrorBox(MainWin, _("Fehler"), _("Konnte Aufnahmeger&at nicht &offnen"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}

	// dialog
	FromResource("record_dialog");
	Check("capture_type:audio", true);
	peak_meter = new PeakMeter(this, "capture_level", tsunami->input);
	SetString("capture_time", a->get_time_str(0));
	Enable("capture_delete", false);
	Enable("capture_pause", false);
	Enable("ok", false);


	Enable("capture_playback", a->used);
	Check("capture_playback", a->used);

	foreach(Track *t, a->track)
		AddString("capture_target", t->GetNiceName());
	AddString("capture_target", _("neue Spur anlegen"));
	if (tsunami->view->cur_track)
		SetInt("capture_target", get_track_index(tsunami->view->cur_track));
	else
		SetInt("capture_target", a->track.num);

	EventM("cancel", this, &CaptureDialog::OnClose);
	EventM("hui:close", this, &CaptureDialog::OnClose);
	EventM("ok", this, &CaptureDialog::OnOk);
	EventM("capture_type:audio", this, &CaptureDialog::OnTypeAudio);
	EventM("capture_type:midi", this, &CaptureDialog::OnTypeMidi);
	EventM("capture_start", this, &CaptureDialog::OnStart);
	EventM("capture_delete", this, &CaptureDialog::OnDelete);
	EventM("capture_pause", this, &CaptureDialog::OnPause);
	Subscribe(tsunami->input);
}

CaptureDialog::~CaptureDialog()
{
	Unsubscribe(tsunami->input);
	delete(peak_meter);
}


void CaptureDialog::OnTypeAudio()
{
	if (type == Track::TYPE_AUDIO)
		return;
	type = Track::TYPE_AUDIO;
	tsunami->input->Start(type, tsunami->input->GetSampleRate());
}

void CaptureDialog::OnTypeMidi()
{
	if (type == Track::TYPE_MIDI)
		return;
	type = Track::TYPE_MIDI;
	tsunami->input->Start(type, tsunami->input->GetSampleRate());
}

void CaptureDialog::OnStart()
{
	if (IsChecked("capture_playback"))
		tsunami->output->Play(audio, false);
	tsunami->input->ResetSync();
	capturing = true;
	Enable("capture_start", false);
	Enable("capture_pause", true);
	Enable("capture_delete", true);
	Enable("ok", true);
	Enable("capture_playback", false);
	Enable("capture_type:audio", false);
	Enable("capture_type:midi", false);
}

void CaptureDialog::OnDelete()
{
	if (tsunami->output->IsPlaying())
		tsunami->output->Stop();
	buf.clear();
	capturing = false;
	Enable("capture_start", true);
	Enable("capture_pause", false);
	Enable("capture_delete", false);
	Enable("ok", false);
	SetString("capture_time", audio->get_time_str(buf.num));
}

void CaptureDialog::OnPause()
{
	// TODO...
	if (tsunami->output->IsPlaying())
		tsunami->output->Stop();
	capturing = false;
	Enable("capture_start", true);
	Enable("capture_pause", false);
}


void CaptureDialog::OnOk()
{
	tsunami->input->Stop();
	tsunami->output->Stop();
	Insert();
	delete(this);
}

void CaptureDialog::OnClose()
{
	tsunami->input->Stop();
	tsunami->output->Stop();
	delete(this);
}

void CaptureDialog::OnUpdate(Observable *o)
{
	//if (tsunami->input->CapturePlayback)
	//msg_write(tsunami->output->GetPos() - buf.num);
	if (capturing){
		buf.append(tsunami->input->current_buffer);
		SetString("capture_time", audio->get_time_str(buf.num));
	}
}

void CaptureDialog::Insert()
{
	msg_db_r("CaptureInsert", 1);
	Track *t;
	int target = GetInt("capture_target");
	int dpos = 0;
	int i0;
	if (audio->used){
		int s_start = audio->selection.start();

		// insert recorded data with some delay
		dpos = - tsunami->input->GetDelay();

		if (target >= audio->track.num){
			// new track
			t = audio->AddEmptyTrack(audio->track.num);
			i0 = s_start - dpos;
		}else{
			// sub track
			t = audio->track[target];
			//dpos = s_start + dpos;
			i0 = s_start - dpos;
		}
	}else{
		// new file
		audio->NewWithOneTrack(DEFAULT_SAMPLE_RATE);
		t = audio->track[0];
		i0 = 0;
	}

	// insert data
	Range r = Range(i0, buf.num);
	audio->action_manager->BeginActionGroup();
	BufferBox tbuf = t->GetBuffers(tsunami->view->cur_level, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, tsunami->view->cur_level, r);
	tbuf.set(buf, 0, 1.0f);
	audio->Execute(a);
	audio->action_manager->EndActionGroup();
	buf.clear();
	msg_db_l(1);
}

