/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "../CaptureConsole.h"
#include "../../../Module/SignalChain.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Session.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"

#include "../../../Module/Audio/AudioSucker.h"
#include "../../../Module/Audio/PeakMeter.h"
//#include "../../../Module/Midi/MidiSucker.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/ModuleFactory.h"
#include "../../../Module/SignalChain.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioInput.h"
#include "../../../Device/Stream/MidiInput.h"

CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_cc) {
	cc = _cc;
	song = cc->song;
	view = cc->view;
	session = cc->session;
	chain = nullptr;
}

void CaptureConsoleMode::start_sync_before() {
	for (auto &d: view->mode_capture->data)
		d.start_sync_before(view->output_stream);
}

void CaptureConsoleMode::sync() {
	for (auto &d: view->mode_capture->data)
		d.sync(view->output_stream);
}

// TODO fix transport in unconnected midi chains

void CaptureConsoleMode::update_data_from_items() {

	chain = session->create_signal_chain_system("capture");

	Array<CaptureTrackData> data;

	for (auto &c: items) {
		auto t = c.track;
		if (t->type == SignalType::AUDIO) {
			c.input_audio = (AudioInput*)chain->add(ModuleCategory::STREAM, "AudioInput");
			c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			auto *recorder_audio = chain->add(ModuleCategory::PLUMBING, "AudioRecorder");
			auto *sucker = chain->add(ModuleCategory::PLUMBING, "AudioSucker");
			if (c.device) {
				c.input_audio->set_device(c.device);
				chain->connect(c.input_audio, 0, c.peak_meter, 0);
			}
			chain->connect(c.peak_meter, 0, recorder_audio, 0);
			chain->connect(recorder_audio, 0, sucker, 0);
			data.add({c.track, c.input_audio, recorder_audio});
		} else if (t->type == SignalType::MIDI) {
			c.input_midi = (MidiInput*)chain->add(ModuleCategory::STREAM, "MidiInput");
			c.recorder = chain->add(ModuleCategory::PLUMBING, "MidiRecorder");
			auto *synth = chain->_add(t->synth->copy());
			c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
			auto *out = chain->add(ModuleCategory::STREAM, "AudioOutput");
			//if (c.device) {
			//	c.input_midi->set_device(c.device);
				chain->connect(c.input_midi, 0, c.recorder, 0);
			//}
			chain->connect(c.recorder, 0, synth, 0);
			chain->connect(synth, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, out, 0);
			data.add({c.track, c.input_midi, c.recorder});
		}
		c.peak_meter_display->set_source(c.peak_meter);
	}
	chain->mark_all_modules_as_system();

	view->mode_capture->set_data(data);
}

void CaptureConsoleMode::CaptureItem::set_device(Device *_dev, SignalChain *chain) {
	device = _dev;

	if (track->type == SignalType::AUDIO) {
		if (device) {
			input_audio->set_device(device);
			chain->connect(input_audio, 0, peak_meter, 0);
		} else {
			chain->disconnect(input_audio, 0, peak_meter, 0);
		}
	} else if (track->type == SignalType::MIDI) {
		if (device) {
			input_midi->set_device(device);
			chain->connect(input_midi, 0, recorder, 0);
		} else {
			//input_midi->unconnect();
		}
	}
	peak_meter->reset_state();
}

