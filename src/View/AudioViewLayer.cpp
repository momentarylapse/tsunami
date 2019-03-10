/*
 * AudioViewLayer.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewLayer.h"
#include "AudioView.h"
#include "Mode/ViewMode.h"
#include "Mode/ViewModeMidi.h"
#include "Painter/BufferPainter.h"
#include "Painter/GridPainter.h"
#include "Painter/MidiPainter.h"
#include "../Tsunami.h"
#include "../Session.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/TrackMarker.h"
#include "../Data/SampleRef.h"
#include "../Data/Sample.h"
#include "../Data/CrossFade.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/Rhythm/Bar.h"
#include "../Data/Rhythm/Beat.h"
#include "../Data/Rhythm/BarCollection.h"
#include "../Data/Midi/MidiData.h"
#include "../Data/Midi/Clef.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Audio/SongRenderer.h"
#include "Helper/SymbolRenderer.h"


const int PITCH_SHOW_COUNT = 30;



AudioViewLayer::AudioViewLayer(AudioView *_view, TrackLayer *_layer)
{
	view = _view;
	layer = _layer;
	solo = false;

	edit_pitch_min = 55;
	edit_pitch_max = edit_pitch_min + PITCH_SHOW_COUNT;

	area = rect(0, 0, 0, 0);
	height_min = height_wish = 0;

	hidden = false;
	represents_imploded = false;

	if (layer)
		set_midi_mode(view->midi_view_mode);
}

void AudioViewLayer::set_midi_mode(MidiMode wanted)
{
	midi_mode = wanted;
	if ((wanted == MidiMode::TAB) and (layer->track->instrument.string_pitch.num > 0))
		midi_mode = MidiMode::TAB;
	view->thm.dirty = true;
	view->force_redraw();
}

color AudioViewLayer::marker_color(const TrackMarker *m)
{
	return MidiPainter::pitch_color(m->text.hash() % MAX_PITCH);
}

// active | passive | active | ...
Array<Range> version_ranges(TrackLayer *l)
{
	Array<Range> r;
	if (!l->is_main())
		r.add(Range::EMPTY);
	int own_index = l->version_number();
	CrossFade prev;
	prev.position = -2000000000;
	prev.samples = 0;
	prev.target = 0;
	for (auto &f: l->track->fades){
		if (f.target == own_index){
			r.add(RangeTo(prev.range().end(), f.range().start()));
			prev = f;
		}else if (prev.target == own_index){
			r.add(RangeTo(prev.range().start(), f.range().end()));
			prev = f;
		}
	}
	r.add(RangeTo(r.back().end(), 2000000000));
	return r;
}

void AudioViewLayer::draw_track_buffers(Painter *c)
{
	view->buffer_painter->set_context(area);
	if (is_playable() and layer->track->has_version_selection()){
		Array<Range> rr = version_ranges(layer);
		for (AudioBuffer &b: layer->buffers){
			foreachi(Range &r, rr, i){
				view->buffer_painter->set_clip(r);
				view->buffer_painter->set_color((i % 2) ? view->colors.text_soft3 : view->colors.text);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}
		}
	}else{
		view->buffer_painter->set_color(is_playable() ? view->colors.text : view->colors.text_soft3);
		for (AudioBuffer &b: layer->buffers)
			view->buffer_painter->draw_buffer(c, b, b.offset);

	}

	if (view->sel.has(layer)){
		// selection
		view->buffer_painter->set_color(view->colors.selection_boundary);
		view->buffer_painter->set_clip(view->sel.range);
		for (AudioBuffer &b: layer->buffers)
			view->buffer_painter->draw_buffer_selection(c, b, b.offset);
	}
}

void AudioViewLayer::draw_sample_frame(Painter *c, SampleRef *s, const color &col, int delay)
{
	// frame
	Range rr = s->range() + delay;
	float asx, aex;
	view->cam.range2screen_clip(rr, area, asx, aex);

	if (delay == 0)
		s->area = rect(asx, aex, area.y1, area.y2);


	color col2 = col;
	col2.a *= 0.5f;
	c->set_color(col2);
	c->draw_rect(asx, area.y1,                             aex - asx, view->SAMPLE_FRAME_HEIGHT);
	c->draw_rect(asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, aex - asx, view->SAMPLE_FRAME_HEIGHT);

	c->set_color(col);
	c->set_line_width(2);
	c->draw_line(asx, area.y1, asx, area.y2);
	c->draw_line(aex, area.y1, aex, area.y2);
	c->draw_line(asx, area.y1, aex, area.y1);
	c->draw_line(asx, area.y2, aex, area.y2);
}


void AudioViewLayer::draw_sample(Painter *c, SampleRef *s)
{
	color col = view->colors.sample;
	if (view->sel.has(s))
		col = view->colors.sample_selected;
	if (view->hover.sample == s)
		col = ColorInterpolate(col,  view->colors.hover, 0.2f);

	draw_sample_frame(c, s, col, 0);

	// buffer
	if (s->type() == SignalType::AUDIO){
		view->buffer_painter->set_color(col);
		view->buffer_painter->draw_buffer(c, *s->buf, s->pos);
	}else if (s->type() == SignalType::MIDI){
		view->mode->draw_midi(c, this, *s->midi, true, s->pos);
	}
}

void AudioViewLayer::draw_marker(Painter *c, const TrackMarker *marker, int index, bool hover)
{
	string text = marker->text;
	if (marker->fx.num > 0)
		text += format(" (%d fx)", marker->fx.num);

	bool sel = view->sel.has(marker);

	if (sel)
		c->set_font("", -1, true, false);

	float w = c->get_str_width(text) + view->CORNER_RADIUS * 2;
	float x0, x1;
	view->cam.range2screen(marker->range, x0, x1);
	float w_threshold = view->high_details ? 20 : 40;
	if (x1 - x0 < w_threshold)
		return;
	float y0 = area.y1;
	float y1 = y0 + 5;

	w = max(w, x1 - x0);

	color col = view->colors.text;
	color col_bg = view->colors.background_track;
	color col2 = marker_color(marker);
	if (sel){
		col = view->colors.text;//view->colors.selection;
		col_bg = view->colors.selection;//ColorInterpolate(view->colors.background_track, view->colors.selection, 0.2f);
		col_bg.a = 0.5f;
		col2 = ColorInterpolate(col2, view->colors.selection, 0.8f);
	}
	if (hover){
		col = ColorInterpolate(col, view->colors.hover, 0.3f);
		col2 = ColorInterpolate(col2, view->colors.hover, 0.3f);
		col_bg = view->colors.hover;
		col_bg.a = 0.5f;
	}

	view->draw_boxed_str(c,  x0 + view->CORNER_RADIUS, y0 + 10, text, col, col_bg);
	c->set_color(col2);
	c->draw_rect(x0, y0, x1-x0, y1-y0);
	c->set_color(col2);
	c->set_line_width(2.0f);
	c->draw_line(x0, area.y1, x0, area.y2);
	c->draw_line(x1, area.y1, x1, area.y2);
	c->set_line_width(1.0f);

	marker_areas[index] = rect(x0, x0 + w, y0, y0 + 15);
	marker_label_areas[index] = view->get_boxed_str_rect(c,  x0 + view->CORNER_RADIUS, y0 + 10, text);

	c->set_font("", view->FONT_SIZE, false, false);
}



void AudioViewLayer::set_edit_pitch_min_max(int _min, int _max)
{
	int diff = _max - _min;
	edit_pitch_min = clampi(_min, 0, MAX_PITCH - 1 - diff);
	edit_pitch_max = edit_pitch_min + diff;
	view->force_redraw();
}

bool AudioViewLayer::mouse_over()
{
	return !hidden and area.inside(view->mx, view->my);
}

bool AudioViewLayer::is_playable()
{
	return view->get_playable_layers().contains(layer);
}

color AudioViewLayer::background_color()
{
	return (view->sel.has(layer)) ? view->colors.background_track_selected : view->colors.background_track;
}

color AudioViewLayer::background_selection_color()
{
	if (view->selection_mode == view->SelectionMode::RECT)
		return background_color(); // complex selection rect as overlay...
	return (view->sel.has(layer)) ? view->colors.background_track_selection : view->colors.background_track;
}

bool AudioView::editing_layer(AudioViewLayer *l)
{
	if (cur_vlayer != l)
		return false;
	if (session->in_mode("midi"))
		return true;
	if (session->in_mode("capture"))
		return true;
	return false;
}


void AudioViewLayer::draw_version_header(Painter *c)
{
	bool hover = (view->hover.layer == layer) and view->hover.is_in(Selection::Type::LAYER_HEADER);
	bool visible = hover or view->editing_layer(this);
	bool playable = view->get_playable_layers().contains(layer);

	color col = view->colors.background_track_selected;
	if (view->sel.has(layer))
		col = ColorInterpolate(col, view->colors.selection, 0.2f);
	if (hover)
		col = ColorInterpolate(col, view->colors.hover, 0.2f);
	c->set_color(col);
	float h = visible ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	c->set_roundness(view->CORNER_RADIUS);
	c->draw_rect(area.x2 - view->LAYER_HANDLE_WIDTH,  area.y1,  view->LAYER_HANDLE_WIDTH, h);
	c->set_roundness(0);

	// track title
	c->set_font("", view->FONT_SIZE, view->sel.has(layer) and playable, false);
	if (playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft2);
	string title;
	if (layer->track->has_version_selection()){
		if (layer->is_main())
			title = _("base");
		else
			title = "v" + i2s(layer->version_number() + 1);
	}else{
		title = "l" + i2s(layer->version_number() + 1);
	}
	if (solo)
		title += " (solo)";
	c->draw_str(area.x2 - view->LAYER_HANDLE_WIDTH + 23, area.y1 + 5, title);

	c->set_font("", -1, false, false);

	// icons
	if (layer->type == SignalType::BEATS){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_time); // "⏱"
	}else if (layer->type == SignalType::MIDI){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_midi); // "♫"
	}else{
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_audio); // "∿"
	}

	color col_but = ColorInterpolate(view->colors.text, view->colors.hover, 0.3f);
	color col_but_hover = view->colors.text;

	if (visible and !layer->track->has_version_selection()){
		/*c->setColor(col_but);
		if ((view->hover.layer == layer) and (view->hover.type == Selection::Type::LAYER_BUTTON_DOMINANT))
			c->setColor(col_but_hover);
		//c->drawStr(area.x1 + 5, area.y1 + 22-2, "\U0001f50a"); // U+1F50A "🔊"
		c->drawMaskImage(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 22, *view->images.speaker);*/


		c->set_color(col_but);
		if ((view->hover.layer == layer) and (view->hover.type == Selection::Type::LAYER_BUTTON_SOLO))
			c->set_color(col_but_hover);
		//c->drawStr(area.x1 + 5 + 17, area.y1 + 22-2, "S");
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 22, area.y1 + 22, *view->images.solo);
	}

	if (visible and layer->is_main()){
		if (represents_imploded){
			c->set_color(col_but);
			if ((view->hover.layer == layer) and (view->hover.type == Selection::Type::LAYER_BUTTON_EXPLODE))
				c->set_color(col_but_hover);
			c->draw_str(area.x2 - view->LAYER_HANDLE_WIDTH + 22+17, area.y1 + 22, "+");
		}else{
			c->set_color(col_but);
			if ((view->hover.layer == layer) and (view->hover.type == Selection::Type::LAYER_BUTTON_IMPLODE))
				c->set_color(col_but_hover);
			c->draw_str(area.x2 - view->LAYER_HANDLE_WIDTH + 22+17, area.y1 + 22, "-");
		}
	}
}

void AudioViewLayer::set_solo(bool _solo)
{
	solo = _solo;
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}


void AudioViewLayer::draw(Painter *c)
{
	if (!represents_imploded)
		view->mode->draw_layer_data(c, this);

	if (layer->track->layers.num > 1)
		draw_version_header(c);
}



GridColors AudioViewLayer::grid_colors()
{
	GridColors g;
	g.bg = background_color();
	g.bg_sel = background_selection_color();
	g.fg = view->colors.grid;
	g.fg_sel = (view->sel.has(layer)) ? view->colors.grid_selected : view->colors.grid;
	return g;
}

bool AudioViewLayer::on_screen()
{
	if (hidden)
		return false;
	return (area.y1 < view->song_area.y2) and (area.y2 > view->song_area.y1);
}
