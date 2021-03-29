/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
#include "CaptureTrackData.h"
#include "../CaptureConsole.h"
#include "../../Dialog/ChannelMapperDialog.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/base.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"
#include "../../../Stuff/BackupManager.h"
#include "../../../Action/ActionManager.h"
#include "../../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../../Module/Audio/AudioBackup.h"
#include "../../../Module/Audio/AudioChannelSelector.h"
#include "../../../Module/SignalChain.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Stream/AudioInput.h"
#include "../../../Device/Stream/AudioOutput.h"
#include "../../../Module/Audio/AudioAccumulator.h"


Array<int> create_default_channel_map(int n_in, int n_out);

CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
	CaptureTrackData a;
	a.panel = cc;
	a.id_source = "source";
	a.id_mapper = "channel-mapper";
	a.id_peaks = "level";
	a.peak_meter_display = cc->peak_meter_display.get();
	items.add(a);

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeAudio::on_source() {
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num))
		items[0].set_device(sources[n]);
}

void CaptureConsoleModeAudio::set_target(Track *t) {
	items[0].track = t;

	bool ok = (items[0].track->type == SignalType::AUDIO);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::AUDIO).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enter() {
	cc->hide_control("single_grid", false);


	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t) and (t->type == SignalType::AUDIO))
			set_target(t);

	chain = session->create_signal_chain_system("capture");

	auto &c = items[0];
	c.add_into_signal_chain(chain.get());


	update_data_from_items();

	update_device_list();
	c.input->subscribe(this, [=]{ update_device_list(); });
	cc->set_options("level", format("height=%d", PeakMeterDisplay::good_size(c.track->channels)));

	c.channel_selector->subscribe(this, [&] {
		cc->peak_meter_display->set_channel_map(c.channel_map());
	});

	session->device_manager->subscribe(this, [=]{ update_device_list(); });


	cc->event(c.id_mapper, [&] {
		//ModuleExternalDialog(c.channel_selector, cc);
		auto dlg = ownify(new ChannelMapDialog(cc, c.channel_selector));
		dlg->run();
	});


	c.enable(true);

	chain->start(); // for preview
}

void CaptureConsoleModeAudio::update_device_list() {
	sources = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == items[0].get_device())
			cc->set_int("source", i);
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	items[0].allow_edit(allow);
	//cc->enable("source", allow);
	//cc->enable("channel-mapper", allow);
}

void CaptureConsoleModeAudio::leave() {
	session->device_manager->unsubscribe(this);
	chain->stop();
	cc->peak_meter_display->set_source(nullptr);
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}
