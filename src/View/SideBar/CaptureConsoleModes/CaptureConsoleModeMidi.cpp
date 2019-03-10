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
#include "../../../Module/Midi/MidiRecorder.h"
#include "../../../Module/SignalChain.h"
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

	cc->event("source", [&]{ on_source(); });
}

void CaptureConsoleModeMidi::on_source()
{
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}


void CaptureConsoleModeMidi::set_target(const Track *t)
{
	target = t;
	preview_synth = (Synthesizer*)t->synth->copy();
	chain->_add(preview_synth);

	//cc->enable("start", true);
}

void CaptureConsoleModeMidi::enter()
{
	chosen_device = session->device_manager->choose_device(DeviceType::MIDI_INPUT);
	sources = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);
	cc->hide_control("single_grid", false);

	chain = new SignalChain(session, "capture");

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->set_int("source", i);


	input = (InputStreamMidi*)chain->add(ModuleType::STREAM, "MidiInput");
	input->set_update_dt(0.005f);
	cc->peak_meter->set_source(nullptr);//input);

	input->set_device(chosen_device);
	recorder = (MidiRecorder*)chain->add(ModuleType::PLUMBING, "MidiRecorder");
	//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");

	for (const Track *t: view->sel.tracks)
		if (t->type == SignalType::MIDI)
			set_target(t);
			
	//preview_synth->plug(0, input, 0);
	peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	preview_stream = (OutputStream*)chain->add(ModuleType::STREAM, "AudioOutput");
	preview_stream->set_buffer_size(512);
	
	
	chain->connect(input, 0, recorder, 0);
	chain->connect(recorder, 0, preview_synth, 0);
	chain->connect(preview_synth, 0, peak_meter, 0);
	chain->connect(peak_meter, 0, preview_stream, 0);

	chain->start();
	view->mode_capture->set_data({CaptureTrackData((Track*)target, input, recorder)});
}

void CaptureConsoleModeMidi::allow_change_device(bool allow)
{
	cc->enable("source", allow);
}

void CaptureConsoleModeMidi::leave()
{
	cc->peak_meter->set_source(nullptr);
	view->mode_capture->set_data({});
	delete chain;
	chain = nullptr;
}


