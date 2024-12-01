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

namespace tsunami {

CaptureConsoleMode::CaptureConsoleMode(CaptureConsole *_console) {
	console = _console;
	song = console->song;
	view = console->view;
	session = console->session;
	chain = nullptr;
}

void CaptureConsoleMode::start_sync_before() {
	for (auto d: items())
		d->start_sync_before(session->playback->output_stream.get());
}

void CaptureConsoleMode::sync() {
	for (auto d: items())
		d->sync(session->playback->output_stream.get());
}


Array<CaptureTrackData*> CaptureConsoleMode::items() const {
	return weak(view->mode_capture->data);
}

void CaptureConsoleMode::add_item(xfer<CaptureTrackData> c) {
	view->mode_capture->data.add(c);
}

void CaptureConsoleMode::clear_items() {
	view->mode_capture->data.clear();
}


void CaptureConsoleMode::update_data_from_items() {

	chain = session->create_signal_chain_system("capture");

	for (auto c: items())
		c->add_into_signal_chain(chain.get());

	chain->mark_all_modules_as_system();


	for (auto c: items()) {
		console->set_options(c->id_peaks, format("height=%d", PeakMeterDisplay::good_size(c->track->channels)));

		// TODO!!!!
		/*c->input->out_changed >> create_sink([this] {
			update_device_list();
		});
		c->input->out_state_changed >> create_sink([this] {
			update_device_list();
		});*/

		if (c->channel_selector) {
			c->channel_selector->out_changed >> create_sink([c] {
				c->peak_meter_display->set_channel_map(c->channel_map());
			});
			c->channel_selector->out_state_changed >> create_sink([c] {
				c->peak_meter_display->set_channel_map(c->channel_map());
			});
		}

		if (c->id_mapper.num > 0 and c->channel_selector) {
			c->event_ids.add(console->event(c->id_mapper, [this, c] {
				hui::fly(new ChannelMapDialog(console, c->channel_selector));
			}));
		}
		if (c->id_active.num > 0)
			c->event_ids.add(console->event(c->id_active, [this, c] {
				c->enable(console->is_checked(""));
			}));
	}
}



void CaptureConsoleMode::accumulation_start() {
	for (auto c: items())
		c->accumulate(true);
	//chain->command(ModuleCommand::ACCUMULATION_START, 0);
}

void CaptureConsoleMode::accumulation_stop() {
	for (auto c: items())
		c->accumulate(false);
	//chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
}

void CaptureConsoleMode::accumulation_clear() {
	accumulation_stop();
	chain->command(ModuleCommand::AccumulationClear, 0);
}

void CaptureConsoleMode::leave() {
	chain->stop();
	session->device_manager->unsubscribe(this);

	clear_items();
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}

}
