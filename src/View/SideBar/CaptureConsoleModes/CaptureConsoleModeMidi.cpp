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
#include "../../../Data/base.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/SongSelection.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = nullptr;
	input = nullptr;
	target = nullptr;

	peak_meter = nullptr;
	preview_synth = nullptr;
	preview_stream = nullptr;

	cc->event("source", std::bind(&CaptureConsoleModeMidi::on_source, this));
}

void CaptureConsoleModeMidi::on_source()
{
	int n = cc->getInt("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}


void CaptureConsoleModeMidi::set_target(Track *t)
{
	if (preview_stream)
		delete preview_stream;
	if (peak_meter)
		delete peak_meter;
	if (preview_synth)
		delete preview_synth;

	target = t;
	// FIXME ...
	//view->setCurTrack(target);
	preview_synth = (Synthesizer*)t->synth->copy();
	preview_synth->set_source(input->out);
	peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
	peak_meter->set_source(preview_synth->out);
	preview_stream = new OutputStream(session, peak_meter->out);
	preview_stream->set_buffer_size(512);
	preview_stream->play();
	view->mode_capture->capturing_track = target;


	bool ok = (target->type == SignalType::MIDI);
	cc->setString("message", "");
	if (!ok)
		cc->setString("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::MIDI).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeMidi::enter_parent()
{
}

void CaptureConsoleModeMidi::enter()
{
	chosen_device = cc->device_manager->chooseDevice(DeviceType::MIDI_INPUT);
	sources = cc->device_manager->getGoodDeviceList(DeviceType::MIDI_INPUT);
	cc->hideControl("single_grid", false);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->setString("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->setInt("source", i);


	input = new InputStreamMidi(session);
	input->set_chunk_size(512);
	input->set_update_dt(0.005f);
	view->mode_capture->set_input_midi(input);
	cc->peak_meter->setSource(nullptr);//input);

	input->set_device(chosen_device);

	for (const Track *t: view->sel.tracks)
		if (t->type == SignalType::MIDI)
			set_target((Track*)t);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
}

void CaptureConsoleModeMidi::leave()
{
	cc->peak_meter->setSource(nullptr);
	view->mode_capture->set_input_midi(nullptr);
	delete(input);
	input = nullptr;
}

void CaptureConsoleModeMidi::pause()
{
	input->accumulate(false);
}

void CaptureConsoleModeMidi::start()
{
	input->reset_sync();
	input->accumulate(true);
	cc->enable("source", false);
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
	cc->enable("source", true);
}

bool CaptureConsoleModeMidi::insert()
{
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->get_delay();

	int i0 = s_start + dpos;

	if (target->type != SignalType::MIDI){
		session->e(format(_("Can't insert recorded data (%s) into target (%s)."), signal_type_name(SignalType::MIDI).c_str(), signal_type_name(target->type).c_str()));
		return false;
	}

	// insert data
	target->layers[0]->insertMidiData(i0, midi_events_to_notes(input->midi).duplicate());

	input->reset_accumulation();
	return true;
}

int CaptureConsoleModeMidi::get_sample_count()
{
	return input->get_sample_count();
}

bool CaptureConsoleModeMidi::is_capturing()
{
	return input->is_capturing();
}


