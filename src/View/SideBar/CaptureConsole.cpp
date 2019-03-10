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
#include "../../Module/SignalChain.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "CaptureConsoleModes/CaptureConsoleMode.h"
#include "CaptureConsoleModes/CaptureConsoleModeAudio.h"
#include "CaptureConsoleModes/CaptureConsoleModeMidi.h"
#include "CaptureConsoleModes/CaptureConsoleModeMulti.h"

#include "CaptureConsole.h"
#include "../../Session.h"





CaptureConsole::CaptureConsole(Session *session):
	SideBarConsole(_("Recording"), session)
{
	mode = nullptr;
	state = State::EMPTY;
	chain = nullptr;


	// dialog
	set_border_width(5);
	embed_dialog("record_dialog", 0, 0);


	// dialog
	peak_meter = new PeakMeterDisplay(this, "level", nullptr);


	event("cancel", [&]{ on_cancel(); });
	event("ok", [&]{ on_ok(); });
	event("start", [&]{ on_start(); });
	event("dump", [&]{ on_dump(); });
	event("pause", [&]{ on_pause(); });
	event("new_version", [&]{ on_new_version(); });

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

void CaptureConsole::on_enter()
{
	hide_control("single_grid", true);
	hide_control("multi_grid", true);

	state = State::EMPTY;
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);

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

	mode->enter();
	chain = mode->chain;
	view->mode_capture->chain = mode->chain;

	session->signal_chain->subscribe(this, [&]{ on_putput_tick(); }, Module::MESSAGE_TICK);
	session->signal_chain->subscribe(this, [&]{ on_output_end_of_stream(); }, Module::MESSAGE_PLAY_END_OF_STREAM);

	// automatically start
	if (num_audio + num_midi == 1)
		on_start();
}

void CaptureConsole::on_leave()
{
	view->mode_capture->set_data({});
	chain = nullptr;
	view->mode_capture->chain = nullptr;
	session->signal_chain->unsubscribe(this);

	view->stop();

	mode->leave();
}

bool CaptureConsole::allow_close()
{
	if (!is_capturing())
		return true;

	string answer = hui::QuestionBox(win, _("Question"), _("Cancel recording?"), true);
	return (answer == "hui:yes");
}


void CaptureConsole::on_start()
{
	if (state == State::PAUSED){
	}else{
		view->prepare_playback(view->get_playback_selection(true), false);
	}

	view->signal_chain->start();
	chain->command(ModuleCommand::ACCUMULATION_START, 0);
	mode->allow_change_device(false);
	enable("start", false);
	enable("pause", true);
	enable("dump", true);
	enable("ok", true);
	state = State::CAPTURING;
}

void CaptureConsole::on_dump()
{
	view->stop();
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	chain->command(ModuleCommand::ACCUMULATION_CLEAR, 0);
	mode->allow_change_device(true);
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);
	update_time();
	state = State::EMPTY;
}

void CaptureConsole::on_pause()
{
	// TODO...
	view->signal_chain->stop();
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	mode->allow_change_device(true);
	enable("start", true);
	enable("pause", false);
	state = State::PAUSED;
}


void CaptureConsole::on_ok()
{
	view->stop();
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	if (state != State::EMPTY)
		view->mode_capture->insert();
	session->set_mode("default");
}

void CaptureConsole::on_cancel()
{
	//on_dump();
	session->set_mode("default");
}

void CaptureConsole::on_new_version()
{
	if (state != State::EMPTY){
		view->stop();
		chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
		view->mode_capture->insert();
		on_dump();
	}
	on_start();
}

void CaptureConsole::update_time()
{
	if (!chain)
		return;
	int s = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
	set_string("time", song->get_time_str_long(s));
}

void CaptureConsole::on_output_end_of_stream()
{
	view->stop();
	chain->command(ModuleCommand::ACCUMULATION_STOP, 0);
	enable("start", true);
	enable("pause", false);
	enable("dump", true);
	state = State::PAUSED;
}

void CaptureConsole::on_putput_tick()
{
	update_time();
}

bool CaptureConsole::is_capturing()
{
	return state == State::CAPTURING;
}

