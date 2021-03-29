/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../../Data/base.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/SongSelection.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../../Module/SignalChain.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioOutput.h"
#include "../../../Device/Stream/MidiInput.h"
#include "../../../Module/Midi/MidiAccumulator.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	CaptureTrackData c;
	c.id_peaks = "level";
	c.id_source = "source";

	items.add(c);

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeMidi::on_source() {
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num)) {
		items[0].set_device(sources[n]);
	}
}


void CaptureConsoleModeMidi::set_target(const Track *t) {
	items[0].track = const_cast<Track*>(t);
	items[0].synth = (Synthesizer*)t->synth.get()->copy();
	chain->_add(items[0].synth);

	//cc->enable("start", true);
}

void CaptureConsoleModeMidi::enter() {
	cc->hide_control("single_grid", false);

	chain = session->create_signal_chain_system("capture");

	auto &c = items[0];
	c.chain = chain.get();
	c.panel = cc;

	c.input = (MidiInput*)chain->add(ModuleCategory::STREAM, "MidiInput");
	c.input->subscribe(this, [=]{ update_device_list(); });
	c.accumulator = chain->add(ModuleCategory::PLUMBING, "MidiAccumulator");
	//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");

	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::MIDI))
			set_target(t);
			
	//preview_synth->plug(0, input, 0);
	c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
	auto preview_stream = (AudioOutput*)chain->add(ModuleCategory::STREAM, "AudioOutput");
	chain->mark_all_modules_as_system();
	

	chain->set_buffer_size(512);
	chain->connect(c.input, 0, c.accumulator, 0);
	chain->connect(c.accumulator, 0, c.synth, 0);
	chain->connect(c.synth, 0, c.peak_meter, 0);
	chain->connect(c.peak_meter, 0, preview_stream, 0);

	cc->peak_meter_display->set_source(c.peak_meter);
	cc->set_options(c.id_peaks, format("height=%d", PeakMeterDisplay::good_size(2)));

	update_device_list();
	session->device_manager->subscribe(this, [=]{ update_device_list(); });

	chain->start();
	view->mode_capture->set_data(items);
}

void CaptureConsoleModeMidi::allow_change_device(bool allow) {
	items[0].allow_edit(allow);
}

void CaptureConsoleModeMidi::update_device_list() {
	sources = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == items[0].midi_input()->get_device())
			cc->set_int("source", i);
}

void CaptureConsoleModeMidi::leave() {
	session->device_manager->unsubscribe(this);
	cc->peak_meter_display->set_source(nullptr);
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}


