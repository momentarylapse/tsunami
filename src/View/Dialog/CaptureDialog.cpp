/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "CaptureDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Audio/AudioInputAny.h"
#include "../../Audio/AudioInputAudio.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/SongRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../../Stuff/Log.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

CaptureDialog::CaptureDialog(HuiWindow *_parent, bool _allow_parent, Song *s):
	HuiWindow("record_dialog", _parent, _allow_parent),
	Observer("CaptureDialog")
{
	song = s;
	view = tsunami->win->view;
	type = -1;
	input = new AudioInputAny(song->sample_rate);
	input->setSaveMode(true);
	subscribe(input);
	view->setInput(input);

	temp_synth = CreateSynthesizer("");

	selected_audio_source = AudioInputAudio::getFavoriteDevice();


	// dialog
	peak_meter = new PeakMeter(this, "capture_level", input);
	setString("capture_time", s->get_time_str_long(0));
	enable("capture_delete", false);
	enable("capture_pause", false);
	enable("ok", false);

	// target list
	foreach(Track *t, s->tracks)
		addString("capture_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_target", _("  - neue Spur anlegen -"));

	if (view->cur_track){
		setTarget(view->cur_track->get_index());
	}else{
		setTarget(song->tracks.num);
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


	// automatically start
	onStart();
}

CaptureDialog::~CaptureDialog()
{
	view->stream->stop();

	delete(peak_meter);
	delete(temp_synth);

	view->setInput(NULL);
	unsubscribe(input);
	delete(input);
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

void CaptureDialog::onSource()
{
	int n = HuiCurWindow->getInt("");
	if (type == Track::TYPE_MIDI){
		if (n >= 0)
			input->connectMidiPort(midi_ports[n]);
	}else if (type == Track::TYPE_AUDIO){
		if ((n >= 0) and (n < audio_sources.num)){
			selected_audio_source = audio_sources[n];
			input->setDevice(selected_audio_source);
		}
	}
}

void CaptureDialog::updateMidiPortList()
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

void CaptureDialog::updateAudioSourceList()
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

void CaptureDialog::setType(int _type)
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

void CaptureDialog::onStart()
{
	view->renderer->prepare(song, view->getPlaybackSelection(), false);
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

void CaptureDialog::onDelete()
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

void CaptureDialog::onPause()
{
	// TODO...
	if (view->stream->isPlaying())
		view->stream->stop();
	input->accumulate(false);
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

void CaptureDialog::updateTime()
{
	setString("capture_time", song->get_time_str_long(input->getSampleCount()));
}

void CaptureDialog::onUpdate(Observable *o, const string &message)
{
	updateTime();
}

bool CaptureDialog::insert()
{
	msg_db_f("CaptureInsert", 1);
	Track *t;
	int target = getInt("capture_target");
	int i0;
	int s_start = tsunami->win->view->sel_range.start();

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
		BufferBox tbuf = t->getBuffers(tsunami->win->view->cur_level, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, tsunami->win->view->cur_level, r);
		tbuf.set(*input->buffer, 0, 1.0f);
		song->execute(a);
		song->action_manager->endActionGroup();
	}else if (type == t->TYPE_MIDI){
		t->insertMidiData(i0, *input->midi);
	}
	input->resetAccumulation();
	return true;
}

