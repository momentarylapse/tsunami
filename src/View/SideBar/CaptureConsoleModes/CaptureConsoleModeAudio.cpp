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

	cc->event("source", std::bind(&CaptureConsoleModeAudio::onSource, this));
}

void CaptureConsoleModeAudio::onSource()
{
	int n = cc->getInt("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}

void CaptureConsoleModeAudio::setTarget(Track *t)
{
	target = t;
	view->setCurTrack(target);
	view->mode_capture->capturing_track = target;

	bool ok = (target->type == Track::Type::AUDIO);
	cc->setString("message", "");
	if (!ok)
		cc->setString("message", format(_("Please select a track of type %s."), track_type(Track::Type::AUDIO).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeAudio::enterParent()
{
}

void CaptureConsoleModeAudio::enter()
{
	sources = cc->device_manager->getGoodDeviceList(Device::Type::AUDIO_INPUT);
	cc->hideControl("single_grid", false);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->setString("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->setInt("source", i);


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
	cc->enable("source", false);
}

void CaptureConsoleModeAudio::stop()
{
	input->stop();
}

void CaptureConsoleModeAudio::dump()
{
	sucker->accumulate(false);
	sucker->reset_accumulation();
	cc->enable("source", true);
}

bool layer_available(TrackLayer *l, const Range &r)
{
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
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

	TrackLayer *layer = NULL;
	for (TrackLayer *l: target->layers)
		if (layer_available(l, r)){
			layer = l;
			break;
		}
	if (!layer)
		layer = target->addLayer(false);

	AudioBuffer tbuf;
	layer->getBuffers(tbuf, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(layer, r);

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
