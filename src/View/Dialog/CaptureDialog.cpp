/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "CaptureDialog.h"
#include "../../Tsunami.h"

#include "../../Action/Track/ActionTrackEditBuffer.h"

CaptureDialog::CaptureDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	audio = a;


	int sample_rate = (a->used) ? a->sample_rate : DEFAULT_SAMPLE_RATE;
	//CapturingByDialog = true;

	if (!tsunami->input->Start(sample_rate, false)){
		/*HuiErrorBox(MainWin, _("Fehler"), _("Konnte Aufnahmeger&at nicht &offnen"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}

	// dialog
	FromResource("record_dialog");
	peak_meter = new PeakMeter(this, "capture_level", tsunami->input);
	SetString("capture_time", a->get_time_str(0));
	Enable("capture_delete", false);
	Enable("capture_pause", false);
	Enable("ok", false);

	SetString("capture_device", _("- Standard -"));
	SetInt("capture_device", 0);
	foreachi(tsunami->input->Device, d, i){
		AddString("capture_device", *d);
		if (*d == tsunami->input->ChosenDevice)
			SetInt("capture_device", i + 1);
	}

	Enable("capture_playback", a->used);

	foreach(a->track, t)
		AddString("capture_target", t->GetNiceName());
	AddString("capture_target", _("neue Spur anlegen"));
	SetInt("capture_target", (a->cur_track >= 0) ? a->cur_track : a->track.num);

	EventM("cancel", this, (void(HuiEventHandler::*)())&CaptureDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&CaptureDialog::OnClose);
	EventM("ok", this, (void(HuiEventHandler::*)())&CaptureDialog::OnOk);
	EventM("capture_device", this, (void(HuiEventHandler::*)())&CaptureDialog::OnDevice);
	EventM("capture_start", this, (void(HuiEventHandler::*)())&CaptureDialog::OnStart);
	EventM("capture_delete", this, (void(HuiEventHandler::*)())&CaptureDialog::OnDelete);
	EventM("capture_pause", this, (void(HuiEventHandler::*)())&CaptureDialog::OnPause);
	Subscribe(tsunami->input);
}

CaptureDialog::~CaptureDialog()
{
	Unsubscribe(tsunami->input);
	delete(peak_meter);
}


void CaptureDialog::OnDevice()
{
	if (GetInt("") > 0)
		tsunami->input->ChosenDevice = tsunami->input->Device[GetInt("") - 1];
	else
		tsunami->input->ChosenDevice = "";
	HuiConfigWriteStr("Input.ChosenDevice", tsunami->input->ChosenDevice);
	if (tsunami->input->Capturing)
		tsunami->input->Start(tsunami->input->CaptureSampleRate, tsunami->input->CaptureAddData);
}

void CaptureDialog::OnStart()
{
	tsunami->input->CapturePlayback = IsChecked("capture_playback");
	if (tsunami->input->CapturePlayback)
		tsunami->output->Play(audio, false);
	tsunami->input->CaptureAddData = true;
	Enable("capture_start", false);
	Enable("capture_pause", true);
	Enable("capture_delete", true);
	Enable("ok", true);
	Enable("capture_playback", false);
	Enable("capture_device", false);
}

void CaptureDialog::OnDelete()
{
	if (tsunami->input->CapturePlayback)
		tsunami->output->Stop();
	tsunami->input->CaptureAddData = false;
	tsunami->input->CaptureBuf.clear();
	Enable("capture_start", true);
	Enable("capture_pause", false);
	Enable("capture_delete", false);
	Enable("ok", false);
}

void CaptureDialog::OnPause()
{
	// TODO...
	if (tsunami->input->CapturePlayback)
		tsunami->output->Stop();
	tsunami->input->CaptureAddData = false;
	Enable("capture_start", true);
	Enable("capture_pause", false);
	Enable("capture_device", true);
}


void CaptureDialog::OnOk()
{
	tsunami->input->Stop();
	Insert();
	tsunami->input->Stop();
	delete(this);
}

void CaptureDialog::OnClose()
{
	tsunami->input->Stop();
	delete(this);
	tsunami->input->CaptureBuf.clear();
}

void CaptureDialog::OnUpdate(Observable *o)
{
	SetString("capture_time", audio->get_time_str(tsunami->input->CaptureBuf.num));
}

void CaptureDialog::Insert()
{
	msg_db_r("CaptureInsert", 1);
	Track *t;
	int target = GetInt("capture_target");
	int length = tsunami->input->CaptureBuf.num;
	int dpos = 0;
	int i0;
	if (audio->used){
		int s_start = audio->selection.start();

		// insert recorded data with some delay
		dpos = - tsunami->input->CaptureMaxDelay - (tsunami->input->CapturePlaybackDelay / 1000.0f) * (float)audio->sample_rate;
		//msg_write(f2s((float)CaptureMaxDelay / (float)audio->sample_rate * 1000,3));

		if (target >= audio->track.num){
			// new track
			t = audio->AddEmptyTrack();
			i0 = s_start - dpos;
		}else{
			// sub track
			t = &audio->track[target];
			//dpos = s_start + dpos;
			i0 = s_start - dpos;
		}
	}else{
		// new file
		audio->NewWithOneTrack(DEFAULT_SAMPLE_RATE);
		t = &audio->track[0];
		i0 = 0;
	}

	// insert data
	Range r = Range(i0, length);
	audio->action_manager->BeginActionGroup();
	BufferBox buf = t->GetBuffers(audio->cur_level, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, audio->cur_level, r);
	buf.set(tsunami->input->CaptureBuf, 0, 1.0f);
	audio->Execute(a);
	audio->action_manager->EndActionGroup();
	tsunami->input->CaptureBuf.clear();
	msg_db_l(1);
}

