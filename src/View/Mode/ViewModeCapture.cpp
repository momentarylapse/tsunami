/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../Node/AudioViewLayer.h"
#include "../Painter/BufferPainter.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Module/Audio/AudioRecorder.h"
#include "../../Module/Midi/MidiRecorder.h"
#include "../../Module/SignalChain.h"
#include "../SideBar/SideBar.h"
#include "../../Session.h"
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../Stream/AudioInput.h"
#include "../../Stream/MidiInput.h"

CaptureTrackData::CaptureTrackData(){}
CaptureTrackData::CaptureTrackData(Track *_target, Module *_recorder)
{
	target = _target;
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

HoverData ViewModeCapture::get_hover()
{
	return get_hover_basic(false);
}

void ViewModeCapture::draw_post(Painter *c)
{
	// capturing preview
	
	if (!chain)
		return;

	int offset = view->get_playback_selection(true).offset;
	for (auto &d: data){
		auto *l = view->get_layer(d.target->layers[0]);
		if (d.type() == SignalType::AUDIO){
			auto &buf = ((AudioRecorder*)d.recorder)->buf;
			view->update_peaks_now(buf);
			view->buffer_painter->set_context(l->area);
			view->buffer_painter->set_color(view->colors.capture_marker);
			view->buffer_painter->draw_buffer(c, buf, offset);
		}else if (d.type() == SignalType::MIDI){
			auto *rec = (MidiRecorder*)d.recorder;
			l->draw_midi(c, midi_events_to_notes(rec->buffer), true, offset);
		}
	}
	
	int l = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
	view->draw_time_line(c, offset + l, view->colors.capture_marker, false, true);
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


bool layer_available(TrackLayer *l, const Range &r)
{
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

void ViewModeCapture::insert_midi(Track *target, const MidiEventBuffer &midi, int delay)
{
	int s_start = view->get_playback_selection(true).start();

	int i0 = s_start + delay;

	// insert data
	target->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi).duplicate());
}


void ViewModeCapture::insert_audio(Track *target, const AudioBuffer &buf, int delay)
{
	Song *song = target->song;

	int s_start = view->get_playback_selection(true).start();
	int i0 = s_start + delay;

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
	auto *a = new ActionTrackEditBuffer(layer, r);

	/*if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(buf, 0, 1.0f, 0);
	else*/
		tbuf.set(buf, 0, 1.0f);
	song->execute(a);
	song->end_action_group();
}

void ViewModeCapture::insert()
{
	song->begin_action_group();
	for (auto &d: data){
		if (d.type() == SignalType::AUDIO){
			auto *rec = (AudioRecorder*)d.recorder;
			insert_audio(d.target, rec->buf, 0);
		}else if (d.type() == SignalType::MIDI){
			auto *rec = (MidiRecorder*)d.recorder;
			insert_midi(d.target, rec->buffer, 0);
		}
	}
	song->end_action_group();
}
