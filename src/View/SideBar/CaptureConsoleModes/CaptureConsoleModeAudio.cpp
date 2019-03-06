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
#include "../../../Module/Audio/AudioSucker.h"
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
	sucker = nullptr;
	chain = nullptr;

	cc->event("source", [&]{ on_source(); });
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
	// FIXME ...
	//view->setCurTrack(target);

	bool ok = (target->type == SignalType::AUDIO);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::AUDIO).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enter_parent()
{
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

	input = new InputStreamAudio(session);
	input->set_chunk_size(4096);
	chain->_add(input);

	//input->set_update_dt(0.03f); // FIXME: SignalChain ticks...
	peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
	chain->_add(peak_meter);
	chain->connect(input, 0, peak_meter, 0);
	cc->peak_meter->set_source(peak_meter);

	auto *backup = new AudioBackup(session);
	backup->set_backup_mode(BACKUP_MODE_TEMP);
	chain->_add(backup);
	chain->connect(peak_meter, 0, backup, 0);

	input->set_device(chosen_device);

	//enable("capture_audio_source", false);

	sucker = CreateAudioSucker(session);
	chain->_add(sucker);
	chain->connect(backup, 0, sucker, 0);
	//chain->connect(peak_meter, 0, sucker, 0);

	chain->start();
	view->mode_capture->set_data({CaptureTrackData(target,input,sucker)});
}

void CaptureConsoleModeAudio::leave()
{
	view->mode_capture->set_data({});
	chain->stop();
	cc->peak_meter->set_source(nullptr);
	delete chain;
	chain = nullptr;
}

void CaptureConsoleModeAudio::pause()
{
	sucker->accumulate(false);
}

void CaptureConsoleModeAudio::start()
{
	input->reset_sync();
	sucker->accumulate(true);
	cc->enable("source", false);
}

void CaptureConsoleModeAudio::stop()
{
	chain->stop();
}

void CaptureConsoleModeAudio::dump()
{
	sucker->accumulate(false);
	sucker->reset_accumulation();
	cc->enable("source", true);
}

bool CaptureConsoleModeAudio::insert()
{
	// insert recorded data with some delay
	int dpos = input->get_delay();

	bool ok = cc->insert_audio(target, sucker->buf, dpos);
	sucker->reset_accumulation();
	return ok;

}

int CaptureConsoleModeAudio::get_sample_count()
{
	return sucker->buf.length;
}

bool CaptureConsoleModeAudio::is_capturing()
{
	return input->is_capturing();
}
