/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioView.h"
#include "Mode/ViewMode.h"
#include "Mode/ViewModeMidi.h"
#include "../Session.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/Midi/MidiData.h"
#include "../Module/Audio/SongRenderer.h"
#include "Helper/SymbolRenderer.h"

const float AudioViewTrack::MIN_GRID_DIST = 10.0f;

AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track)
{
	view = _view;
	track = _track;
	solo = false;

	area = rect(0, 0, 0, 0);
	imploded = false;

	if (track){
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_CHANGE);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_ADD_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_DELETE_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_ADD_MIDI_EFFECT);
		track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_DELETE_MIDI_EFFECT);
		track->subscribe(this, [=]{ track = NULL; }, track->MESSAGE_DELETE);
	}
}

AudioViewTrack::~AudioViewTrack()
{
	if (track)
		track->unsubscribe(this);
}

void AudioViewTrack::on_track_change()
{
	notify(MESSAGE_CHANGE);
}


bool AudioView::editing_track(Track *t)
{
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

void AudioViewTrack::draw_header(Painter *c)
{
	bool hover = (view->hover.track == track) and view->hover.is_in(Selection::Type::TRACK_HEADER);
	bool visible = hover or view->editing_track(track);
	bool playable = view->get_playable_tracks().contains(track);

	color col = view->colors.background_track_selected;
	if (view->sel.has(track))
		col = ColorInterpolate(col, view->colors.selection, 0.4f);
	if (hover)
		col = ColorInterpolate(col, view->colors.hover, 0.2f);
	c->set_color(col);
	float h = visible ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	c->set_roundness(view->CORNER_RADIUS);
	c->draw_rect(area.x1,  area.y1,  view->TRACK_HANDLE_WIDTH, h);
	c->set_roundness(0);

	// track title
	c->set_font("", view->FONT_SIZE, view->sel.has(track) and playable, false);
	if (playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft2);
	c->draw_str(area.x1 + 23, area.y1 + 5, track->nice_name() + (solo ? " (solo)" : ""));

	c->set_font("", -1, false, false);

	// icons
	if (track->type == SignalType::BEATS){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x1 + 5, area.y1 + 5, *view->images.track_time); // "⏱"
	}else if (track->type == SignalType::MIDI){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x1 + 5, area.y1 + 5, *view->images.track_midi); // "♫"
	}else{
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x1 + 5, area.y1 + 5, *view->images.track_audio); // "∿"
	}
	if (track->muted and !visible)
		c->draw_image(area.x1 + 5, area.y1 + 5, *view->images.x);

	color col_but = ColorInterpolate(view->colors.text, view->colors.hover, 0.3f);
	color col_but_hover = view->colors.text;

	if (visible){
		c->set_color(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_MUTE))
			c->set_color(col_but_hover);
		//c->drawStr(area.x1 + 5, area.y1 + 22-2, "\U0001f50a"); // U+1F50A "🔊"
		c->draw_mask_image(area.x1 + 5, area.y1 + 22, *view->images.speaker);
		if (track->muted)
			c->draw_image(area.x1 + 5, area.y1 + 22, *view->images.x);
	}
	if ((view->song->tracks.num > 1) and visible){
		c->set_color(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_SOLO))
			c->set_color(col_but_hover);
		//c->drawStr(area.x1 + 5 + 17, area.y1 + 22-2, "S");
		c->draw_mask_image(area.x1 + 22, area.y1 + 22, *view->images.solo);
	}
	if (visible){
		c->set_color(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_EDIT))
			c->set_color(col_but_hover);
		c->draw_str(area.x1 + 5 + 17*2, area.y1 + 22, "\U0001f527"); // U+1F527 "🔧"

		/*c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_FX))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*3, area.y1 + 22-2, "⚡"); // ...*/

		/*c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_CURVE))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*4, area.y1 + 22-2, "☊"); // ... */
	}
}

void AudioViewTrack::set_muted(bool muted)
{
	track->set_muted(muted);
	view->renderer->allow_tracks(view->get_playable_tracks());
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}

void AudioViewTrack::set_solo(bool _solo)
{
	solo = _solo;
	view->renderer->allow_tracks(view->get_playable_tracks());
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}

void AudioViewTrack::set_panning(float panning)
{
	track->set_panning(panning);
}

void AudioViewTrack::set_volume(float volume)
{
	track->set_volume(volume);
}


void AudioViewTrack::draw(Painter *c)
{
	if (imploded){
		view->mode->draw_imploded_track_data(c, this);

	}else{
		view->mode->draw_track_data(c, this);
	}

	draw_header(c);
}

bool AudioViewTrack::is_playable()
{
	return view->get_playable_tracks().contains(track);
}

