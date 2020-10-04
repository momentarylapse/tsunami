/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "../CaptureConsole.h"
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
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioOutput.h"
#include "../../../Device/Stream/MidiInput.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	input = nullptr;
	target = nullptr;

	peak_meter = nullptr;
	preview_synth = nullptr;
	preview_stream = nullptr;

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeMidi::on_source() {
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num)) {
		input->set_device(sources[n]);
	}
}


void CaptureConsoleModeMidi::set_target(const Track *t) {
	target = t;
	preview_synth = (Synthesizer*)t->synth.get()->copy();
	chain->_add(preview_synth);

	//cc->enable("start", true);
}

void CaptureConsoleModeMidi::enter() {
	cc->hide_control("single_grid", false);

	chain = session->create_signal_chain_system("capture");


	input = (MidiInput*)chain->add(ModuleType::STREAM, "MidiInput");
	input->subscribe(this, [=]{ update_device_list(); });
	auto *recorder = chain->add(ModuleType::PLUMBING, "MidiRecorder");
	//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");

	for (Track *t: view->song->tracks)
		if (view->sel.has(t) and (t->type == SignalType::MIDI))
			set_target(t);
			
	//preview_synth->plug(0, input, 0);
	peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	preview_stream = (AudioOutput*)chain->add(ModuleType::STREAM, "AudioOutput");
	chain->mark_all_modules_as_system();
	

	chain->set_buffer_size(512);
	chain->connect(input, 0, recorder, 0);
	chain->connect(recorder, 0, preview_synth, 0);
	chain->connect(preview_synth, 0, peak_meter, 0);
	chain->connect(peak_meter, 0, preview_stream, 0);

	cc->peak_meter->set_source(peak_meter);

	update_device_list();
	session->device_manager->subscribe(this, [=]{ update_device_list(); });

	chain->start();
	view->mode_capture->set_data({{(Track*)target, input, recorder}});
}

void CaptureConsoleModeMidi::allow_change_device(bool allow) {
	cc->enable("source", allow);
}

void CaptureConsoleModeMidi::update_device_list() {
	sources = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == input->get_device())
			cc->set_int("source", i);
}

void CaptureConsoleModeMidi::leave() {
	session->device_manager->unsubscribe(this);
	cc->peak_meter->set_source(nullptr);
	chain = nullptr;
}


