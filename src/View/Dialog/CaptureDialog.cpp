/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "CaptureDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Audio/AudioInput.h"
#include "../../Audio/AudioInputMidi.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioRenderer.h"
#include "../AudioView.h"
#include "../../Stuff/Log.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

string track_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return "audio";
	if (type == Track::TYPE_MIDI)
		return "midi";
	if (type == Track::TYPE_TIME)
		return "time";
	return "???";
}

CaptureDialog::CaptureDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a):
	HuiWindow("record_dialog", _parent, _allow_parent),
	Observer("CaptureDialog")
{
	audio = a;
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
	Check("capture_type:audio", true);
	peak_meter = new PeakMeter(this, "capture_level", tsunami->input);
	SetString("capture_time", a->get_time_str_long(0));
	Enable("capture_delete", false);
	Enable("capture_pause", false);
	Enable("ok", false);

	foreach(Track *t, a->track)
		AddString("capture_target", t->GetNiceName() + "     (" + track_type(t->type) + ")");
	AddString("capture_target", _("neue Spur anlegen"));
	if (tsunami->win->view->cur_track)
		SetInt("capture_target", get_track_index(tsunami->win->view->cur_track));
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
	tsunami->input->in_midi->Unconnect();
	tsunami->input->Stop();
	tsunami->output->Stop();
	tsunami->input->buffer.clear();
	delete(peak_meter);
}


void CaptureDialog::OnTypeAudio()
{
	if (type == Track::TYPE_AUDIO)
		return;
	type = Track::TYPE_AUDIO;
	tsunami->input->Start(type, tsunami->input->GetSampleRate());
}

static Array<AudioInputMidi::MidiPort> ports;

void OnSelectPort()
{
	int n = HuiCurWindow->GetInt("");
	if (n < ports.num)
		tsunami->input->in_midi->ConnectTo(ports[n]);
	delete HuiCurWindow;
}

void SelectMidiPort(HuiWindow *parent)
{
	ports = tsunami->input->in_midi->FindPorts();
	HuiDialog *dlg = new HuiDialog(_("MIDI Quelle ausw&ahlen"), 400, 300, parent, false);
	dlg->AddListView(_("MIDI Quellen"), 0, 0, 0, 0, "port_list");
	foreach(AudioInputMidi::MidiPort &p, ports)
		dlg->SetString("port_list", p.client_name + " : " + p.port_name);
	dlg->SetString("port_list", _("        - nicht verbinden -"));
	dlg->SetTooltip("port_list", _("* entweder eine Quelle w&ahlen (empfohlen) oder\n* \"nicht verbinden\" um es anderen Programmen &uberlassen, eine Verbindung herzustellen"));
	dlg->SetInt("port_list", 0);
	dlg->Event("port_list", &OnSelectPort);
	dlg->Run();
}

void CaptureDialog::OnTypeMidi()
{
	if (type == Track::TYPE_MIDI)
		return;
	type = Track::TYPE_MIDI;
	SelectMidiPort(this);
	tsunami->input->Start(type, tsunami->input->GetSampleRate());
}

void CaptureDialog::OnStart()
{
	if (audio->used){
		tsunami->renderer->Prepare(audio, tsunami->win->view->GetPlaybackSelection(), false);
		tsunami->output->Play(tsunami->renderer);
	}

	tsunami->input->ResetSync();
	tsunami->input->Accumulate(true);
	Enable("capture_start", false);
	Enable("capture_pause", true);
	Enable("capture_delete", true);
	Enable("ok", true);
	Enable("capture_type:audio", false);
	Enable("capture_type:midi", false);
}

void CaptureDialog::OnDelete()
{
	if (tsunami->output->IsPlaying())
		tsunami->output->Stop();
	tsunami->input->ResetAccumulation();
	tsunami->input->Accumulate(false);
	Enable("capture_start", true);
	Enable("capture_pause", false);
	Enable("capture_delete", false);
	Enable("ok", false);
	OnUpdate(NULL, "");
}

void CaptureDialog::OnPause()
{
	// TODO...
	if (tsunami->output->IsPlaying())
		tsunami->output->Stop();
	tsunami->input->Accumulate(false);
	Enable("capture_start", true);
	Enable("capture_pause", false);
}


void CaptureDialog::OnOk()
{
	if (Insert())
		delete(this);
}

void CaptureDialog::OnClose()
{
	delete(this);
}

void CaptureDialog::OnUpdate(Observable *o, const string &message)
{
	SetString("capture_time", audio->get_time_str_long(tsunami->input->GetSampleCount()));
}

bool CaptureDialog::Insert()
{
	msg_db_f("CaptureInsert", 1);
	Track *t;
	int target = GetInt("capture_target");
	int i0;
	if (audio->used){
		int s_start = tsunami->win->view->sel_range.start();

		// insert recorded data with some delay
		int dpos = tsunami->input->GetDelay();

		if (target >= audio->track.num){
			// new track
			t = audio->AddTrack(type, audio->track.num);
		}else{
			// overwrite
			t = audio->track[target];
		}
		i0 = s_start + dpos;
	}else{
		// new file
		audio->NewWithOneTrack(type, DEFAULT_SAMPLE_RATE);
		t = audio->track[0];
		i0 = 0;
	}
	if (t->type != type){
		tsunami->log->Error(format(_("Kann aufgenommene Daten (%s) nicht in Ziel (%s) einf&ugen."), track_type(type).c_str(), track_type(t->type).c_str()));
		return false;
	}

	// insert data
	if (type == t->TYPE_AUDIO){
		Range r = Range(i0, tsunami->input->GetSampleCount());
		audio->action_manager->BeginActionGroup();
		BufferBox tbuf = t->GetBuffers(tsunami->win->view->cur_level, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, tsunami->win->view->cur_level, r);
		tbuf.set(tsunami->input->buffer, 0, 1.0f);
		audio->Execute(a);
		audio->action_manager->EndActionGroup();
	}else if (type == t->TYPE_MIDI){
		t->InsertMidiData(i0, tsunami->input->midi);
	}
	tsunami->input->ResetAccumulation();
	return true;
}

