/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../Painter/BufferPainter.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Module/Audio/AudioRecorder.h"
#include "../../Module/Midi/MidiRecorder.h"
#include "../../Module/SignalChain.h"
#include "../SideBar/SideBar.h"

CaptureTrackData::CaptureTrackData(){}
CaptureTrackData::CaptureTrackData(Track *_target, Module *_input, Module *_recorder)
{
	target = _target;
	input = _input;
	recorder = _recorder;
}

SignalType CaptureTrackData::type()
{
	return target->type;
}


ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	side_bar_console = SideBar::CAPTURE_CONSOLE;
	chain = nullptr;
}

ViewModeCapture::~ViewModeCapture()
{
	set_data({});
}

void ViewModeCapture::on_start()
{
}

void ViewModeCapture::on_end()
{
}

Selection ViewModeCapture::get_hover()
{
	return get_hover_basic(false);
}

void ViewModeCapture::draw_post(Painter *c)
{
	// capturing preview
	
	if (!chain)
		return;

	for (auto &d: data){
		if (d.type() == SignalType::AUDIO){
			InputStreamAudio *input_audio = (InputStreamAudio*)d.input;
				AudioBuffer &buf = ((AudioRecorder*)d.recorder)->buf;
				view->update_peaks_now(buf);
				auto *l = view->get_layer(d.target->layers[0]);
				view->buffer_painter->set_context(l->area);
				view->buffer_painter->set_color(view->colors.capture_marker);
				view->buffer_painter->draw_buffer(c, buf, view->sel.range.offset);
				//view->draw_time_line(c, view->sel.range.start() + buf.length, (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
		}else if (d.type() == SignalType::MIDI){
				auto *rec = (MidiRecorder*)d.recorder;
				draw_midi(c, view->get_layer(d.target->layers[0]), midi_events_to_notes(rec->buffer), true, view->sel.range.start());
				//view->draw_time_line(c, view->sel.range.start() + rec->buffer.samples, (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
		}
	}
	
	int l = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
	view->draw_time_line(c, view->sel.range.start() + l, (int)Selection::Type::PLAYBACK, view->colors.capture_marker, true);
}

Set<Track*> ViewModeCapture::prevent_playback()
{
	Set<Track*> prev;
	for (auto &d: data)
		prev.add(d.target);
	return prev;
}

void ViewModeCapture::set_data(const Array<CaptureTrackData> &_data)
{
	data = _data;
}
