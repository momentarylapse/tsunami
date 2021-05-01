/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "../AudioView.h"
#include "../Mode/ViewMode.h"
#include "../Mode/ViewModeMidi.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/CrossFade.h"
#include "../../Module/Audio/SongRenderer.h"
#include "AudioViewLayer.h"
#include "TrackHeader.h"
#include "../Helper/SymbolRenderer.h"
#include "../Painter/BufferPainter.h"

const float AudioViewTrack::MIN_GRID_DIST = 10.0f;


AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track) : scenegraph::NodeFree() {
	view = _view;
	track = _track;
	solo = false;
	midi_mode_wanted = MidiMode::CLASSICAL;
	if (view)
		if (view->midi_view_mode != MidiMode::DONT_CARE)
			midi_mode_wanted = view->midi_view_mode;

	imploded = false;
	set_perf_name("vtrack");

	if (track) {
		add_child(new TrackHeader(this));
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_CHANGE);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_ADD_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_DELETE_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_ADD_MIDI_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_DELETE_MIDI_EFFECT);
		track->subscribe(this, [=]{ track = NULL; }, track->MESSAGE_DELETE);
	} else {
		hidden = true;
	}
}

AudioViewTrack::~AudioViewTrack() {
	if (track)
		track->unsubscribe(this);
}

void AudioViewTrack::on_track_change() {
	view->renderer->allow_layers(view->get_playable_layers());
	notify(MESSAGE_CHANGE);
}


bool AudioView::editing_track(Track *t) {
	if (cur_track() != t)
		return false;
	if (session->in_mode(EditMode::DefaultTrack))
		return true;
	if (session->in_mode(EditMode::DefaultFx))
		return true;
	if (session->in_mode(EditMode::DefaultMidiFx))
		return true;
	if (session->in_mode(EditMode::EditTrack))
		return true;
	if (session->in_mode(EditMode::Capture))
		return true;
	return false;
}

void AudioViewTrack::set_muted(bool muted) {
	track->set_muted(muted);
	/*view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);*/
}

void AudioViewTrack::set_solo(bool _solo) {
	solo = _solo;
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}

void AudioViewTrack::set_panning(float panning) {
	track->set_panning(panning);
}

void AudioViewTrack::set_volume(float volume) {
	track->set_volume(volume);
}


void AudioViewTrack::set_midi_mode(MidiMode mode) {
	midi_mode_wanted = mode;
	notify();
	view->update_menu(); // FIXME
	view->thm.set_dirty();
	view->force_redraw();
}

MidiMode AudioViewTrack::midi_mode() {
	if ((midi_mode_wanted == MidiMode::TAB) and (track->instrument.string_pitch.num > 0))
		return MidiMode::TAB;
	return midi_mode_wanted;
}

void AudioViewTrack::draw_imploded_data(Painter *c) {
	auto *l = view->get_layer(track->layers[0].get());
	view->buffer_painter->set_context(l->area);

	view->buffer_painter->set_color(is_playable() ? view->colors.text_soft1 : view->colors.text_soft3);

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
			draw_buffer_selection(c, b, view_pos_rel, view->colors.selection_boundary, view->sel.range);
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

