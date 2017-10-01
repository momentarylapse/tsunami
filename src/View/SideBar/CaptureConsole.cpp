/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Tsunami.h"
#include "../../Device/OutputStream.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "CaptureConsoleModes/CaptureConsoleMode.h"
#include "CaptureConsoleModes/CaptureConsoleModeAudio.h"
#include "CaptureConsoleModes/CaptureConsoleModeMidi.h"
#include "CaptureConsoleModes/CaptureConsoleModeMulti.h"

#include "CaptureConsole.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Device.h"




CaptureConsole::CaptureConsole(Song *s, AudioView *v):
	SideBarConsole(_("Recording"))
{
	song = s;
	view = v;
	mode = NULL;


	// dialog
	setBorderWidth(5);
	embedDialog("record_dialog", 0, 0);

	device_manager = tsunami->device_manager;


	// dialog
	peak_meter = new PeakMeter(this, "capture_level", NULL, view);

	//enable("capture_type", false);


	event("cancel", std::bind(&CaptureConsole::onCancel, this));
	//event("hui:close", std::bind(&CaptureConsole::onClose, this));
	event("ok", std::bind(&CaptureConsole::onOk, this));
	//event("capture_type", std::bind(&CaptureConsole::onType, this));
	event("capture_start", std::bind(&CaptureConsole::onStart, this));
	event("capture_delete", std::bind(&CaptureConsole::onDelete, this));
	event("capture_pause", std::bind(&CaptureConsole::onPause, this));

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

inline int dev_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return Device::TYPE_AUDIO_INPUT;
	return Device::TYPE_MIDI_INPUT;
}

void CaptureConsole::onEnter()
{
	hideControl("capture_audio_grid", true);
	hideControl("capture_midi_grid", true);
	hideControl("capture_multi_grid", true);

	int num_audio = 0, num_midi = 0;
	for (const Track *t: view->sel.tracks){
		if (t->type == t->TYPE_AUDIO)
			num_audio ++;
		if (t->type == t->TYPE_MIDI)
			num_midi ++;
	}

	if ((num_audio == 1) and (num_midi == 0)){
		mode = mode_audio;
		setInt("capture_type", 0);
	}else if ((num_audio == 0) and (num_midi == 1)){
		mode = mode_midi;
		setInt("capture_type", 1);
	}else{ // TYPE_TIME
		mode = mode_multi;
		setInt("capture_type", 2);
	}

	mode_audio->enterParent();
	mode_midi->enterParent();
	mode_multi->enterParent();

	view->setMode(view->mode_capture);

	mode->enter();

	// automatically start
	onStart();
}

void CaptureConsole::onLeave()
{
	if (mode->isCapturing())
		mode->insert();

	view->stop();

	mode->leave();

	mode_audio->leaveParent();
	mode_midi->leaveParent();
	mode_multi->leaveParent();

	view->setMode(view->mode_default);
}


void CaptureConsole::onType()
{
	/*mode->leave();

	int n = getInt("capture_type");
	if (n == 0)
		mode = mode_audio;
	if (n == 1)
		mode = mode_midi;
	if (n == 2)
		mode = mode_multi;
	mode->enter();*/
}


void CaptureConsole::onStart()
{
	if (view->isPlaying()){
		view->pause(false);
	}else{
		mode->dump();
		view->play(view->getPlaybackSelection(true), false);
	}
	view->stream->subscribe(this, std::bind(&CaptureConsole::onOutputUpdate, this), view->stream->MESSAGE_UPDATE);
	view->stream->subscribe(this, std::bind(&CaptureConsole::onOutputEndOfStream, this), view->stream->MESSAGE_PLAY_END_OF_STREAM);

	mode->start();
	enable("capture_start", false);
	enable("capture_pause", true);
	enable("capture_delete", true);
	enable("ok", true);
	//enable("capture_type", false);
}

void CaptureConsole::onDelete()
{
	if (view->isPlaying()){
		view->stream->unsubscribe(this);
		view->stop();
	}
	mode->dump();
	enable("capture_start", true);
	enable("capture_pause", false);
	enable("capture_delete", false);
	//enable("capture_type", true);
	enable("ok", false);
	updateTime();
}

void CaptureConsole::onPause()
{
	// TODO...
	//view->stream->unsubscribe(this);
	view->pause(true);
	mode->pause();
	enable("capture_start", true);
	enable("capture_pause", false);
}


void CaptureConsole::onOk()
{
	mode->stop();
	if (mode->insert())
		bar()->_hide();
}

void CaptureConsole::onCancel()
{
	mode->stop();
	bar()->_hide();
}

void CaptureConsole::onClose()
{
	bar()->_hide();
}

void CaptureConsole::updateTime()
{
	setString("capture_time", song->get_time_str_long(mode->getSampleCount()));
}

void CaptureConsole::onOutputEndOfStream()
{
	view->stream->unsubscribe(this);
	view->stop();
	mode->pause();
	enable("capture_start", true);
	enable("capture_pause", false);
	enable("capture_delete", true);
}

void CaptureConsole::onOutputUpdate()
{
	updateTime();
}

bool CaptureConsole::isCapturing()
{
	return mode->isCapturing();
}
