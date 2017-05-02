/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Device/OutputStream.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "../../Stuff/Log.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "CaptureConsole.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Device.h"

CaptureConsole::CaptureConsole(Song *s, AudioView *v):
	SideBarConsole(_("Recording")),
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

	device_manager = tsunami->device_manager;
	chosen_device_audio = NULL;
	chosen_device_midi = NULL;


	// dialog
	peak_meter = new PeakMeter(this, "capture_level", input, view);



	event("cancel", this, &CaptureConsole::onCancel);
	//event("hui:close", this, &CaptureConsole::onClose);
	event("ok", this, &CaptureConsole::onOk);
	event("capture_type", this, &CaptureConsole::onType);
	event("capture_audio_source", this, &CaptureConsole::onSourceAudio);
	event("capture_audio_target", this, &CaptureConsole::onTargetAudio);
	event("capture_midi_source", this, &CaptureConsole::onSourceMidi);
	event("capture_midi_target", this, &CaptureConsole::onTargetMidi);
	event("capture_start", this, &CaptureConsole::onStart);
	event("capture_delete", this, &CaptureConsole::onDelete);
	event("capture_pause", this, &CaptureConsole::onPause);
}

CaptureConsole::~CaptureConsole()
{
	delete(peak_meter);
	delete(temp_synth);
}

inline int dev_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return Device::TYPE_AUDIO_INPUT;
	return Device::TYPE_MIDI_INPUT;
}

