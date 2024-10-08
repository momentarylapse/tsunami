/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioViewLayer.h"
#include "TrackHeader.h"
#include "../AudioView.h"
#include "../../helper/SymbolRenderer.h"
#include "../../painter/BufferPainter.h"
#include "../../mode/ViewMode.h"
#include "../../ColorScheme.h"
#include "../../../Session.h"
#include "../../../EditModes.h"
#include "../../../data/base.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/audio/AudioBuffer.h"
#include "../../../data/midi/MidiData.h"

namespace tsunami {

const float AudioViewTrack::MinGridDist = 10.0f;


AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track) :
		in_track_changed(this, [this] { on_track_change(); }) {
	view = _view;
	track = _track;
	solo = false;
	audio_mode = AudioViewMode::Peaks;
	midi_mode_wanted = MidiMode::Classical;
	if (view)
		if (view->midi_view_mode != MidiMode::DontCare)
			midi_mode_wanted = view->midi_view_mode;

	imploded = false;
	set_perf_name("vtrack");

	if (track) {
		header = new TrackHeader(this);
		add_child(header);
		track->out_changed >> in_track_changed;
		track->out_effect_list_changed >> in_track_changed;
		track->out_midi_effect_list_changed >> in_track_changed;
		track->out_death >> create_sink([this] { track = nullptr; });
	} else {
		hidden = true;
		header = nullptr;
	}
}

AudioViewTrack::~AudioViewTrack() {
}

void AudioViewTrack::on_track_change() {
	view->update_playback_layers(); // TODO let AudioView observe
	out_changed.notify();
}


bool AudioView::editing_track(Track *t) {
	if (cur_track() != t)
		return false;
	if (in_mode(EditMode::DefaultTrack))
		return true;
	if (in_mode(EditMode::DefaultTrackFx))
		return true;
	if (in_mode(EditMode::DefaultMidiFx))
		return true;
	if (in_mode(EditMode::Curves))
		return true;
	if (in_mode(EditMode::EditTrack))
		return true;
	if (in_mode(EditMode::Capture))
		return true;
	return false;
}

void AudioViewTrack::set_muted(bool muted) {
	track->set_muted(muted);
}

void AudioViewTrack::set_solo(bool _solo) {
	solo = _solo;
	out_changed.notify();
	out_solo_changed.notify();
}

void AudioViewTrack::set_panning(float panning) {
	track->set_panning(panning);
}

void AudioViewTrack::set_volume(float volume) {
	track->set_volume(volume);
}


void AudioViewTrack::set_midi_mode(MidiMode mode) {
	midi_mode_wanted = mode;
	out_changed.notify();
	view->update_menu(); // FIXME
	view->thm.set_dirty();
	view->force_redraw();
}


void AudioViewTrack::set_audio_mode(AudioViewMode mode) {
	audio_mode = mode;
	out_changed.notify();
	view->update_menu(); // FIXME
	view->thm.set_dirty();
	view->force_redraw();
}

MidiMode AudioViewTrack::midi_mode() {
	if ((midi_mode_wanted == MidiMode::Tab) and (track->instrument.string_pitch.num > 0))
		return MidiMode::Tab;
	return midi_mode_wanted;
}

void AudioViewTrack::draw_imploded_data(Painter *c) {
	auto *l = view->get_layer(track->layers[0].get());
	view->buffer_painter->set_context(l->area, audio_mode);

	if (is_playable())
		view->buffer_painter->set_color(theme.text_soft1, theme.background);
	else
		view->buffer_painter->set_color(theme.text_soft3, theme.background);

	for (auto *layer: weak(track->layers)) {
		auto rr = layer->active_version_ranges();

		for (auto &r: rr) {
			view->buffer_painter->set_clip(r);

			for (AudioBuffer &b: layer->buffers)
				view->buffer_painter->draw_buffer(c, b, b.offset);
				
			/*for (auto *m: layer->markers)
				if (m->range.overlaps(r))
					...argh*/
		}
	}

	/*if (view->sel.has(layer)){
		// selection
		for (AudioBuffer &b: layer->buffers){
			draw_buffer_selection(c, b, view_pos_rel, colors.selection_boundary, view->sel.range);
		}
	}*/
}

void AudioViewTrack::on_draw(Painter *c) {
	if (imploded) {
		draw_imploded_data(c);

	} else {
		view->mode->draw_track_data(c, this);
	}
}

AudioViewLayer* AudioViewTrack::first_layer() {
	return view->get_layer(track->layers[0].get());
}

bool AudioViewTrack::is_playable() {
	return view->get_playable_tracks().contains(track);
}

}

