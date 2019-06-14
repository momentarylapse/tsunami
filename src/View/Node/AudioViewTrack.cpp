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
#include "../Mode/ViewMode.h"
#include "../Mode/ViewModeMidi.h"
#include "../../Session.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/CrossFade.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../Helper/SymbolRenderer.h"
#include "../Painter/BufferPainter.h"

const float AudioViewTrack::MIN_GRID_DIST = 10.0f;


AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track) : ViewNode() { //_view->scene_graph, 0, 0, 0, 0) {
	view = _view;
	track = _track;
	solo = false;

	imploded = false;

	if (track) {
		children.add(new TrackHeader(this));
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
	notify(MESSAGE_CHANGE);
}


bool AudioView::editing_track(Track *t) {
	if (cur_track() != t)
		return false;
	if (session->in_mode("default/track"))
		return true;
	if (session->in_mode("default/fx"))
		return true;
	if (session->in_mode("default/midi-fx"))
		return true;
	if (session->in_mode("default/synth"))
		return true;
	if (session->in_mode("midi"))
		return true;
	if (session->in_mode("capture"))
		return true;
	return false;
}

void AudioViewTrack::set_muted(bool muted) {
	track->set_muted(muted);
	view->renderer->allow_tracks(view->get_playable_tracks());
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}

void AudioViewTrack::set_solo(bool _solo) {
	solo = _solo;
	view->renderer->allow_tracks(view->get_playable_tracks());
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


void AudioViewTrack::draw_imploded_data(Painter *c) {
	auto *l = view->get_layer(track->layers[0]);
	view->buffer_painter->set_context(l->area);


	if (track->has_version_selection()) {
		Range r = Range(track->range().start(), 0);
		int index = 0;
		for (auto &f: track->fades) {
			r = RangeTo(r.end(), f.position);
			view->buffer_painter->set_clip(r);

			for (AudioBuffer &b: track->layers[index]->buffers) {
				view->buffer_painter->set_color(is_playable() ? view->colors.text : view->colors.text_soft3);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}

			index = f.target;
		}

		r = RangeTo(r.end(), track->range().end());
		view->buffer_painter->set_clip(r);
		for (AudioBuffer &b: track->layers[index]->buffers) {
			view->buffer_painter->set_color(is_playable() ? view->colors.text : view->colors.text_soft3);
			view->buffer_painter->draw_buffer(c, b, b.offset);
		}
	} else {
		view->buffer_painter->set_color(is_playable() ? view->colors.text : view->colors.text_soft3);
		for (auto *layer: track->layers)
			for (AudioBuffer &b: layer->buffers)
				view->buffer_painter->draw_buffer(c, b, b.offset);

	}

	/*if (view->sel.has(layer)){
		// selection
		for (AudioBuffer &b: layer->buffers){
			draw_buffer_selection(c, b, view_pos_rel, view->colors.selection_boundary, view->sel.range);
		}
	}*/



	view->draw_boxed_str(c, area.x2 - 200, area.y1 + 10, "imploded...", view->colors.text, view->colors.background_track_selection);
}

void AudioViewTrack::draw(Painter *c) {
	if (imploded) {
		draw_imploded_data(c);

	} else {
		view->mode->draw_track_data(c, this);
	}
}

AudioViewLayer* AudioViewTrack::first_layer() {
	return view->get_layer(track->layers[0]);
}

bool AudioViewTrack::is_playable() {
	return view->get_playable_tracks().contains(track);
}

