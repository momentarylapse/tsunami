/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Device/OutputStream.h"
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




CaptureConsole::CaptureConsole(Session *session):
	SideBarConsole(_("Recording"), session)
{
	mode = NULL;


	// dialog
	setBorderWidth(5);
	embedDialog("record_dialog", 0, 0);

	device_manager = session->device_manager;


	// dialog
	peak_meter = new PeakMeterDisplay(this, "level", NULL, view);


	event("cancel", std::bind(&CaptureConsole::onCancel, this));
	//event("hui:close", std::bind(&CaptureConsole::onClose, this));
	event("ok", std::bind(&CaptureConsole::onOk, this));
	event("start", std::bind(&CaptureConsole::onStart, this));
	event("dump", std::bind(&CaptureConsole::onDump, this));
	event("pause", std::bind(&CaptureConsole::onPause, this));

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
	if (type == Track::Type::AUDIO)
		return Device::Type::AUDIO_INPUT;
	return Device::Type::MIDI_INPUT;
}

void CaptureConsole::onEnter()
{
	hideControl("single_grid", true);
	hideControl("multi_grid", true);

	int num_audio = 0, num_midi = 0;
	for (const Track *t: view->sel.tracks){
		if (t->type == t->Type::AUDIO)
			num_audio ++;
		if (t->type == t->Type::MIDI)
			num_midi ++;
	}

	if ((num_audio == 1) and (num_midi == 0)){
		mode = mode_audio;
	}else if ((num_audio == 0) and (num_midi == 1)){
		mode = mode_midi;
	}else{ // TYPE_TIME
		mode = mode_multi;
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
	view->stream->unsubscribe(this);

	view->stop();

	mode->leave();

	mode_audio->leaveParent();
	mode_midi->leaveParent();
	mode_multi->leaveParent();

	view->setMode(view->mode_default);
}


void CaptureConsole::onStart()
{
	if (view->isPlaybackActive()){
		view->pause(false);
	}else{
		mode->dump();
		view->play(view->getPlaybackSelection(true), false);
	}
	view->stream->subscribe(this, std::bind(&CaptureConsole::onOutputUpdate, this), view->stream->MESSAGE_UPDATE);
	view->stream->subscribe(this, std::bind(&CaptureConsole::onOutputEndOfStream, this), view->stream->MESSAGE_PLAY_END_OF_STREAM);

	mode->start();
	enable("start", false);
	enable("pause", true);
	enable("dump", true);
	enable("ok", true);
}

void CaptureConsole::onDump()
{
	if (view->isPlaybackActive()){
		view->stream->unsubscribe(this);
		view->stop();
	}
	mode->dump();
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);
	updateTime();
}

void CaptureConsole::onPause()
{
	// TODO...
	//view->stream->unsubscribe(this);
	view->pause(true);
	mode->pause();
	enable("start", true);
	enable("pause", false);
}


void CaptureConsole::onOk()
{
	view->stream->unsubscribe(this);
	mode->stop();
	if (mode->insert())
		bar()->_hide();
}

void CaptureConsole::onCancel()
{
	view->stream->unsubscribe(this);
	mode->stop();
	bar()->_hide();
}

void CaptureConsole::onClose()
{
	bar()->_hide();
}

void CaptureConsole::updateTime()
{
	setString("time", song->get_time_str_long(mode->getSampleCount()));
}

void CaptureConsole::onOutputEndOfStream()
{
	view->stream->unsubscribe(this);
	view->stop();
	mode->pause();
	enable("start", true);
	enable("pause", false);
	enable("dump", true);
}

void CaptureConsole::onOutputUpdate()
{
	updateTime();
}

bool CaptureConsole::isCapturing()
{
	return mode->isCapturing();
}
