/*
 * CaptureTrackData.cpp
 *
 *  Created on: Mar 29, 2021
 *      Author: michi
 */

#include "CaptureTrackData.h"
#include "../../helper/PeakMeterDisplay.h"
#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../data/TrackLayer.h"
#include "../../../module/SignalChain.h"
#include "../../../stuff/BackupManager.h"
#include "../../../Session.h"
#include "../../../device/Device.h"
#include "../../../device/DeviceManager.h"
#include "../../../device/stream/AudioInput.h"
#include "../../../device/stream/MidiInput.h"
#include "../../../device/stream/AudioOutput.h"
#include "../../../module/audio/AudioAccumulator.h"
#include "../../../module/audio/AudioBackup.h"
#include "../../../module/audio/AudioChannelSelector.h"
#include "../../../module/audio/AudioSucker.h"
#include "../../../module/midi/MidiAccumulator.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include "../../../lib/hui/hui.h"


Array<int> create_default_channel_map(int n_in, int n_out) {
	Array<int> map;
	for (int o=0; o<n_out; o++)
		map.add(min(o, n_in-1));
	return map;
}


// TODO: subscribe to input and catch device change -> update channel map...



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
	if (type() == SignalType::AUDIO) {
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
		accumulator->command(ModuleCommand::ACCUMULATION_START, 0);
		if (backup)
			backup->command(ModuleCommand::ACCUMULATION_START, 0);
	} else {
		accumulator->command(ModuleCommand::ACCUMULATION_STOP, 0);
		if (backup)
			backup->command(ModuleCommand::ACCUMULATION_STOP, 0);
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
	if (type() == SignalType::AUDIO)
		return audio_input()->get_device();
	if (type() == SignalType::MIDI)
		return midi_input()->get_device();
	return nullptr;
}

void CaptureTrackData::set_map(const Array<int> &_map) {
	int channels = 2;
	auto dev = get_device();
	if (dev)
		channels = dev->channels;
	channel_selector->set_channel_map(channels, _map);
}

void CaptureTrackData::set_device(Device *device) {

	if (type() == SignalType::AUDIO) {
		audio_input()->set_device(device);
		set_map(create_default_channel_map(device->channels, track->channels));
	} else if (type() == SignalType::MIDI) {
		midi_input()->set_device(device);
	}

	enable(true);

	peak_meter->reset_state();
}

void CaptureTrackData::add_into_signal_chain(SignalChain *_chain, Device *preferred_device) {
	chain = _chain;
	auto t = track;
	auto device = preferred_device;


	if (type() == SignalType::AUDIO) {
		if (!device)
			device = chain->session->device_manager->choose_device(DeviceType::AUDIO_INPUT);

		// create modules
		input = chain->addx<AudioOutput>(ModuleCategory::STREAM, "AudioInput").get();
		//c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
		channel_selector = chain->addx<AudioChannelSelector>(ModuleCategory::PLUMBING, "AudioChannelSelector").get();
		peak_meter = channel_selector->peak_meter.get();
		accumulator = chain->add(ModuleCategory::PLUMBING, "AudioAccumulator").get();
		backup = chain->add(ModuleCategory::PLUMBING, "AudioBackup").get();
		auto sucker = chain->addx<AudioSucker>(ModuleCategory::PLUMBING, "AudioSucker").get();

		// configure
		audio_input()->set_device(device);
		channel_selector->out_changed >> create_sink([&] {
			peak_meter_display->set_channel_map(channel_map());
		});
		channel_selector->out_state_changed >> create_sink([&] {
			peak_meter_display->set_channel_map(channel_map());
		});
		accumulator->command(ModuleCommand::SET_INPUT_CHANNELS, t->channels);
		backup->command(ModuleCommand::SET_INPUT_CHANNELS, track->channels);
		backup->command(ModuleCommand::ACCUMULATION_STOP, 0);
		((AudioBackup*)backup)->set_backup_mode(BackupMode::TEMP);
		sucker->set_channels(t->channels);
		set_map(create_default_channel_map(device->channels, track->channels));

		// link
		chain->connect(input, 0, channel_selector, 0);
		chain->connect(channel_selector, 0, accumulator, 0);
		chain->connect(accumulator, 0, backup, 0);
		chain->connect(backup, 0, sucker, 0);

	} else if (t->type == SignalType::MIDI) {
		if (!device)
			device = chain->session->device_manager->choose_device(DeviceType::MIDI_INPUT);

		// create modules
		input = (MidiInput*)chain->addx<MidiInput>(ModuleCategory::STREAM, "MidiInput").get();
		accumulator = chain->add(ModuleCategory::PLUMBING, "MidiAccumulator").get();
		//backup = chain->add(ModuleCategory::PLUMBING, "MidiBackup");
		synth = (Synthesizer*)chain->_add(t->synth->copy()).get();
		peak_meter = chain->addx<PeakMeter>(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter").get();
		//auto *sucker = chain->add(ModuleType::PLUMBING, "MidiSucker");
		auto *out = chain->add(ModuleCategory::STREAM, "AudioOutput").get();

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

	AudioBuffer &buf = audio_recorder()->buf;

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
	if (type() == SignalType::AUDIO) {
		insert_audio(pos, delay);
	} else if (type() == SignalType::MIDI) {
		insert_midi(pos, delay);
	}
}
