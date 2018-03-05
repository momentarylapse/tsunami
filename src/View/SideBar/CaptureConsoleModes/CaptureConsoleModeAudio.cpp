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
#include "../../../Audio/AudioSucker.h"
#include "../../../Data/Song.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Stuff/Log.h"
#include "../../../Stuff/BackupManager.h"
#include "../../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../../Tsunami.h"



extern AudioSucker *export_view_sucker;

CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = cc->device_manager->chooseDevice(Device::TYPE_AUDIO_INPUT);
	input = NULL;
	target = NULL;
	sucker = NULL;

	cc->enable("capture_audio_target", false);

	cc->event("capture_audio_source", std::bind(&CaptureConsoleModeAudio::onSource, this));
	//cc->event("capture_audio_target", std::bind(&CaptureConsoleModeAudio::onTarget, this));
}

void CaptureConsoleModeAudio::onSource()
{
	int n = cc->getInt("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->setDevice(chosen_device);
	}
}

void CaptureConsoleModeAudio::onTarget()
{
	/*int index = cc->getInt("capture_audio_target");
	if (index >= 0)
		setTarget(cc->song->tracks[index]);*/
}

void CaptureConsoleModeAudio::setTarget(Track *t)
{
	target = t;
	view->setCurTrack(target);
	view->mode_capture->capturing_track = target;
	cc->setInt("capture_audio_target", target->get_index());

	bool ok = (target->type == Track::TYPE_AUDIO);
	cc->setString("capture_audio_message", "");
	if (!ok)
		cc->setString("capture_audio_message", format(_("Please select a track of type %s."), track_type(Track::TYPE_AUDIO).c_str()));
	cc->enable("capture_start", ok);
}

void CaptureConsoleModeAudio::enterParent()
{
}

void CaptureConsoleModeAudio::enter()
{
	sources = cc->device_manager->getGoodDeviceList(Device::TYPE_AUDIO_INPUT);
	cc->hideControl("capture_audio_grid", false);

	// add all
	cc->reset("capture_audio_source");
	for (Device *d: sources)
		cc->setString("capture_audio_source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->setInt("capture_audio_source", i);


	// target list
	cc->reset("capture_audio_target");
	for (Track *t: song->tracks)
		cc->addString("capture_audio_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
	//cc->addString("capture_audio_target", _("  - create new track -"));

	for (const Track *t: view->sel.tracks)
		if (t->type == t->TYPE_AUDIO)
			setTarget((Track*)t);


	input = new InputStreamAudio(song->sample_rate);
	input->setBackupMode(BACKUP_MODE_TEMP, view->win);
	input->setChunkSize(4096);
	input->setUpdateDt(0.03f);
	view->mode_capture->setInputAudio(input);
	cc->peak_meter->setSource(input);

	input->setDevice(chosen_device);

	//enable("capture_audio_source", false);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}

	sucker = new AudioSucker(input->source);
	sucker->start();
	export_view_sucker = sucker;
}

void CaptureConsoleModeAudio::leave()
{
	delete sucker;
	cc->peak_meter->setSource(NULL);
	view->mode_capture->setInputAudio(NULL);
	delete(input);
	input = NULL;
}

void CaptureConsoleModeAudio::pause()
{
	sucker->accumulate(false);
}

void CaptureConsoleModeAudio::start()
{
	input->resetSync();
	sucker->accumulate(true);
	cc->enable("capture_audio_source", false);
	//cc->enable("capture_audio_target", false);
}

void CaptureConsoleModeAudio::stop()
{
	input->stop();
}

void CaptureConsoleModeAudio::dump()
{
	sucker->accumulate(false);
	sucker->resetAccumulation();
	cc->enable("capture_audio_source", true);
	//cc->enable("capture_audio_target", true);
}

bool CaptureConsoleModeAudio::insert()
{
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->getDelay();

	// overwrite
	int i0 = s_start + dpos;

	if (target->type != Track::TYPE_AUDIO){
		tsunami->log->error(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::TYPE_AUDIO).c_str(), track_type(target->type).c_str()));
		return false;
	}

	// insert data
	Range r = Range(i0, sucker->buf.length);
	cc->song->action_manager->beginActionGroup();
	AudioBuffer tbuf = target->getBuffers(view->cur_layer, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(target, view->cur_layer, r);

	if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(sucker->buf, 0, 1.0f, 0);
	else
		tbuf.set(sucker->buf, 0, 1.0f);
	song->execute(a);
	song->action_manager->endActionGroup();

	sucker->resetAccumulation();
	return true;
}

int CaptureConsoleModeAudio::getSampleCount()
{
	return sucker->buf.length;
}

bool CaptureConsoleModeAudio::isCapturing()
{
	return input->isCapturing();
}
