/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
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

//#include "../../../Module/Audio/SongRenderer.h"


Array<int> create_default_channel_map(int n_in, int n_out);

CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
	CaptureItem a;
	items.add(a);

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeAudio::on_source() {
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num))
		items[0].set_device(sources[n], chain.get());
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
	int channels = c.track->channels;

	c.input_audio = (AudioInput*)chain->add(ModuleCategory::STREAM, "AudioInput");
	c.input_audio->subscribe(this, [=]{ update_device_list(); });
	c.device = c.input_audio->get_device();
	c.channel_map = create_default_channel_map(c.device->channels, channels);

	c.channel_selector = (AudioChannelSelector*)chain->add(ModuleCategory::PLUMBING, "AudioChannelSelector");
	c.channel_selector->set_channel_map(c.device->channels, c.channel_map);
	c.peak_meter = c.channel_selector->peak_meter.get();
//	c.peak_meter = (PeakMeter*)chain->add(ModuleCategory::AUDIO_VISUALIZER, "PeakMeter");
	auto *backup = (AudioBackup*)chain->add(ModuleCategory::PLUMBING, "AudioBackup");
	backup->command(ModuleCommand::SET_INPUT_CHANNELS, channels);
	backup->set_backup_mode(BackupMode::TEMP);
	c.accumulator = chain->add(ModuleCategory::PLUMBING, "AudioAccumulator");
	c.accumulator->command(ModuleCommand::SET_INPUT_CHANNELS, channels);
	auto *sucker = chain->add(ModuleCategory::PLUMBING, "AudioSucker");
	sucker->command(ModuleCommand::SET_INPUT_CHANNELS, channels);

	chain->mark_all_modules_as_system();

	chain->connect(c.input_audio, 0, c.channel_selector, 0);
	chain->connect(c.channel_selector, 0, backup, 0);
	chain->connect(backup, 0, c.accumulator, 0);
	chain->connect(c.accumulator, 0, sucker, 0);


	update_device_list();

	cc->peak_meter_display->set_channel_map(c.channel_map);
	cc->peak_meter_display->set_source(c.peak_meter);
	cc->set_options("level", format("height=%d", PeakMeterDisplay::good_size(channels)));

	chain->start(); // for preview
	view->mode_capture->set_data({{c.track, c.input_audio, c.accumulator}});

	session->device_manager->subscribe(this, [=]{ update_device_list(); });


	c.id_mapper = "channel-mapper";
	c.peak_meter_display = cc->peak_meter_display.get();
	cc->event(c.id_mapper, [&] {
		if (!c.device)
			return;
		auto dlg = new ChannelMapDialog(cc, c.channel_selector);
		dlg->run();
		delete dlg;
		c.set_map(c.channel_selector->config.map);
	});
}

void CaptureConsoleModeAudio::update_device_list() {
	sources = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == items[0].device)
			cc->set_int("source", i);
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	cc->enable("source", allow);
	cc->enable("channel-mapper", allow);
}

void CaptureConsoleModeAudio::leave() {
	session->device_manager->unsubscribe(this);
	chain->stop();
	cc->peak_meter_display->set_source(nullptr);
	session->remove_signal_chain(chain.get());
	chain = nullptr;
}
