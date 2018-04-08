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
#include "../../../Data/Song.h"
#include "../../../Data/SongSelection.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = cc->device_manager->chooseDevice(Device::Type::MIDI_INPUT);
	input = NULL;
	target = NULL;

	peak_meter = NULL;
	preview_synth = NULL;
	preview_stream = NULL;

	cc->enable("capture_midi_target", false);

	cc->event("capture_midi_source", std::bind(&CaptureConsoleModeMidi::onSource, this));
	//cc->event("capture_midi_target", std::bind(&CaptureConsoleModeMidi::onTarget, this));
}

void CaptureConsoleModeMidi::onSource()
{
	int n = cc->getInt("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}

void CaptureConsoleModeMidi::onTarget()
{
	//setTarget(cc->song->tracks[cc->getInt("capture_midi_target")]);
}


void CaptureConsoleModeMidi::setTarget(Track *t)
{
	if (preview_stream)
		delete preview_stream;
	if (peak_meter)
		delete peak_meter;
	if (preview_synth)
		delete preview_synth;

	target = t;
	view->setCurTrack(target);
	preview_synth = (Synthesizer*)t->synth->copy();
	preview_synth->set_source(input->out);
	peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
	peak_meter->set_source(preview_synth->out);
	preview_stream = new OutputStream(session, peak_meter->out);
	preview_stream->set_buffer_size(512);
	preview_stream->play();
	view->setCurTrack(target);
	view->mode_capture->capturing_track = target;
	cc->setInt("capture_midi_target", target->get_index());


	bool ok = (target->type == Track::Type::MIDI);
	cc->setString("capture_midi_message", "");
	if (!ok)
		cc->setString("capture_midi_message", format(_("Please select a track of type %s."), track_type(Track::Type::MIDI).c_str()));
	cc->enable("capture_start", ok);
}

void CaptureConsoleModeMidi::enterParent()
{
}

void CaptureConsoleModeMidi::enter()
{
	sources = cc->device_manager->getGoodDeviceList(Device::Type::MIDI_INPUT);
	cc->hideControl("capture_midi_grid", false);

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

	input = new InputStreamMidi(session);
	input->set_chunk_size(512);
	input->set_update_dt(0.005f);
	view->mode_capture->setInputMidi(input);
	cc->peak_meter->setSource(NULL);//input);

	input->set_device(chosen_device);

	for (const Track *t: view->sel.tracks)
		if (t->type == t->Type::MIDI)
			setTarget((Track*)t);

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
	input->reset_sync();
	input->accumulate(true);
	cc->enable("capture_midi_source", false);
	//cc->enable("capture_midi_target", false);
}

void CaptureConsoleModeMidi::stop()
{
	preview_stream->stop();
	input->stop();
}

void CaptureConsoleModeMidi::dump()
{
	input->accumulate(false);
	input->reset_accumulation();
	cc->enable("capture_midi_source", true);
	//cc->enable("capture_midi_target", true);
}

bool CaptureConsoleModeMidi::insert()
{
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->get_delay();

	int i0 = s_start + dpos;

	if (target->type != Track::Type::MIDI){
		session->e(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::Type::MIDI).c_str(), track_type(target->type).c_str()));
		return false;
	}

	// insert data
	target->insertMidiData(i0, midi_events_to_notes(input->midi));

	input->reset_accumulation();
	return true;
}

int CaptureConsoleModeMidi::getSampleCount()
{
	return input->get_sample_count();
}

bool CaptureConsoleModeMidi::isCapturing()
{
	return input->is_capturing();
}


