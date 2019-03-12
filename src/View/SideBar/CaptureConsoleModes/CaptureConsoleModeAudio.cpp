/*
 * CaptureConsoleModeAudio.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeAudio.h"
#include "../CaptureConsole.h"
#include "../../../Device/InputStreamAudio.h"
#include "../../../Device/OutputStream.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Device.h"
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



CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = nullptr;
	input = nullptr;
	peak_meter = nullptr;
	target = nullptr;
	recorder = nullptr;
	chain = nullptr;

	cc->event("source", [=]{ on_source(); });
}

void CaptureConsoleModeAudio::on_source()
{
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}

void CaptureConsoleModeAudio::set_target(Track *t)
{
	target = t;

	bool ok = (target->type == SignalType::AUDIO);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::AUDIO).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enter()
{
	chosen_device = session->device_manager->choose_device(DeviceType::AUDIO_INPUT);
	sources = session->device_manager->good_device_list(DeviceType::AUDIO_INPUT);
	cc->hide_control("single_grid", false);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->set_int("source", i);


	for (const Track *t: view->sel.tracks)
		if (t->type == SignalType::AUDIO)
			set_target((Track*)t);

	chain = new SignalChain(session, "capture");

	input = (InputStreamAudio*)chain->add(ModuleType::STREAM, "AudioInput");
	input->set_chunk_size(4096);

	//input->set_update_dt(0.03f); // FIXME: SignalChain ticks...
	peak_meter = (PeakMeter*)chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	chain->connect(input, 0, peak_meter, 0);
	cc->peak_meter->set_source(peak_meter);

	auto *backup = (AudioBackup*)chain->add(ModuleType::PLUMBING, "AudioBackup");
	backup->set_backup_mode(BACKUP_MODE_TEMP);
	chain->connect(peak_meter, 0, backup, 0);

	recorder = (AudioRecorder*)chain->add(ModuleType::PLUMBING, "AudioRecorder");
	chain->connect(backup, 0, recorder, 0);

	input->set_device(chosen_device);

	//enable("capture_audio_source", false);

	auto *sucker = chain->add(ModuleType::PLUMBING, "AudioSucker");
	chain->connect(recorder, 0, sucker, 0);
	//chain->connect(peak_meter, 0, sucker, 0);

	chain->start(); // for preview
	view->mode_capture->set_data({CaptureTrackData(target, input, recorder)});
}

void CaptureConsoleModeAudio::allow_change_device(bool allow)
{
	cc->enable("source", allow);
}

void CaptureConsoleModeAudio::leave()
{
	chain->stop();
	cc->peak_meter->set_source(nullptr);
	delete chain;
	chain = nullptr;
}
