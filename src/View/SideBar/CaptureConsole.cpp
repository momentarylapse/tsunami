/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Audio/AudioInputAny.h"
#include "../../Audio/AudioInputAudio.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "../../Stuff/Log.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "CaptureConsole.h"
#include "../../Audio/DeviceManager.h"

CaptureConsole::CaptureConsole(Song *s, AudioView *v):
	SideBarConsole(_("Aufnahme")),
	Observer("CaptureDialog")
{
	song = s;
	view = v;
	type = -1;
	input = NULL;


	// dialog
	setBorderWidth(5);
	embedDialog("record_dialog", 0, 0);

	temp_synth = CreateSynthesizer("", s);

	selected_audio_source = AudioInputAudio::getFavoriteDevice();


	// dialog
	peak_meter = new PeakMeter(this, "capture_level", input);



	event("cancel", this, &CaptureConsole::onClose);
	//event("hui:close", this, &CaptureConsole::onClose);
	event("ok", this, &CaptureConsole::onOk);
	event("capture_type:audio", this, &CaptureConsole::onTypeAudio);
	event("capture_type:midi", this, &CaptureConsole::onTypeMidi);
	event("capture_source", this, &CaptureConsole::onSource);
	event("capture_target", this, &CaptureConsole::onTarget);
	event("capture_start", this, &CaptureConsole::onStart);
	event("capture_delete", this, &CaptureConsole::onDelete);
	event("capture_pause", this, &CaptureConsole::onPause);
}

CaptureConsole::~CaptureConsole()
{
	delete(peak_meter);
	delete(temp_synth);
}

void CaptureConsole::onEnter()
{
	type = -1;
	input = new AudioInputAny(song->sample_rate);
	input->setSaveMode(true);
	input->setChunkSize(4096);
	input->setUpdateDt(0.03f);
	subscribe(input);
	view->setInput(input);
	peak_meter->setSource(input);



	selected_audio_source = AudioInputAudio::getFavoriteDevice();


	// dialog
	setString("capture_time", song->get_time_str_long(0));
	enable("capture_delete", false);
	enable("capture_pause", false);
	enable("ok", false);

	// target list
	reset("capture_target");
	foreach(Track *t, song->tracks)
		addString("capture_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_target", _("  - neue Spur anlegen -"));

	if (view->cur_track){
		setTarget(view->cur_track->get_index());
	}else{
		setTarget(song->tracks.num);
		setType(Track::TYPE_AUDIO);
	}


	view->setMode(view->mode_capture);

	// automatically start
	onStart();
}

void CaptureConsole::onLeave()
{
	peak_meter->setSource(NULL);
	view->stream->stop();

	view->setInput(NULL);
	unsubscribe(input);
	delete(input);

	view->setMode(view->mode_default);
}

void CaptureConsole::onTarget()
{
	int target = getInt("capture_target");
	setTarget(target);
}


void CaptureConsole::onTypeAudio()
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

void CaptureConsole::onTypeMidi()
{
	setType(Track::TYPE_MIDI);
}

void CaptureConsole::setTarget(int index)
{
	if (index < song->tracks.num){
		Track *t = song->tracks[index];
		if (t->type == t->TYPE_TIME){
			index = song->tracks.num;
			setType(Track::TYPE_AUDIO);
			input->setPreviewSynthesizer(temp_synth);
		}else{
			setType(t->type);
			input->setPreviewSynthesizer(t->synth);
		}
	}else{
		input->setPreviewSynthesizer(temp_synth);
	}
	view->capturing_track = index;
	setInt("capture_target", index);
}

void CaptureConsole::onSource()
{
	int n = getInt("");
	if (type == Track::TYPE_MIDI){
		if (n >= 0){
			input->connectMidiPort(midi_ports[n]);
		}
	}else if (type == Track::TYPE_AUDIO){
		if ((n >= 0) and (n < audio_sources.num)){
			selected_audio_source = audio_sources[n];
			input->setDevice(selected_audio_source);
		}
	}
}

