/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Device/OutputStream.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "CaptureConsoleModes/CaptureConsoleMode.h"
#include "CaptureConsoleModes/CaptureConsoleModeAudio.h"
#include "CaptureConsoleModes/CaptureConsoleModeMidi.h"
#include "CaptureConsoleModes/CaptureConsoleModeMulti.h"

#include "CaptureConsole.h"
#include "../../Session.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Device.h"
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"





CaptureConsole::CaptureConsole(Session *session):
	SideBarConsole(_("Recording"), session)
{
	mode = nullptr;


	// dialog
	setBorderWidth(5);
	embedDialog("record_dialog", 0, 0);

	device_manager = session->device_manager;


	// dialog
	peak_meter = new PeakMeterDisplay(this, "level", nullptr);


	event("cancel", std::bind(&CaptureConsole::on_cancel, this));
	//event("hui:close", std::bind(&CaptureConsole::onClose, this));
	event("ok", std::bind(&CaptureConsole::on_ok, this));
	event("start", std::bind(&CaptureConsole::on_start, this));
	event("dump", std::bind(&CaptureConsole::on_dump, this));
	event("pause", std::bind(&CaptureConsole::on_pause, this));
	event("new_version", std::bind(&CaptureConsole::on_new_version, this));

	mode_audio = new CaptureConsoleModeAudio(this);
	mode_midi = new CaptureConsoleModeMidi(this);
	mode_multi = new CaptureConsoleModeMulti(this);
}

CaptureConsole::~CaptureConsole()
{
	delete(mode_audio);
	delete(mode_midi);
	delete(mode_multi);
	delete(peak_meter);
}

DeviceType dev_type(SignalType type)
{
	if (type == SignalType::AUDIO)
		return DeviceType::AUDIO_INPUT;
	return DeviceType::MIDI_INPUT;
}

void CaptureConsole::on_enter()
{
	hideControl("single_grid", true);
	hideControl("multi_grid", true);

	int num_audio = 0, num_midi = 0;
	for (const Track *t: view->sel.tracks){
		if (t->type == SignalType::AUDIO)
			num_audio ++;
		if (t->type == SignalType::MIDI)
			num_midi ++;
	}

	if ((num_audio == 1) and (num_midi == 0)){
		mode = mode_audio;
	}else if ((num_audio == 0) and (num_midi == 1)){
		mode = mode_midi;
	}else{ // TYPE_TIME
		mode = mode_multi;
	}

	mode_audio->enter_parent();
	mode_midi->enter_parent();
	mode_multi->enter_parent();

	view->set_mode(view->mode_capture);

	mode->enter();

	// automatically start
	if (num_audio + num_midi == 1)
		on_start();
}

void CaptureConsole::on_leave()
{
	if (mode->is_capturing())
		mode->insert();
	view->stream->unsubscribe(this);

	view->stop();

	mode->leave();

	mode_audio->leave_parent();
	mode_midi->leave_parent();
	mode_multi->leave_parent();

	view->set_mode(view->mode_default);
}


void CaptureConsole::on_start()
{
	if (view->is_playback_active()){
		view->pause(false);
	}else{
		mode->dump();
		view->play(view->get_playback_selection(true), false);
	}
	view->stream->subscribe(this, std::bind(&CaptureConsole::on_putput_update, this), view->stream->MESSAGE_UPDATE);
	view->stream->subscribe(this, std::bind(&CaptureConsole::on_output_end_of_stream, this), view->stream->MESSAGE_PLAY_END_OF_STREAM);

	mode->start();
	enable("start", false);
	enable("pause", true);
	enable("dump", true);
	enable("ok", true);
}

void CaptureConsole::on_dump()
{
	if (view->is_playback_active()){
		view->stream->unsubscribe(this);
		view->stop();
	}
	mode->dump();
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);
	update_time();
}

void CaptureConsole::on_pause()
{
	// TODO...
	//view->stream->unsubscribe(this);
	view->pause(true);
	mode->pause();
	enable("start", true);
	enable("pause", false);
}


void CaptureConsole::on_ok()
{
	view->stream->unsubscribe(this);
	mode->stop();
	if (mode->insert())
		bar()->_hide();
}

void CaptureConsole::on_cancel()
{
	view->stream->unsubscribe(this);
	mode->stop();
	bar()->_hide();
}

void CaptureConsole::on_close()
{
	bar()->_hide();
}

void CaptureConsole::on_new_version()
{
	if (view->is_playback_active()){
		view->stream->unsubscribe(this);
		view->stop();
	}
	mode->insert();
	on_start();
}

void CaptureConsole::update_time()
{
	setString("time", song->get_time_str_long(mode->get_sample_count()));
}

void CaptureConsole::on_output_end_of_stream()
{
	view->stream->unsubscribe(this);
	view->stop();
	mode->pause();
	enable("start", true);
	enable("pause", false);
	enable("dump", true);
}

void CaptureConsole::on_putput_update()
{
	update_time();
}

bool CaptureConsole::is_capturing()
{
	return mode->is_capturing();
}


bool layer_available(TrackLayer *l, const Range &r)
{
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

bool CaptureConsole::insert_audio(Track *target, AudioBuffer &buf, int i0)
{
	Song *song = target->song;

	if (target->type != SignalType::AUDIO){
		song->session->e(format(_("Can't insert recorded data (%s) into target (%s)."), signal_type_name(SignalType::AUDIO).c_str(), signal_type_name(target->type).c_str()));
		return false;
	}

	// insert data
	Range r = Range(i0, buf.length);
	song->beginActionGroup();

	TrackLayer *layer = nullptr;
	for (TrackLayer *l: target->layers)
		if (layer_available(l, r)){
			layer = l;
			break;
		}
	if (!layer)
		layer = target->addLayer();

	AudioBuffer tbuf;
	layer->getBuffers(tbuf, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(layer, r);

	/*if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(buf, 0, 1.0f, 0);
	else*/
		tbuf.set(buf, 0, 1.0f);
	song->execute(a);
	song->endActionGroup();

	return true;
}
