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
#include "../../../Module/Audio/AudioChannelSelector.h"
#include "../../../Module/Audio/PeakMeter.h"
//#include "../../../Module/Midi/MidiSucker.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/ModuleFactory.h"
#include "../../../Module/SignalChain.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioInput.h"
#include "../../../Device/Stream/MidiInput.h"

Array<int> create_default_channel_map(int n_in, int n_out);

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
		c.chain = chain.get();

		if (t->type == SignalType::AUDIO) {
			c.input_audio = (AudioInput*)chain->add(ModuleCategory::STREAM, "AudioInput");
			//c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			c.channel_selector = (AudioChannelSelector*)chain->add(ModuleCategory::PLUMBING, "AudioChannelSelector");
			c.channel_selector->subscribe(this, [&] {
				c.peak_meter_display->set_channel_map(c.channel_map());
			});
			c.peak_meter = c.channel_selector->peak_meter.get();
			c.accumulator = chain->add(ModuleCategory::PLUMBING, "AudioAccumulator");
			c.accumulator->command(ModuleCommand::SET_INPUT_CHANNELS, t->channels);
			auto *sucker = (AudioSucker*)chain->add(ModuleCategory::PLUMBING, "AudioSucker");
			sucker->set_channels(t->channels);
			chain->connect(c.input_audio, 0, c.channel_selector, 0);
			chain->connect(c.channel_selector, 0, c.accumulator, 0);
			chain->connect(c.accumulator, 0, sucker, 0);
			if (c.device) {
				c.input_audio->set_device(c.device);
				c.set_map(create_default_channel_map(c.device->channels, c.track->channels));
			}
			data.add({c.track, c.input_audio, c.accumulator});

		} else if (t->type == SignalType::MIDI) {
			c.input_midi = (MidiInput*)chain->add(ModuleCategory::STREAM, "MidiInput");
			c.accumulator = chain->add(ModuleCategory::PLUMBING, "MidiAccumulator");
			auto *synth = chain->_add(t->synth->copy());
			c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
			//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
			auto *out = chain->add(ModuleCategory::STREAM, "AudioOutput");
			if (c.device) {
				c.input_midi->set_device(c.device);
			}
			chain->connect(c.input_midi, 0, c.accumulator, 0);
			chain->connect(c.accumulator, 0, synth, 0);
			chain->connect(synth, 0, c.peak_meter, 0);
			chain->connect(c.peak_meter, 0, out, 0);
			data.add({c.track, c.input_midi, c.accumulator});
		}
		c.peak_meter_display->set_source(c.peak_meter);
		if (c.id_source.num > 0)
			c.panel->enable(c.id_source, c.enabled and c.allowing_edit);
		if (c.id_mapper.num > 0)
			c.panel->enable(c.id_mapper, c.enabled and c.allowing_edit);
		if (c.enabled)
			c.enable(true);
	}
	chain->mark_all_modules_as_system();

	view->mode_capture->set_data(data);
}



void CaptureConsoleMode::CaptureTrackItem::accumulate(bool acc) {
	if (acc and enabled)
		accumulator->command(ModuleCommand::ACCUMULATION_START, 0);
	else
		accumulator->command(ModuleCommand::ACCUMULATION_STOP, 0);
}

void CaptureConsoleMode::CaptureTrackItem::enable(bool _enabled) {
	if (_enabled == enabled)
		return;
	enabled = _enabled;
/*
	if (track->type == SignalType::AUDIO) {
		if (enabled) {
			chain->connect(input_audio, 0, channel_selector, 0);
		} else {
			chain->disconnect(input_audio, 0, channel_selector, 0);
		}
	} else if (track->type == SignalType::MIDI) {
		if (device) {
			chain->connect(input_midi, 0, accumulator, 0);
		} else {
			//input_midi->unconnect();
		}
	}*/

	if (panel) {
		panel->check(id_active, enabled);
		if (id_source.num > 0)
			panel->enable(id_source, enabled and allowing_edit);
		if (id_mapper.num > 0)
			panel->enable(id_mapper, enabled and allowing_edit);
	}
	peak_meter_display->set_visible(enabled);
}

void CaptureConsoleMode::CaptureTrackItem::allow_edit(bool _allow) {
	allowing_edit = _allow;
	if (id_active.num > 0)
		panel->enable(id_active, allowing_edit);
	if (id_source.num > 0)
		panel->enable(id_source, allowing_edit and enabled);
	if (id_mapper.num > 0)
		panel->enable(id_mapper, allowing_edit and enabled);
}

Array<int> CaptureConsoleMode::CaptureTrackItem::channel_map() {
	return channel_selector->config.map;
}

void CaptureConsoleMode::CaptureTrackItem::set_map(const Array<int> &_map) {
	channel_selector->set_channel_map(device->channels, _map);
}

void CaptureConsoleMode::CaptureTrackItem::set_device(Device *_dev) {
	device = _dev;
	//if (!device)
	//	enable(false);

	if (device) {
		if (track->type == SignalType::AUDIO) {
			input_audio->set_device(device);
			set_map(create_default_channel_map(device->channels, track->channels));
		} else if (track->type == SignalType::MIDI) {
			input_midi->set_device(device);
		}
	}

	if (device)
		enable(true);

	peak_meter->reset_state();
}

void CaptureConsoleMode::accumulation_start() {
	for (auto &c: items)
		c.accumulate(true);
	//chain->command(ModuleCommand::ACCUMULATION_START, 0);
}

void CaptureConsoleMode::accumulation_stop() {
	for (auto &c: items)
		c.accumulate(false);
	//chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
}

void CaptureConsoleMode::accumulation_clear() {
	accumulation_stop();
	chain->command(ModuleCommand::ACCUMULATION_CLEAR, 0);
}