void CaptureConsole::updateMidiPortList()
{
	midi_ports = input->findMidiPorts();
	AudioInputMidi::MidiPort cur = input->getCurMidiPort();

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

void CaptureConsole::updateAudioSourceList()
{
	audio_sources = AudioInputAudio::getDevices();

	// add all
	reset("capture_source");
	foreach(string &d, audio_sources)
		setString("capture_source", d);

	// add "default"
	audio_sources.add("");
	setString("capture_source", _("        - Standard -"));

	// select current
	foreachi(string &d, audio_sources, i)
		if (d == selected_audio_source)
			setInt("capture_source", i);

	enable("capture_source", true);

}

void CaptureConsole::setType(int _type)
{
	if (type == _type)
		return;
	type = _type;

	// consistency test: ...
	int target = getInt("capture_target");
	if ((target >= 0) and (target < song->tracks.num))
		if (type != song->tracks[target]->type)
			setInt("capture_target", song->tracks.num);

	input->setType(type);

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

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Fehler"), _("Konnte Aufnahmeger&at nicht &offnen"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureConsole::onStart()
{
	view->renderer->prepare(view->getPlaybackSelection(), false);
	view->stream->play();

	input->resetSync();
	input->accumulate(true);
	enable("capture_start", false);
	enable("capture_pause", true);
	enable("capture_delete", true);
	enable("ok", true);
	enable("capture_type:audio", false);
	enable("capture_type:midi", false);
	enable("capture_source", false);
	enable("capture_target", false);
}

void CaptureConsole::onDelete()
{
	if (view->stream->isPlaying())
		view->stream->stop();
	input->resetAccumulation();
	input->accumulate(false);
	enable("capture_start", true);
	enable("capture_pause", false);
	enable("capture_delete", false);
	enable("capture_type:audio", true);
	enable("capture_type:midi", true);
	enable("capture_source", true);
	enable("capture_target", true);
	enable("ok", false);
	updateTime();
}

void CaptureConsole::onPause()
{
	// TODO...
	if (view->stream->isPlaying())
		view->stream->stop();
	input->accumulate(false);
	enable("capture_start", true);
	enable("capture_pause", false);
}


void CaptureConsole::onOk()
{
	if (insert())
		((SideBar*)parent)->_hide();
}

void CaptureConsole::onClose()
{
	((SideBar*)parent)->_hide();
}

void CaptureConsole::updateTime()
{
	setString("capture_time", song->get_time_str_long(input->getSampleCount()));
}

void CaptureConsole::onUpdate(Observable *o, const string &message)
{
	updateTime();
}

bool CaptureConsole::insert()
{
	msg_db_f("CaptureInsert", 1);
	Track *t;
	int target = getInt("capture_target");
	int i0;
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->getDelay();

	if (target >= song->tracks.num){
		// new track
		t = song->addTrack(type, song->tracks.num);
	}else{
		// overwrite
		t = song->tracks[target];
	}
	i0 = s_start + dpos;

	if (t->type != type){
		tsunami->log->error(format(_("Kann aufgenommene Daten (%s) nicht in Ziel (%s) einf&ugen."), track_type(type).c_str(), track_type(t->type).c_str()));
		return false;
	}

	// insert data
	if (type == t->TYPE_AUDIO){
		Range r = Range(i0, input->getSampleCount());
		song->action_manager->beginActionGroup();
		BufferBox tbuf = t->getBuffers(view->cur_level, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, view->cur_level, r);
		tbuf.set(*input->buffer, 0, 1.0f);
		song->execute(a);
		song->action_manager->endActionGroup();
	}else if (type == t->TYPE_MIDI){
		t->insertMidiData(i0, midi_events_to_notes(*input->midi));
	}
	input->resetAccumulation();
	return true;
}

