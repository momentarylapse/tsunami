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
#include "../../Audio/AudioInputAudio.h"
#include "../../Audio/AudioInputMidi.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/AudioRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../../Stuff/Log.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

CaptureDialog::CaptureDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a):
	HuiWindow("record_dialog", _parent, _allow_parent),
	Observer("CaptureDialog")
{
	audio = a;
	view = tsunami->win->view;
	type = -1;

	temp_synth = CreateSynthesizer("");

	// dialog
	peak_meter = new PeakMeter(this, "capture_level", tsunami->input);
	setString("capture_time", a->get_time_str_long(0));
	enable("capture_delete", false);
	enable("capture_pause", false);
	enable("ok", false);

	// target list
	foreach(Track *t, a->tracks)
		addString("capture_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_target", _("  - neue Spur anlegen -"));



	if (view->cur_track){
		setTarget(view->cur_track->get_index());
	}else{
		setTarget(audio->tracks.num);
		setType(Track::TYPE_AUDIO);
	}

	event("cancel", this, &CaptureDialog::onClose);
	event("hui:close", this, &CaptureDialog::onClose);
	event("ok", this, &CaptureDialog::onOk);
	event("capture_type:audio", this, &CaptureDialog::onTypeAudio);
	event("capture_type:midi", this, &CaptureDialog::onTypeMidi);
	event("capture_source", this, &CaptureDialog::onSource);
	event("capture_target", this, &CaptureDialog::onTarget);
	event("capture_start", this, &CaptureDialog::onStart);
	event("capture_delete", this, &CaptureDialog::onDelete);
	event("capture_pause", this, &CaptureDialog::onPause);
	subscribe(tsunami->input);
}

CaptureDialog::~CaptureDialog()
{
	unsubscribe(tsunami->input);
	tsunami->input->in_midi->unconnect();
	tsunami->input->stop();
	view->stream->stop();
	tsunami->input->buffer.clear();
	delete(peak_meter);

	tsunami->input->in_midi->setPreviewSynthesizer(NULL);
	delete(temp_synth);
}

void CaptureDialog::onTarget()
{
	int target = getInt("capture_target");
	setTarget(target);
}


void CaptureDialog::onTypeAudio()
{
	setType(Track::TYPE_AUDIO);
}

/*void OnSelectPort()
{
	int n = HuiCurWindow->getInt("");
	if (n < ports.num)
		tsunami->input->in_midi->connectTo(ports[n]);
	delete HuiCurWindow;
}

void SelectMidiPort(HuiWindow *parent)
{
	ports = tsunami->input->in_midi->findPorts();
	HuiDialog *dlg = new HuiDialog(_("MIDI Quelle ausw&ahlen"), 400, 300, parent, false);
	dlg->addListView(_("MIDI Quellen"), 0, 0, 0, 0, "port_list");
	foreach(AudioInputMidi::MidiPort &p, ports)
		dlg->setString("port_list", p.client_name + " : " + p.port_name);
	dlg->setString("port_list", _("        - nicht verbinden -"));
	dlg->setTooltip("port_list", _("* entweder eine Quelle w&ahlen (empfohlen) oder\n* \"nicht verbinden\" um es anderen Programmen &uberlassen, eine Verbindung herzustellen"));
	dlg->setInt("port_list", 0);
	dlg->eventS("port_list", &OnSelectPort);
	dlg->run();
}*/

void CaptureDialog::onTypeMidi()
{
	setType(Track::TYPE_MIDI);
}

void CaptureDialog::setTarget(int index)
{
	if (index < audio->tracks.num){
		Track *t = audio->tracks[index];
		setType(t->type);
		tsunami->input->in_midi->setPreviewSynthesizer(t->synth);
	}else{
		tsunami->input->in_midi->setPreviewSynthesizer(temp_synth);
	}
	setInt("capture_target", index);
}

void CaptureDialog::onSource()
{
	int n = HuiCurWindow->getInt("");
	if (type == Track::TYPE_MIDI){
		if ((n >= 0) and (n < midi_ports.num))
			tsunami->input->in_midi->connectTo(midi_ports[n]);
		else
			tsunami->input->in_midi->unconnect();
	}else if (type == Track::TYPE_AUDIO){
		if ((n >= 0) and (n < audio_sources.num))
			tsunami->input->in_audio->setDevice(audio_sources[n]);
	}
}

