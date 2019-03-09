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
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"





CaptureConsole::CaptureConsole(Session *session):
	SideBarConsole(_("Recording"), session)
{
	mode = nullptr;
	state = State::EMPTY;


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
	msg_write("cc on enter.....");
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

	mode_audio->enter_parent();
	mode_midi->enter_parent();
	mode_multi->enter_parent();

	mode->enter();

	session->signal_chain->subscribe(this, [&]{ on_putput_tick(); }, Module::MESSAGE_TICK);
	session->signal_chain->subscribe(this, [&]{ on_output_end_of_stream(); }, Module::MESSAGE_PLAY_END_OF_STREAM);

	// automatically start
	if (num_audio + num_midi == 1)
		on_start();
}

void CaptureConsole::on_leave()
{
	if (state != State::EMPTY)
		mode->insert();
	session->signal_chain->unsubscribe(this);

	view->stop();

	mode->leave();

	mode_audio->leave_parent();
	mode_midi->leave_parent();
	mode_multi->leave_parent();
}


void CaptureConsole::on_start()
{
	if (state == State::PAUSED){
	}else{
		view->prepare_playback(view->get_playback_selection(true), false);
	}

	view->signal_chain->start();
	mode->start();
	enable("start", false);
	enable("pause", true);
	enable("dump", true);
	enable("ok", true);
	state = State::CAPTURING;
}

void CaptureConsole::on_dump()
{
	view->stop();
	mode->pause();
	mode->dump();
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
	mode->pause();
	enable("start", true);
	enable("pause", false);
	state = State::PAUSED;
}


void CaptureConsole::on_ok()
{
	view->stop();
	mode->pause();
	if (mode->insert())
		session->set_mode("default");
}

void CaptureConsole::on_cancel()
{
	view->stop();
	mode->pause();
	mode->dump();
	session->set_mode("default");
}

void CaptureConsole::on_new_version()
{
	if (state != State::EMPTY){
		view->stop();
		mode->insert();
		on_dump();
	}
	on_start();
}

void CaptureConsole::update_time()
{
	set_string("time", song->get_time_str_long(mode->get_sample_count()));
}

void CaptureConsole::on_output_end_of_stream()
{
	view->stop();
	mode->pause();
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


bool layer_available(TrackLayer *l, const Range &r)
{
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

bool CaptureConsole::insert_midi(Track *target, const MidiEventBuffer &midi, int delay)
{
	int s_start = view->sel.range.start();

	int i0 = s_start + delay;

	if (target->type != SignalType::MIDI){
		session->e(format(_("Can't insert recorded data (%s) into target (%s)."), signal_type_name(SignalType::MIDI).c_str(), signal_type_name(target->type).c_str()));
		return false;
	}

	// insert data
	target->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi).duplicate());
	return true;
}


bool CaptureConsole::insert_audio(Track *target, AudioBuffer &buf, int delay)
{
	Song *song = target->song;

	int s_start = view->sel.range.start();
	int i0 = s_start + delay;

	if (target->type != SignalType::AUDIO){
		song->session->e(format(_("Can't insert recorded data (%s) into target (%s)."), signal_type_name(SignalType::AUDIO).c_str(), signal_type_name(target->type).c_str()));
		return false;
	}

	// insert data
	Range r = Range(i0, buf.length);
	song->begin_action_group();

	TrackLayer *layer = nullptr;
	for (TrackLayer *l: target->layers)
		if (layer_available(l, r)){
			layer = l;
			break;
		}
	if (!layer)
		layer = target->add_layer();

	AudioBuffer tbuf;
	layer->get_buffers(tbuf, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(layer, r);

	/*if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(buf, 0, 1.0f, 0);
	else*/
		tbuf.set(buf, 0, 1.0f);
	song->execute(a);
	song->end_action_group();

	return true;
}
