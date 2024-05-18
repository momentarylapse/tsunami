/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../audioview/AudioView.h"
#include "../../mode/ViewModeCapture.h"
#include "../../dialog/ChannelMapperDialog.h"
#include "../../../module/SignalChain.h"
#include "../../../module/audio/AudioChannelSelector.h"
#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../device/DeviceManager.h"
#include "../../../device/Device.h"
#include "../../../Session.h"
#include "../../../Playback.h"


CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_cc) {
	cc = _cc;
	song = cc->song;
	view = cc->view;
	session = cc->session;
	chain = nullptr;
}

void CaptureConsoleMode::start_sync_before() {
	for (auto &d: view->mode_capture->data)
		d.start_sync_before(session->playback->output_stream.get());
}

void CaptureConsoleMode::sync() {
	for (auto &d: view->mode_capture->data)
		d.sync(session->playback->output_stream.get());
}


Array<CaptureTrackData> &CaptureConsoleMode::items() {
	return this->view->mode_capture->data;
}

void CaptureConsoleMode::update_data_from_items() {

	chain = session->create_signal_chain_system("capture");

	for (auto &c: items())
		c.add_into_signal_chain(chain.get());

	chain->mark_all_modules_as_system();


	session->device_manager->out_add_device >> create_data_sink<Device*>([this] (Device*) { update_device_list(); });
	session->device_manager->out_remove_device >> create_data_sink<Device*>([this] (Device*) { update_device_list(); });


	for (auto &c: items()) {
		cc->set_options(c.id_peaks, format("height=%d", PeakMeterDisplay::good_size(c.track->channels)));

		c.input->out_changed >> create_sink([this] {
			update_device_list();
		});
		c.input->out_state_changed >> create_sink([this] {
			update_device_list();
		});

		if (c.channel_selector) {
			c.channel_selector->out_changed >> create_sink([&] {
				cc->peak_meter_display->set_channel_map(c.channel_map());
			});
			c.channel_selector->out_state_changed >> create_sink([&] {
				cc->peak_meter_display->set_channel_map(c.channel_map());
			});
		}

		if (c.id_mapper.num > 0 and c.channel_selector) {
			cc->event(c.id_mapper, [&] {
				//ModuleExternalDialog(c.channel_selector, cc);
				hui::fly(new ChannelMapDialog(cc, c.channel_selector));
			});
		}
		if (c.id_active.num > 0)
			cc->event(c.id_active, [&] {
				c.enable(cc->is_checked(""));
			});
	}

	update_device_list();
}

Device* CaptureConsoleMode::get_source(SignalType type, int i) {
	if (i >= 0) {
		if (type == SignalType::AUDIO)
			return sources_audio[i];
		if (type == SignalType::MIDI)
			return sources_midi[i];
	}
	return nullptr;
}


string shorten(const string &s, int max_length);

void CaptureConsoleMode::update_device_list() {
	sources_audio = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	for (auto &c: items()) {
		auto sources = sources_audio;
		if (c.track->type == SignalType::MIDI)
			sources = sources_midi;

		// add all
		c.panel->reset(c.id_source);
		for (Device *d: sources)
			c.panel->set_string(c.id_source, shorten(d->get_name(), 42));

		// select current
		foreachi(Device *d, sources, i)
			if (d == c.get_device())
				c.panel->set_int(c.id_source, i);
	}
}



void CaptureConsoleMode::accumulation_start() {
	for (auto &c: items())
		c.accumulate(true);
	//chain->command(ModuleCommand::ACCUMULATION_START, 0);
}

void CaptureConsoleMode::accumulation_stop() {
	for (auto &c: items())
		c.accumulate(false);
	//chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
}

void CaptureConsoleMode::accumulation_clear() {
	accumulation_stop();
	chain->command(ModuleCommand::ACCUMULATION_CLEAR, 0);
}

void CaptureConsoleMode::leave() {
	chain->stop();
	session->device_manager->unsubscribe(this);

	for (int id: event_ids)
		cc->remove_event_handler(id);
	event_ids.clear();

	for (auto &c: items()) {
		if (c.peak_meter_display)
			c.peak_meter_display->set_source(nullptr);
	}
	items().clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}
