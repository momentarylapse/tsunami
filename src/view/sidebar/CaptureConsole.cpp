/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "CaptureConsole.h"
#include "captureconsolemodes/CaptureConsoleMode.h"
#include "captureconsolemodes/CaptureConsoleModeAudio.h"
#include "captureconsolemodes/CaptureConsoleModeMidi.h"
#include "captureconsolemodes/CaptureConsoleModeMulti.h"
#include "captureconsolemodes/CaptureTrackData.h"
#include "../audioview/AudioView.h"
#include "../mode/ViewModeCapture.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../device/stream/AudioOutput.h"
#include "../../module/SignalChain.h"
#include "../../Session.h"
#include "../../Playback.h"
#include "../../EditModes.h"


bool _capture_console_force_complex_ = false;


CaptureConsole::CaptureConsole(Session *session, SideBar *bar):
	SideBarConsole(_("Recording"), "capture-console", session, bar)
{
	mode = nullptr;
	state = State::EMPTY;


	// dialog
	embed_dialog("record_dialog", 0, 0);


	// dialog
	peak_meter_display = new PeakMeterDisplay(this, "level", nullptr);


	event("cancel", [this] { on_cancel(); });
	event("ok", [this] { on_ok(); });
	event("start", [this] { on_start(); });
	event("dump", [this] { on_dump(); });
	event("pause", [this] { on_pause(); });
	event("new_version", [this] { on_new_version(); });

	mode_audio = new CaptureConsoleModeAudio(this);
	mode_midi = new CaptureConsoleModeMidi(this);
	mode_multi = new CaptureConsoleModeMulti(this);
}

void CaptureConsole::on_enter() {
	hide_control("single_grid", true);
	hide_control("multi_grid", true);

	state = State::EMPTY;
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);

	int num_audio = 0, num_midi = 0;
	for (Track *t: weak(view->song->tracks))
		if (view->sel.has(t)) {
			if (t->type == SignalType::AUDIO)
				num_audio ++;
			if (t->type == SignalType::MIDI)
				num_midi ++;
		}

	if ((num_audio == 1) and (num_midi == 0) and !_capture_console_force_complex_) {
		mode = mode_audio.get();
	} else if ((num_audio == 0) and (num_midi == 1) and !_capture_console_force_complex_) {
		mode = mode_midi.get();
	} else {
		mode = mode_multi.get();
	}

	mode->enter();
	chain = mode->chain.get();
	view->mode_capture->chain = chain.get();

	session->playback->signal_chain->out_tick >> create_sink([this] {
		on_putput_tick();
	});
	session->playback->signal_chain->out_play_end_of_stream >> create_sink([this] {
		on_output_end_of_stream();
	});

	// automatically start
	if (num_audio + num_midi == 1 and !_capture_console_force_complex_)
		on_start();
}

void CaptureConsole::on_leave() {
	//view->mode_capture->data.clear();
	chain = nullptr;
	view->mode_capture->chain = nullptr;
	session->playback->signal_chain->unsubscribe(this);

	view->stop();

	mode->leave();
}

base::future<bool> CaptureConsole::test_allow_close() {
	base::promise<bool> promise;
	if (!has_data()) {
		promise(true);
	} else {
		hui::question_box(win, _("Question"), _("Cancel recording?")).then([promise] (bool answer) mutable {
			promise(answer);
		});
	}

	return promise.get_future();
}


void CaptureConsole::on_start() {
	if (state == State::PAUSED) {
	} else {
		view->prepare_playback(view->get_playback_selection(true), false);
	}
	
	mode->start_sync_before();
	
	session->playback->signal_chain->start();
	mode->accumulation_start();
	mode->allow_change_device(false);
	enable("start", false);
	enable("pause", true);
	enable("dump", true);
	enable("ok", true);
	state = State::CAPTURING;
}

void CaptureConsole::on_dump() {
	view->stop();
	mode->accumulation_clear();
	mode->allow_change_device(true);
	enable("start", true);
	enable("pause", false);
	enable("dump", false);
	enable("ok", false);
	update_time();
	state = State::EMPTY;
}

void CaptureConsole::on_pause() {
	// TODO...
	session->playback->signal_chain->stop();

	mode->accumulation_stop();
	mode->allow_change_device(true);
	enable("start", true);
	enable("pause", false);
	state = State::PAUSED;
}


void CaptureConsole::on_ok() {
	view->stop();
	mode->accumulation_stop();
	if (has_data())
		view->mode_capture->insert();
	session->set_mode(EditMode::Default);
}

void CaptureConsole::on_cancel() {
	//on_dump();
	session->set_mode(EditMode::Default);
}

void CaptureConsole::on_new_version() {
	if (has_data()) {
		view->stop();
		mode->accumulation_stop();
		view->mode_capture->insert();
		on_dump();
	}
	on_start();
}

void CaptureConsole::update_time() {
	if (!chain)
		return;
	if (auto s = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0))
		set_string("time", song->get_time_str_long((int)*s));
}

void CaptureConsole::on_output_end_of_stream() {
	view->stop();
	mode->accumulation_stop();
	enable("start", true);
	enable("pause", false);
	enable("dump", true);
	state = State::PAUSED;
}

void CaptureConsole::on_putput_tick() {
	update_time();
	
	n_sync ++;
	if (n_sync > 20) {
		mode->sync();
		n_sync = 0;
	}
}

bool CaptureConsole::has_data() {
	return state != State::EMPTY;
}

