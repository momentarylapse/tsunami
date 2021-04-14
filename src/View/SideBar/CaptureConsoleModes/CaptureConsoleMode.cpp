/*
 * CaptureConsoleMode.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleMode.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../../Module/SignalChain.h"
#include "../../../Module/Audio/AudioChannelSelector.h"
#include "../../Dialog/ChannelMapperDialog.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Session.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Device.h"


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

void CaptureConsoleMode::update_data_from_items() {

	chain = session->create_signal_chain_system("capture");

	for (auto &c: items)
		c.add_into_signal_chain(chain.get());

	chain->mark_all_modules_as_system();

	view->mode_capture->set_data(items);


	session->device_manager->subscribe(this, [=]{ update_device_list(); });


	for (auto &c: items) {
		cc->set_options(c.id_peaks, format("height=%d", PeakMeterDisplay::good_size(c.track->channels)));

		c.input->subscribe(this, [=] {
			update_device_list();
		});

		if (c.channel_selector) {
			c.channel_selector->subscribe(this, [&] {
				cc->peak_meter_display->set_channel_map(c.channel_map());
			});
		}

		if (c.id_mapper.num > 0 and c.channel_selector) {
			cc->event(c.id_mapper, [&] {
				//ModuleExternalDialog(c.channel_selector, cc);
				auto dlg = ownify(new ChannelMapDialog(cc, c.channel_selector));
				dlg->run();
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


void CaptureConsoleMode::update_device_list() {
	sources_audio = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);

	for (auto &c: items) {
		auto sources = sources_audio;
		if (c.track->type == SignalType::MIDI)
			sources = sources_midi;

		// add all
		c.panel->reset(c.id_source);
		for (Device *d: sources)
			c.panel->set_string(c.id_source, d->get_name());

		// select current
		foreachi(Device *d, sources, i)
			if (d == c.get_device())
				c.panel->set_int(c.id_source, i);
	}
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

void CaptureConsoleMode::leave() {
	chain->stop();
	session->device_manager->unsubscribe(this);

	for (int id: event_ids)
		cc->remove_event_handler(id);
	event_ids.clear();

	for (auto &c: items) {
		c.peak_meter_display->set_source(nullptr);
	}
	items.clear();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}
