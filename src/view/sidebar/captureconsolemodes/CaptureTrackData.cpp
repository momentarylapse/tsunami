/*
 * CaptureTrackData.cpp
 *
 *  Created on: Mar 29, 2021
 *      Author: michi
 */

#include "CaptureTrackData.h"
#include "../../helper/PeakMeterDisplay.h"
#include "../../helper/DeviceSelector.h"
#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../data/TrackLayer.h"
#include "../../../module/SignalChain.h"
#include "../../../stuff/BackupManager.h"
#include "../../../Session.h"
#include "../../../device/Device.h"
#include "../../../device/DeviceManager.h"
#include "../../../module/stream/AudioInput.h"
#include "../../../module/stream/MidiInput.h"
#include "../../../module/stream/AudioOutput.h"
#include "../../../module/audio/AudioAccumulator.h"
#include "../../../module/audio/AudioBackup.h"
#include "../../../module/audio/AudioChannelSelector.h"
#include "../../../module/audio/AudioSucker.h"
#include "../../../module/midi/MidiAccumulator.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include "../../../lib/hui/hui.h"

namespace tsunami {

Array<int> create_default_channel_map(int n_in, int n_out) {
	Array<int> map;
	for (int o=0; o<n_out; o++)
		map.add(min(o, n_in-1));
	return map;
}


// TODO: subscribe to input and catch device change -> update channel map...

CaptureTrackData::CaptureTrackData(hui::Panel* _panel, const string& _id_source, const string& _id_peaks) {
	panel = _panel;
	id_source = _id_source;
	id_peaks = _id_peaks;
}

CaptureTrackData::~CaptureTrackData() {
	for (int id: event_ids)
		panel->remove_event_handler(id);
	event_ids.clear();
	if (peak_meter_display)
		peak_meter_display->set_source(nullptr);
}

void CaptureTrackData::attach_to_gui(SignalType type, Session* session) {
	device_selector = new DeviceSelector(panel, id_source, session, type == SignalType::Midi ? DeviceType::MidiInput : DeviceType::AudioInput);
	device_selector->out_value >> create_data_sink<Device*>([this] (Device* d) {
		set_device(d);
	});
	panel->set_tooltip(id_source, _("Source device"));
	peak_meter_display = new PeakMeterDisplay(panel, id_peaks, nullptr);
}



SignalType CaptureTrackData::type() {
	return track->type;
}

AudioInput *CaptureTrackData::audio_input() {
	return (AudioInput*)input;
}

MidiInput *CaptureTrackData::midi_input() {
	return (MidiInput*)input;
}

AudioAccumulator *CaptureTrackData::audio_recorder() {
	return (AudioAccumulator*)accumulator;
}

MidiAccumulator *CaptureTrackData::midi_recorder() {
	return (MidiAccumulator*)accumulator;
}




void CaptureTrackData::start_sync_before(AudioOutput *out) {
	sync_points.clear();
	samples_played_before_capture = out->estimate_samples_played();
	samples_skiped_before_capture = audio_recorder()->samples_skipped;
}

void CaptureTrackData::sync(AudioOutput *out) {
	if (type() == SignalType::Audio) {
		auto sr = out->estimate_samples_played();
		auto sp = audio_input()->samples_recorded();
		if (sr.has_value() and sp.has_value()) {
			SyncPoint p;
			p.pos_play = *sp;
			p.pos_record = *sr;
			p.samples_skipped_start = audio_recorder()->samples_skipped;
			sync_points.add(p);
		}
	}
}

int SyncPoint::delay(int64 samples_played_before_capture) {
	return (pos_play - samples_played_before_capture) - (pos_record - samples_skipped_start);
}

int CaptureTrackData::get_sync_delay() {
	if (sync_points.num == 0)
		return 0;
	if (!samples_played_before_capture.has_value())
		return 0;
	int d = 0;
	for (auto &p: sync_points)
		d += p.delay(*samples_played_before_capture);
	d /= sync_points.num;
	/*if (fabs(d) > 50000)
		return 0;*/
	return d;
}



void CaptureTrackData::accumulate(bool acc) {
	if (acc and enabled) {
		accumulator->command(ModuleCommand::AccumulationStart, 0);
		if (backup)
			backup->command(ModuleCommand::AccumulationStart, 0);
	} else {
		accumulator->command(ModuleCommand::AccumulationStop, 0);
		if (backup)
			backup->command(ModuleCommand::AccumulationStop, 0);
	}
}

void CaptureTrackData::enable(bool _enabled) {
	if (_enabled == enabled)
		return;
	enabled = _enabled;

	if (panel) {
		if (id_active.num > 0)
			panel->check(id_active, enabled);
		if (id_source.num > 0)
			panel->enable(id_source, enabled and allowing_edit);
		if (id_mapper.num > 0)
			panel->enable(id_mapper, enabled and allowing_edit);
	}
	peak_meter_display->set_visible(enabled);
}

void CaptureTrackData::allow_edit(bool _allow) {
	allowing_edit = _allow;
	if (id_active.num > 0)
		panel->enable(id_active, allowing_edit);
	if (id_source.num > 0)
		panel->enable(id_source, allowing_edit and enabled);
	if (id_mapper.num > 0)
		panel->enable(id_mapper, allowing_edit and enabled);
}

Array<int> CaptureTrackData::channel_map() {
	return channel_selector->config.map;
}

Device *CaptureTrackData::get_device() {
	if (type() == SignalType::Audio)
		return audio_input()->get_device();
	if (type() == SignalType::Midi)
		return midi_input()->get_device();
	return nullptr;
	//return device_selector->get();
}

void CaptureTrackData::set_channel_map(const Array<int> &_map) {
	int channels = 2;
	auto dev = get_device();
	if (dev)
		channels = dev->channels;
	channel_selector->set_channel_map(channels, _map);
}

void CaptureTrackData::set_device(Device *device) {
	if (type() == SignalType::Audio) {
		audio_input()->set_device(device);
		set_channel_map(create_default_channel_map(device->channels, track->channels));
	} else if (type() == SignalType::Midi) {
		midi_input()->set_device(device);
	}

	enable(true);

	peak_meter->reset_state();
}

void CaptureTrackData::add_into_signal_chain(SignalChain *_chain, Device *preferred_device) {
	chain = _chain;
	auto t = track;
	auto device = preferred_device;


	if (type() == SignalType::Audio) {
		if (!device)
			device = chain->session->device_manager->choose_device(DeviceType::AudioInput);

		// create modules
		input = chain->addx<AudioOutput>(ModuleCategory::Stream, "AudioInput").get();
		//c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
		channel_selector = chain->addx<AudioChannelSelector>(ModuleCategory::Plumbing, "AudioChannelSelector").get();
		peak_meter = channel_selector->peak_meter.get();
		accumulator = chain->add(ModuleCategory::Plumbing, "AudioAccumulator").get();
		backup = chain->add(ModuleCategory::Plumbing, "AudioBackup").get();
		auto sucker = chain->addx<AudioSucker>(ModuleCategory::Plumbing, "AudioSucker").get();

		// configure
		audio_input()->set_device(device);
		channel_selector->out_changed >> create_sink([&] {
			peak_meter_display->set_channel_map(channel_map());
		});
		channel_selector->out_state_changed >> create_sink([&] {
			peak_meter_display->set_channel_map(channel_map());
		});
		accumulator->command(ModuleCommand::SetInputChannels, t->channels);
		backup->command(ModuleCommand::SetInputChannels, track->channels);
		backup->command(ModuleCommand::AccumulationStop, 0);
		reinterpret_cast<AudioBackup*>(backup)->set_backup_mode(BackupMode::Temporary);
		sucker->set_channels(t->channels);
		set_channel_map(create_default_channel_map(device->channels, track->channels));

		// link
		chain->connect(input, 0, channel_selector, 0);
		chain->connect(channel_selector, 0, accumulator, 0);
		chain->connect(accumulator, 0, backup, 0);
		chain->connect(backup, 0, sucker, 0);

	} else if (t->type == SignalType::Midi) {
		if (!device)
			device = chain->session->device_manager->choose_device(DeviceType::MidiInput);

		// create modules
		input = (MidiInput*)chain->addx<MidiInput>(ModuleCategory::Stream, "MidiInput").get();
		accumulator = chain->add(ModuleCategory::Plumbing, "MidiAccumulator").get();
		//backup = chain->add(ModuleCategory::PLUMBING, "MidiBackup");
		synth = (Synthesizer*)chain->_add(t->synth->copy()).get();
		peak_meter = chain->addx<PeakMeter>(ModuleCategory::AudioVisualizer, "PeakMeter").get();
		//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
		auto *out = chain->add(ModuleCategory::Stream, "AudioOutput").get();

		// configure
		midi_input()->set_device(device);

		// link
		chain->connect(input, 0, accumulator, 0);
		chain->connect(accumulator, 0, synth, 0);
		chain->connect(synth, 0, peak_meter, 0);
		chain->connect(peak_meter, 0, out, 0);
	}
	peak_meter_display->set_source(peak_meter);
	if (id_source.num > 0)
		panel->enable(id_source, enabled and allowing_edit);
	if (id_mapper.num > 0)
		panel->enable(id_mapper, enabled and allowing_edit);

	if (enabled)
		enable(true);
}








// TODO move outside!

void CaptureTrackData::insert_midi(int s_start, int delay) {

	int i0 = s_start + delay;

	// insert data
	track->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi_recorder()->buffer));
}


bool layer_available(TrackLayer *l, const Range &r) {
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

TrackLayer *find_or_create_available_layer(Track *target, const Range &r) {
	for (auto *l: weak(target->layers))
		if (layer_available(l, r))
			return l;
	return target->add_layer();
}

void CaptureTrackData::insert_audio(int s_start, int delay) {
	Song *song = track->song;

	int i0 = s_start + delay;

	AudioBuffer &buf = audio_recorder()->buffer;

	// insert data
	Range r = Range(i0, buf.length);
	song->begin_action_group("insert capture audio");

	TrackLayer *layer = find_or_create_available_layer(track, r);

	AudioBuffer tbuf;
	auto *a = layer->edit_buffers(tbuf, r);

	/*if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(buf, 0, 1.0f, 0);
	else*/
		tbuf.set(buf, 0, 1.0f);
	layer->edit_buffers_finish(a);
	song->end_action_group();
}

void CaptureTrackData::insert(int pos) {
	int delay = get_sync_delay();
	track->song->session->debug("input", format("latency: %d samples", delay));
	if (type() == SignalType::Audio) {
		insert_audio(pos, delay);
	} else if (type() == SignalType::Midi) {
		insert_midi(pos, delay);
	}
}

}
