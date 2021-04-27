/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../SideBar/CaptureConsoleModes/CaptureTrackData.h"
#include "../AudioView.h"
#include "../Painter/BufferPainter.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Module/SignalChain.h"
#include "../SideBar/SideBar.h"
#include "../../Session.h"
#include "../../Module/Audio/AudioAccumulator.h"
#include "../../Module/Midi/MidiAccumulator.h"
#include "../Graph/AudioViewLayer.h"





ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	chain = nullptr;
}

ViewModeCapture::~ViewModeCapture() {
	set_data({});
}

void ViewModeCapture::on_start() {
	set_side_bar(SideBar::CAPTURE_CONSOLE);
}

void ViewModeCapture::on_end() {
}

void ViewModeCapture::draw_post(Painter *c) {
	// capturing preview
	
	if (!chain)
		return;

	int offset = view->get_playback_selection(true).offset;
	for (auto &d: data) {
		auto *l = view->get_layer(d.track->layers[0].get());
		if (d.type() == SignalType::AUDIO) {
			auto *rec = d.audio_recorder();

			view->buffer_painter->set_context(l->area);
			view->buffer_painter->set_color(view->colors.capture_marker);

			std::lock_guard<std::mutex> lock(rec->mtx_buf);
			view->update_peaks_now(rec->buf);
			view->buffer_painter->draw_buffer(c, rec->buf, offset);
		} else if (d.type() == SignalType::MIDI) {
			auto *rec = d.midi_recorder();
			std::lock_guard<std::mutex> lock(rec->mtx_buf);
			l->draw_midi(c, midi_events_to_notes(rec->buffer), true, offset);
		}
	}
	
	int l = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
	view->draw_time_line(c, offset + l, view->colors.capture_marker, false, true);
}

Set<Track*> ViewModeCapture::prevent_playback() {
	Set<Track*> prev;
	for (auto &d: data)
		if (d.enabled)
			prev.add(d.track);
	return prev;
}

void ViewModeCapture::set_data(const Array<CaptureTrackData> &_data) {
	data = _data;
}


void ViewModeCapture::insert() {

	// FIXME stupid hack
	auto clear_sync_data = [this] {
		session->i("HACK: ignoring sync data");
		for (auto& d : data)
			d.sync_points.clear();
	};
#ifdef OS_WINDOWS
	clear_sync_data();
#else
	if (data.num > 1)
		clear_sync_data();
#endif

	song->begin_action_group();
	for (auto &d: data)
		d.insert(view->get_playback_selection(true).start());
	song->end_action_group();
}
