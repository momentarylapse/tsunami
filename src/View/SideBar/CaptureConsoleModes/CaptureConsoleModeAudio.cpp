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
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"
#include "../../../Stuff/BackupManager.h"
#include "../../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../../Module/Audio/AudioSucker.h"
#include "../../../Module/Audio/PeakMeter.h"



extern AudioSucker *export_view_sucker;

CaptureConsoleModeAudio::CaptureConsoleModeAudio(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = cc->device_manager->chooseDevice(Device::Type::AUDIO_INPUT);
	input = NULL;
	peak_meter = NULL;
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
		input->set_device(chosen_device);
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

	bool ok = (target->type == Track::Type::AUDIO);
	cc->setString("capture_audio_message", "");
	if (!ok)
		cc->setString("capture_audio_message", format(_("Please select a track of type %s."), track_type(Track::Type::AUDIO).c_str()));
	cc->enable("capture_start", ok);
}

void CaptureConsoleModeAudio::enterParent()
{
}

void CaptureConsoleModeAudio::enter()
{
	sources = cc->device_manager->getGoodDeviceList(Device::Type::AUDIO_INPUT);
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
		if (t->type == t->Type::AUDIO)
			setTarget((Track*)t);


	input = new InputStreamAudio(session);
	input->set_backup_mode(BACKUP_MODE_TEMP);
	input->set_chunk_size(4096);
	input->set_update_dt(0.03f);
	view->mode_capture->setInputAudio(input);
	peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
	peak_meter->set_source(input->out);
	cc->peak_meter->setSource(peak_meter);

	input->set_device(chosen_device);

	//enable("capture_audio_source", false);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}

	sucker = CreateAudioSucker(session);
	sucker->set_source(peak_meter->out);
	sucker->start();
	export_view_sucker = sucker;
}

void CaptureConsoleModeAudio::leave()
{
	delete sucker;
	cc->peak_meter->setSource(NULL);
	delete peak_meter;
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
	input->reset_sync();
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
	sucker->reset_accumulation();
	cc->enable("capture_audio_source", true);
	//cc->enable("capture_audio_target", true);
}

bool CaptureConsoleModeAudio::insert()
{
	int s_start = view->sel.range.start();

	// insert recorded data with some delay
	int dpos = input->get_delay();

	// overwrite
	int i0 = s_start + dpos;

	if (target->type != Track::Type::AUDIO){
		session->e(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::Type::AUDIO).c_str(), track_type(target->type).c_str()));
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

	sucker->reset_accumulation();
	return true;
}

int CaptureConsoleModeAudio::getSampleCount()
{
	return sucker->buf.length;
}

bool CaptureConsoleModeAudio::isCapturing()
{
	return input->is_capturing();
}