void CaptureDialog::updateMidiPortList()
{
	midi_ports = tsunami->input->in_midi->findPorts();
	AudioInputMidi::MidiPort cur = tsunami->input->in_midi->getCurMidiPort();

	reset("capture_source");
	foreachi(AudioInputMidi::MidiPort &p, midi_ports, i){
		setString("capture_source", p.client_name + " : " + p.port_name);
		if ((p.client == cur.client) and (p.port == cur.port))
			setInt("capture_source", i);
	}
	setString("capture_source", _("        - nicht verbinden -"));
	if ((cur.client < 0) or (cur.port < 0))
		setInt("capture_source", midi_ports.num);
	enable("capture_source", true);

}

void CaptureDialog::updateAudioSourceList()
{
	audio_sources = tsunami->input->in_audio->getDevices();
	string cur = tsunami->input->in_audio->getChosenDevice();

	// add all
	reset("capture_source");
	foreach(string &d, audio_sources)
		setString("capture_source", d);

	// add "default"
	audio_sources.add("");
	setString("capture_source", _("        - Standard -"));

	// select current
	foreachi(string &d, audio_sources, i)
		if (cur == d)
			setInt("capture_source", i);

	enable("capture_source", true);

}

void CaptureDialog::setType(int _type)
{
	if (type == _type)
		return;
	type = _type;
	reset("capture_source");
	enable("capture_source", false);
	if (type == Track::TYPE_MIDI){
		updateMidiPortList();
		check("capture_type:midi", true);
	}else if (type == Track::TYPE_AUDIO){
		updateAudioSourceList();
		check("capture_type:audio", true);
	}else{
	}

	// consistency test: ...
	int target = getInt("capture_target");
	if ((target >= 0) and (target < audio->tracks.num))
		if (type != audio->tracks[target]->type)
			setInt("capture_target", audio->tracks.num);

	if (!tsunami->input->start(type, audio->sample_rate)){
		/*HuiErrorBox(MainWin, _("Fehler"), _("Konnte Aufnahmeger&at nicht &offnen"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureDialog::onStart()
{
	view->renderer->prepare(audio, view->getPlaybackSelection(), false);
	view->stream->play();

	tsunami->input->resetSync();
	tsunami->input->accumulate(true);
	enable("capture_start", false);
	enable("capture_pause", true);
	enable("capture_delete", true);
	enable("ok", true);
	enable("capture_type:audio", false);
	enable("capture_type:midi", false);
	enable("capture_source", false);
	enable("capture_target", false);
}

void CaptureDialog::onDelete()
{
	if (view->stream->isPlaying())
		view->stream->stop();
	tsunami->input->resetAccumulation();
	tsunami->input->accumulate(false);
	enable("capture_start", true);
	enable("capture_pause", false);
	enable("capture_delete", false);
	enable("capture_type:audio", true);
	enable("capture_type:midi", true);
	enable("capture_source", true);
	enable("capture_target", true);
	enable("ok", false);
	onUpdate(NULL, "");
}

void CaptureDialog::onPause()
{
	// TODO...
	if (view->stream->isPlaying())
		view->stream->stop();
	tsunami->input->accumulate(false);
	enable("capture_start", true);
	enable("capture_pause", false);
}


void CaptureDialog::onOk()
{
	if (insert())
		delete(this);
}

void CaptureDialog::onClose()
{
	delete(this);
}

void CaptureDialog::onUpdate(Observable *o, const string &message)
{
	setString("capture_time", audio->get_time_str_long(tsunami->input->getSampleCount()));
}

bool CaptureDialog::insert()
{
	msg_db_f("CaptureInsert", 1);
	Track *t;
	int target = getInt("capture_target");
	int i0;
	int s_start = tsunami->win->view->sel_range.start();

	// insert recorded data with some delay
	int dpos = tsunami->input->getDelay();

	if (target >= audio->tracks.num){
		// new track
		t = audio->addTrack(type, audio->tracks.num);
	}else{
		// overwrite
		t = audio->tracks[target];
	}
	i0 = s_start + dpos;

	if (t->type != type){
		tsunami->log->error(format(_("Kann aufgenommene Daten (%s) nicht in Ziel (%s) einf&ugen."), track_type(type).c_str(), track_type(t->type).c_str()));
		return false;
	}

	// insert data
	if (type == t->TYPE_AUDIO){
		Range r = Range(i0, tsunami->input->getSampleCount());
		audio->action_manager->beginActionGroup();
		BufferBox tbuf = t->getBuffers(tsunami->win->view->cur_level, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, tsunami->win->view->cur_level, r);
		tbuf.set(tsunami->input->buffer, 0, 1.0f);
		audio->execute(a);
		audio->action_manager->endActionGroup();
	}else if (type == t->TYPE_MIDI){
		t->insertMidiData(i0, tsunami->input->midi);
	}
	tsunami->input->resetAccumulation();
	return true;
}

