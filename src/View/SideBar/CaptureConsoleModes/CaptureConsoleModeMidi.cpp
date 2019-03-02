/*
 * CaptureConsoleModeMidi.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMidi.h"
#include "../CaptureConsole.h"
#include "../../../Device/InputStreamMidi.h"
#include "../../../Device/OutputStream.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Device.h"
#include "../../../Data/base.h"
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/SongSelection.h"
#include "../../../Module/Synth/Synthesizer.h"
#include "../../../Module/Audio/PeakMeter.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Session.h"

CaptureConsoleModeMidi::CaptureConsoleModeMidi(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	chosen_device = nullptr;
	input = nullptr;
	target = nullptr;

	peak_meter = nullptr;
	preview_synth = nullptr;
	preview_stream = nullptr;

	cc->event("source", [&]{ on_source(); });
}

void CaptureConsoleModeMidi::on_source()
{
	int n = cc->get_int("");
	if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}
}


void CaptureConsoleModeMidi::set_target(Track *t)
{
	if (preview_stream)
		delete preview_stream;
	if (peak_meter)
		delete peak_meter;
	if (preview_synth)
		delete preview_synth;

	target = t;
	// FIXME ...
	//view->setCurTrack(target);
	preview_synth = (Synthesizer*)t->synth->copy();
	preview_synth->plug(0, input, 0);
	peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
	peak_meter->plug(0, preview_synth, 0);
	preview_stream = new OutputStream(session);
	preview_stream->plug(0, peak_meter, 0);
	preview_stream->set_buffer_size(512);
	preview_stream->play();


	bool ok = (target->type == SignalType::MIDI);
	cc->set_string("message", "");
	if (!ok)
		cc->set_string("message", format(_("Please select a track of type %s."), signal_type_name(SignalType::MIDI).c_str()));
	cc->enable("start", ok);
}

void CaptureConsoleModeMidi::enter_parent()
{
}

void CaptureConsoleModeMidi::enter()
{
	chosen_device = session->device_manager->choose_device(DeviceType::MIDI_INPUT);
	sources = session->device_manager->good_device_list(DeviceType::MIDI_INPUT);
	cc->hide_control("single_grid", false);

	// add all
	cc->reset("source");
	for (Device *d: sources)
		cc->set_string("source", d->get_name());

	// select current
	foreachi(Device *d, sources, i)
		if (d == chosen_device)
			cc->set_int("source", i);


	input = new InputStreamMidi(session);
	input->set_chunk_size(512);
	input->set_update_dt(0.005f);
	cc->peak_meter->set_source(nullptr);//input);

	input->set_device(chosen_device);

	for (const Track *t: view->sel.tracks)
		if (t->type == SignalType::MIDI)
			set_target((Track*)t);

	if (!input->start()){
		/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
		CapturingByDialog = false;
		msg_db_l(1);
		return;*/
	}
	view->mode_capture->set_data({CaptureTrackData(target,input)});
}

void CaptureConsoleModeMidi::leave()
{
	cc->peak_meter->set_source(nullptr);
	view->mode_capture->set_data({});
	delete(input);
	input = nullptr;
}

void CaptureConsoleModeMidi::pause()
{
	input->accumulate(false);
}

void CaptureConsoleModeMidi::start()
{
	input->reset_sync();
	input->accumulate(true);
	cc->enable("source", false);
}

void CaptureConsoleModeMidi::stop()
{
	preview_stream->stop();
	input->stop();
}

void CaptureConsoleModeMidi::dump()
{
	input->accumulate(false);
	input->reset_accumulation();
	cc->enable("source", true);
}

bool CaptureConsoleModeMidi::insert()
{
	// insert recorded data with some delay
	int dpos = input->get_delay();
	bool ok = cc->insert_midi(target, input->midi, dpos);

	input->reset_accumulation();
	return ok;
}

int CaptureConsoleModeMidi::get_sample_count()
{
	return input->get_sample_count();
}

bool CaptureConsoleModeMidi::is_capturing()
{
	return input->is_capturing();
}