void CaptureConsole::onEnter()
{
	type = -1;
	input = NULL;

	chosen_device_audio = NULL;
	chosen_device_midi = NULL;

	chosen_device_audio = device_manager->chooseDevice(Device::TYPE_AUDIO_INPUT);
	chosen_device_midi = device_manager->chooseDevice(Device::TYPE_MIDI_INPUT);

	updateSourceList();


	// dialog
	setString("capture_time", song->get_time_str_long(0));
	enable("capture_delete", false);
	enable("capture_pause", false);
	enable("ok", false);

	// target list
	reset("capture_audio_target");
	for (Track *t: song->tracks)
		addString("capture_audio_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_audio_target", _("  - create new track -"));

	// target list midi
	reset("capture_midi_target");
	for (Track *t: song->tracks)
		addString("capture_midi_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	addString("capture_midi_target", _("  - create new track -"));

	// target list multi
	reset("capture_multi_list");
	for (Track *t: song->tracks)
		addString("capture_multi_list", t->getNiceName() + "\\" + track_type(t->type) + "\\ - none -");


	if (view->cur_track){
		if (view->cur_track->type == Track::TYPE_AUDIO)
			beginAudio();
		else if (view->cur_track->type == Track::TYPE_MIDI)
			beginMidi();

		setTargetAudio(view->cur_track->get_index());
		setTargetMidi(view->cur_track->get_index());
	}else{
		beginAudio();
		setTargetAudio(song->tracks.num);
		setTargetMidi(song->tracks.num);
	}


	//chosen_device = input->getDevice();//device_manager->chooseDevice(dev_type(type));

	view->setMode(view->mode_capture);

	// automatically start
	onStart();
}

void CaptureConsole::onLeave()
{
	if (input->isCapturing())
		insert();

	peak_meter->setSource(NULL);
	view->stream->stop();

	view->setInput(NULL);
	unsubscribe(input);
	delete(input);

	view->setMode(view->mode_default);
}

void CaptureConsole::onTargetAudio()
{
	int target = getInt("capture_audio_target");
	setTargetAudio(target);
}

void CaptureConsole::onTargetMidi()
{
	int target = getInt("capture_midi_target");
	setTargetMidi(target);
}


void CaptureConsole::onType()
{
	int n = getInt("capture_type");
	if (n == 0)
		beginAudio();
	if (n == 1)
		beginMidi();
	if (n == 2)
		beginMulti();
}


void CaptureConsole::beginMode(int mode)
{
	if (mode == Track::TYPE_AUDIO)
		beginAudio();
	if (mode == Track::TYPE_MIDI)
		beginMidi();
}

void CaptureConsole::beginAudio()
{
	if (input){
		delete(input);
		input = NULL;
	}

	type = Track::TYPE_AUDIO;
	setInt("capture_type", 0);

	input = new InputStreamAudio(song->sample_rate);
	input->setBackupMode(BACKUP_MODE_TEMP);
	input->setChunkSize(4096);
	input->setUpdateDt(0.03f);
	subscribe(input);
	view->setInput(input);
	peak_meter->setSource(input);

	input->setDevice(chosen_device_audio);

	enable("capture_audio_source", false);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureConsole::beginMidi()
{
	if (input){
		delete(input);
		input = NULL;
	}

	type = Track::TYPE_MIDI;
	setInt("capture_type", 1);

	input = new InputStreamMidi(song->sample_rate);
	input->setBackupMode(BACKUP_MODE_TEMP);
	input->setChunkSize(4096);
	input->setUpdateDt(0.03f);
	subscribe(input);
	view->setInput(input);
	peak_meter->setSource(input);

	input->setDevice(chosen_device_midi);

	enable("capture_midi_source", false);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureConsole::beginMulti()
{
	type = -1;
	setInt("capture_type", 2);
}

void CaptureConsole::setTargetAudio(int index)
{
	if (type != Track::TYPE_AUDIO)
		return;

	if (index < song->tracks.num){
		Track *t = song->tracks[index];
		if (t->type == t->TYPE_TIME){
			index = song->tracks.num;
		}
	}
	view->capturing_track = index;
	setInt("capture_audio_target", index);
}

void CaptureConsole::setTargetMidi(int index)
{
	if (type != Track::TYPE_MIDI)
		return;
	if (index < song->tracks.num){
		Track *t = song->tracks[index];
		if (t->type == t->TYPE_TIME){
			index = song->tracks.num;
			((InputStreamMidi*)input)->setPreviewSynthesizer(temp_synth);
		}else{
			((InputStreamMidi*)input)->setPreviewSynthesizer(t->synth);
		}
	}else{
		((InputStreamMidi*)input)->setPreviewSynthesizer(temp_synth);
	}
	view->capturing_track = index;
	setInt("capture_midi_target", index);
}

void CaptureConsole::onSourceAudio()
{
	if (type != Track::TYPE_AUDIO)
		return;
	int n = getInt("");
	if ((n >= 0) and (n < sources_audio.num)){
		chosen_device_audio = sources_audio[n];
		input->setDevice(chosen_device_audio);
	}
}

void CaptureConsole::onSourceMidi()
{
	if (type != Track::TYPE_MIDI)
		return;
	int n = getInt("");
	if ((n >= 0) and (n < sources_midi.num)){
		chosen_device_midi = sources_midi[n];
		input->setDevice(chosen_device_midi);
	}
}

void CaptureConsole::updateSourceList()
{
	sources_audio = device_manager->getGoodDeviceList(Device::TYPE_AUDIO_INPUT);
	sources_midi = device_manager->getGoodDeviceList(Device::TYPE_MIDI_INPUT);

	// add all
	reset("capture_audio_source");
	for (Device *d: sources_audio)
		setString("capture_audio_source", d->get_name());

	// add all
	reset("capture_midi_source");
	for (Device *d: sources_midi)
		setString("capture_midi_source", d->get_name());

	// select current
	foreachi(Device *d, sources_audio, i)
		if (d == chosen_device_audio)
			setInt("capture_audio_source", i);
	foreachi(Device *d, sources_midi, i)
		if (d == chosen_device_midi)
			setInt("capture_midi_source", i);
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
	enable("capture_type", false);
	enable("capture_audio_source", false);
	enable("capture_audio_target", false);
	enable("capture_midi_source", false);
	enable("capture_midi_target", false);
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
	enable("capture_type", true);
	enable("capture_audio_source", true);
	enable("capture_audio_target", true);
	enable("capture_midi_source", true);
	enable("capture_midi_target", true);
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
	input->stop();
	if (insert())
		((SideBar*)parent)->_hide();
}

void CaptureConsole::onCancel()
{
	input->stop();
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
	Track *t;
	int target = getInt("capture_audio_target");
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
		tsunami->log->error(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(type).c_str(), track_type(t->type).c_str()));
		return false;
	}

	// insert data
	if (type == t->TYPE_AUDIO){
		Range r = Range(i0, input->getSampleCount());
		song->action_manager->beginActionGroup();
		BufferBox tbuf = t->getBuffers(view->cur_layer, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, view->cur_layer, r);
		tbuf.set(((InputStreamAudio*)input)->buffer, 0, 1.0f);
		song->execute(a);
		song->action_manager->endActionGroup();
	}else if (type == t->TYPE_MIDI){
		t->insertMidiData(i0, midi_events_to_notes(((InputStreamMidi*)input)->midi));
	}
	input->resetAccumulation();
	return true;
}

