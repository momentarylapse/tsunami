/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "../CaptureConsole.h"
#include "../../../Device/InputStreamMidi.h"
#include "../../../Device/OutputStream.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Device.h"
#include "../../../Audio/Synth/Synthesizer.h"
#include "../../../Data/Song.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Stuff/Log.h"
#include "../../../Tsunami.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = cc->device_manager->chooseDevice(Device::TYPE_MIDI_INPUT);
	input = NULL;
	target = NULL;

	preview_synth = NULL;
	preview_stream = NULL;

	cc->event("capture_midi_source", std::bind(&CaptureConsoleModeMidi::onSource, this));
	cc->event("capture_midi_target", std::bind(&CaptureConsoleModeMidi::onTarget, this));
}

void CaptureConsoleModeMidi::onSource()
{
	int n = cc->getInt("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->setDevice(chosen_device);
	}
}

void CaptureConsoleModeMidi::onTarget()
{
	setTarget(cc->song->tracks[cc->getInt("capture_midi_target")]);
}


void CaptureConsoleModeMidi::setTarget(Track *t)
{
	if (preview_stream)
		delete preview_stream;
	if (preview_synth)
		delete preview_synth;

	target = t;
	preview_synth = (Synthesizer*)t->synth->copy();
	preview_synth->out->setSource(input->out);
	preview_stream = new OutputStream(preview_synth->out);
	preview_stream->setBufferSize(512);
	preview_stream->_play();
	view->setCurTrack(target);
	view->mode_capture->capturing_track = target;
	cc->setInt("capture_midi_target", target->get_index());


	bool ok = (target->type == Track::TYPE_MIDI);
	cc->setString("capture_midi_message", "");
	if (!ok)
		cc->setString("capture_midi_message", format(_("Please select a track of type %s."), track_type(Track::TYPE_MIDI).c_str()));
	cc->enable("capture_start", ok);
}

void CaptureConsoleModeMidi::enterParent()
{
	target = view->cur_track;
}

void CaptureConsoleModeMidi::enter()
{
	sources = cc->device_manager->getGoodDeviceList(Device::TYPE_MIDI_INPUT);

	// add all
	cc->reset("capture_midi_source");
	for (Device *d: sources)
		cc->setString("capture_midi_source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->setInt("capture_midi_source", i);


	// target list
	cc->reset("capture_midi_target");
	for (Track *t: song->tracks)
		cc->addString("capture_midi_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	//cc->addString("capture_midi_target", _("  - create new track -"));

	input = new InputStreamMidi(song->sample_rate);
	input->setBackupMode(BACKUP_MODE_TEMP);
	input->setChunkSize(512);
	input->setUpdateDt(0.005f);
	view->mode_capture->setInputMidi(input);
	cc->peak_meter->setSource(input);

	input->setDevice(chosen_device);

	setTarget(target);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureConsoleModeMidi::leave()
{
	cc->peak_meter->setSource(NULL);
	view->mode_capture->setInputMidi(NULL);
	delete(input);
	input = NULL;
}

void CaptureConsoleModeMidi::pause()
{
	input->accumulate(false);
}

void CaptureConsoleModeMidi::start()
{
	input->resetSync();
	input->accumulate(true);
	cc->enable("capture_midi_source", false);
	cc->enable("capture_midi_target", false);
}

void CaptureConsoleModeMidi::stop()
{
	preview_stream->_stop();
	input->stop();
}

void CaptureConsoleModeMidi::dump()
{
	input->accumulate(false);
	input->resetAccumulation();
	cc->enable("capture_midi_source", true);
	cc->enable("capture_midi_target", true);
}

bool CaptureConsoleModeMidi::insert()
{
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->getDelay();

	int i0 = s_start + dpos;

	if (target->type != Track::TYPE_MIDI){
		tsunami->log->error(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::TYPE_MIDI).c_str(), track_type(target->type).c_str()));
		return false;
	}

	// insert data
	target->insertMidiData(i0, midi_events_to_notes(input->midi));

	input->resetAccumulation();
	return true;
}

int CaptureConsoleModeMidi::getSampleCount()
{
	return input->getSampleCount();
}

bool CaptureConsoleModeMidi::isCapturing()
{
	return input->isCapturing();
}


