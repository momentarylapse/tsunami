/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
#include "../CaptureConsole.h"
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
#include "../../../Module/Audio/AudioRecorder.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../../Module/Audio/AudioBackup.h"
#include "../../../Module/SignalChain.h"
#include "../../../Device/Device.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Stream/AudioInput.h"
#include "../../../Stream/AudioOutput.h"



CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc) {
	input = nullptr;
	peak_meter = nullptr;
	target = nullptr;
	chain = nullptr;

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeAudio::on_source() {
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num))
		input->set_device(sources[n]);
}

void CaptureConsoleModeAudio::set_target(Track *t) {
	target = t;

	bool ok = (target->type == SignalType::AUDIO);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::AUDIO).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enter() {
	cc->hide_control("single_grid", false);


	for (Track *t: view->song->tracks)
		if (view->sel.has(t) and (t->type == SignalType::AUDIO))
			set_target(t);

	chain = session->add_signal_chain_system("capture");

	input = (AudioInput*)chain->add(ModuleType::STREAM, "AudioInput");
	input->set_chunk_size(4096);
	input->subscribe(this, [=]{ update_device_list(); });

	//input->set_update_dt(0.03f); // FIXME: SignalChain ticks...
	peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");

	auto *backup = (AudioBackup*)chain->add(ModuleType::PLUMBING, "AudioBackup");
	backup->set_backup_mode(BACKUP_MODE_TEMP);

	auto *recorder = chain->add(ModuleType::PLUMBING, "AudioRecorder");
	auto *sucker = chain->add(ModuleType::PLUMBING, "AudioSucker");
	chain->mark_all_modules_as_system();

	chain->connect(input, 0, peak_meter, 0);
	chain->connect(peak_meter, 0, backup, 0);
	chain->connect(backup, 0, recorder, 0);
	chain->connect(recorder, 0, sucker, 0);


	update_device_list();

	cc->peak_meter->set_source(peak_meter);

	chain->start(); // for preview
	view->mode_capture->set_data({CaptureTrackData(target, recorder)});

	session->device_manager->subscribe(this, [=]{ update_device_list(); });
}

void CaptureConsoleModeAudio::update_device_list() {
	sources = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == input->get_device())
			cc->set_int("source", i);
}

void CaptureConsoleModeAudio::allow_change_device(bool allow) {
	cc->enable("source", allow);
}

void CaptureConsoleModeAudio::leave() {
	session->device_manager->unsubscribe(this);
	chain->stop();
	cc->peak_meter->set_source(nullptr);
	delete chain;
	chain = nullptr;
}
