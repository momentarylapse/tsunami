/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../sidebar/captureconsolemodes/CaptureTrackData.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../painter/BufferPainter.h"
#include "../sidebar/SideBar.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/Song.h"
#include "../../data/TrackLayer.h"
#include "../../module/SignalChain.h"
#include "../../module/audio/AudioAccumulator.h"
#include "../../module/midi/MidiAccumulator.h"
#include "../../Session.h"





ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	mode_name = "capture";
	chain = nullptr;
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

			view->buffer_painter->set_context(l->area, AudioViewMode(0));
			view->buffer_painter->set_color(theme.capture_marker, l->background_color());

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
	view->draw_time_line(c, offset + l, theme.capture_marker, false, true);
}

base::set<Track*> ViewModeCapture::prevent_playback() {
	base::set<Track*> prev;
	for (auto &d: data)
		if (d.enabled)
			prev.add(d.track);
	return prev;
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
	if (data.num >= 1)
		clear_sync_data();
#endif

	song->begin_action_group("insert capture");
	for (auto &d: data)
		d.insert(view->get_playback_selection(true).start());
	song->end_action_group();
}
