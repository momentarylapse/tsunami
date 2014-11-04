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
#include "../../Audio/AudioStream.h"
#include "../../Audio/AudioRenderer.h"
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

	// dialog
	peak_meter = new PeakMeter(this, "capture_level", tsunami->input);
	setString("capture_time", a->get_time_str_long(0));
	enable("capture_delete", false);
	enable("capture_pause", false);
	enable("capture_type:audio", false);
	enable("capture_type:midi", false);
	enable("ok", false);

	foreach(Track *t, a->track)
		addString("capture_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_target", _("neue Spur anlegen (" + track_type(Track::TYPE_AUDIO) + ")"));
	addString("capture_target", _("neue Spur anlegen (" + track_type(Track::TYPE_MIDI) + ")"));



	if (view->cur_track)
		setTarget(view->cur_track->get_index());
	else
		setTarget(audio->track.num);

	event("cancel", this, &CaptureDialog::onClose);
	event("hui:close", this, &CaptureDialog::onClose);
	event("ok", this, &CaptureDialog::onOk);
	//event("capture_type:audio", this, &CaptureDialog::onTypeAudio);
	//event("capture_type:midi", this, &CaptureDialog::onTypeMidi);
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

static Array<AudioInputMidi::MidiPort> ports;

void OnSelectPort()
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
}

void CaptureDialog::onTypeMidi()
{
	setType(Track::TYPE_MIDI);
}

void CaptureDialog::setTarget(int index)
{
	if (index < audio->track.num){
		Track *t = audio->track[index];
		setType(t->type);
	}else if (index == audio->track.num)
		setType(Track::TYPE_AUDIO);
	else
		setType(Track::TYPE_MIDI);
	setInt("capture_target", index);
}

void CaptureDialog::setType(int _type)
{
	if (type == _type)
		return;
	type = _type;
	if (type == Track::TYPE_MIDI){
		SelectMidiPort(this);
		check("capture_type:midi", true);
	}else if (type == Track::TYPE_AUDIO){
		check("capture_type:audio", true);
	}else{
	}

	if (!tsunami->input->start(type, audio->sample_rate)){
		/*HuiErrorBox(MainWin, _("Fehler"), _("Konnte Aufnahmeger&at nicht &offnen"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureDialog::onStart()
{
	tsunami->renderer->prepare(audio, tsunami->win->view->getPlaybackSelection(), false);
	view->stream->play();

	tsunami->input->resetSync();
	tsunami->input->accumulate(true);
	enable("capture_start", false);
	enable("capture_pause", true);
	enable("capture_delete", true);
	enable("ok", true);
	//enable("capture_type:audio", false);
	//enable("capture_type:midi", false);
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

	if (target >= audio->track.num){
		// new track
		t = audio->addTrack(type, audio->track.num);
	}else{
		// overwrite
		t = audio->track[target];
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

