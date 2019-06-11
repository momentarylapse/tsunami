/*
 * AudioViewLayer.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewLayer.h"
#include "LayerHeader.h"
#include "ScrollBar.h"
#include "../AudioView.h"
#include "../MouseDelayPlanner.h"
#include "../Mode/ViewMode.h"
#include "../Mode/ViewModeMidi.h"
#include "../Painter/BufferPainter.h"
#include "../Painter/GridPainter.h"
#include "../Painter/MidiPainter.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Sample.h"
#include "../../Data/CrossFade.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Rhythm/BarCollection.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Midi/Clef.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../Helper/SymbolRenderer.h"
#include "../Painter/MidiPainter.h"



const int PITCH_SHOW_COUNT = 30;



int hover_buffer(HoverData &hover) {
	if (!hover.layer())
		return -1;
	foreachi (auto &b, hover.layer()->buffers, i)
		if (b.range().is_inside(hover.pos))
			return i;
	return -1;
}


AudioViewLayer::AudioViewLayer(AudioView *_view, TrackLayer *_layer) : ViewNode(_view)//_view->scene_graph, 0, 0, 0, 0)
{
	layer = _layer;
	solo = false;
	z = 1;

	edit_pitch_min = 55;
	edit_pitch_max = edit_pitch_min + PITCH_SHOW_COUNT;

	height_min = height_wish = 0;

	represents_imploded = false;

	if (layer){
		set_midi_mode(view->midi_view_mode);
		layer->track->subscribe(this, [=]{ on_track_change(); }, layer->track->MESSAGE_CHANGE);
		layer->track->subscribe(this, [=]{ layer->track->unsubscribe(this); layer=nullptr; }, layer->track->MESSAGE_DELETE);
		header = new LayerHeader(this);
		children.add(header);
		scroll_bar = new ScrollBar(this);
		scroll_bar->align.align_right = true;
		scroll_bar->align.h = -1;
		scroll_bar->hidden = true;
		children.add(scroll_bar);
	}
}

AudioViewLayer::~AudioViewLayer()
{
	if (layer)
		layer->track->unsubscribe(this);
}

void AudioViewLayer::on_track_change()
{
	if (layer)
		update_midi_key_changes();

	//notify(MESSAGE_CHANGE);
}

Array<MidiKeyChange> get_key_changes(const Track *t)
{
	Array<MidiKeyChange> key_changes;
	for (auto *m: t->markers_sorted())
		if (marker_is_key(m->text)){
			MidiKeyChange c;
			c.pos = m->range.offset;
			c.key = parse_marker_key(m->text);
			key_changes.add(c);
		}
	return key_changes;
}

void AudioViewLayer::update_midi_key_changes()
{
	if (layer)
		midi_key_changes = get_key_changes(layer->track);
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
		view->buffer_painter->set_context(area);
		view->buffer_painter->set_color(col);
		view->buffer_painter->draw_buffer(c, s->buf(), s->pos);
	}else if (s->type() == SignalType::MIDI){
		draw_midi(c, s->midi(), true, s->pos);
	}
}


void AudioViewLayer::draw_midi(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift)
{
	view->midi_painter->set_context(area, layer->track->instrument, is_playable(), midi_mode);
	view->midi_painter->set_key_changes(midi_key_changes);
	view->midi_painter->set_quality(view->high_details ? 1.0f : 0.4f, view->antialiasing);
	view->midi_painter->set_shift(shift);
	if (view->editing_layer(this))
		view->midi_painter->set_linear_range(edit_pitch_min, edit_pitch_max);
	view->midi_painter->draw(c, midi);
}

bool can_merge(TrackMarker *a, TrackMarker *b)
{
	if (!a)
		return false;
	if (abs(a->range.end() - b->range.start()) > 100)
		return false;
	return a->text == b->text;
}

Array<Array<TrackMarker*>> group_markers(const Array<TrackMarker*> &markers)
{
	if (markers.num == 0)
		return {};
	Array<Array<TrackMarker*>> groups;
	Array<TrackMarker*> group;
	for (auto *m: markers){
		if (group.num > 0){
			if (can_merge(group.back(), m)){
				group.add(m);
			}else{
				groups.add(group);
				group = {m}; // new group
			}
		}else{
			group.add(m);
		}
	}
	groups.add(group);
	return groups;
}

void AudioViewLayer::draw_markers(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover)
{
	marker_areas.clear();
	marker_label_areas.clear();
	auto groups = group_markers(markers);
	for (auto &g: groups)
		draw_marker_group(c, g, hover);
}

void AudioViewLayer::draw_marker_group(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover)
{
	Range group_range = RangeTo(markers[0]->range.start(), markers.back()->range.end());
	foreachi(auto *m, markers, i)
		draw_marker(c, m, (hover.type == HoverData::Type::MARKER) and (hover.marker == m), group_range, i == 0, i == markers.num-1);
}

float marker_alpha_factor(float w, float w_group, bool border)
{
	if (w == 0)
		return 0.5f;
	if (border)
		return clampf((w_group - 30) / 100, 0, 1.0f);
	return clampf((w - 40) / 80, 0, 1.0f);
}


void AudioViewLayer::draw_marker(Painter *c, const TrackMarker *marker, bool hover, const Range &group_range, bool first, bool last)
{
	string text = marker->nice_text();
	if (marker->fx.num > 0)
		text += format(" (%d fx)", marker->fx.num);

	bool sel = view->sel.has(marker);

	if (sel)
		c->set_font("", -1, true, false);

	float w = c->get_str_width(text) + view->CORNER_RADIUS * 2;
	float x0, x1, gx0, gx1;
	view->cam.range2screen(group_range, gx0, gx1);
	float w_threshold = view->high_details ? 30 : 50;
	//if (gx1 - gx0 < w_threshold)
	//	return;
	view->cam.range2screen(marker->range, x0, x1);
	bool merged = (x1 - x0 < w_threshold);
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

	bool allow_label = ((!merged or first) and (gx1-gx0) > 40);
	if (marker->range.empty())
		allow_label = (view->cam.dsample2screen(2000) > 1);
	if (allow_label){
		view->draw_boxed_str(c,  x0 + view->CORNER_RADIUS, y0 + 10, text, col, col_bg);
	}


	c->set_line_width(2.0f);

	// left line
	if (first){
		color cl = col2;
		cl.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, true);
		c->set_color(cl);
		c->draw_line(x0, area.y1, x0, area.y2);
	}
	// right line
	color cr = col2;
	cr.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, last);
	c->set_color(cr);
	c->draw_line(x1, area.y1, x1, area.y2);
	c->set_line_width(1.0f);

	// top
	c->set_color(col2);
	c->draw_rect(x0, y0, x1-x0, y1-y0);

	marker_areas.set(marker, rect(x0, x0 + w, y0, y0 + 15));
	marker_label_areas.set(marker, view->get_boxed_str_rect(c,  x0 + view->CORNER_RADIUS, y0 + 10, text));

	c->set_font("", view->FONT_SIZE, false, false);
}

Range dominant_range(Track *t, int index)
{
	if (index == -1){
		return t->fades[0].range();
	}
	int start = t->fades[index].position;
	if (index + 1 < t->fades.num)
		return RangeTo(start, t->fades[index + 1].position + t->fades[index + 1].samples);
	return Range(start, t->fades[index].samples);
}

void draw_fade_bg(Painter *c, AudioViewLayer *l, AudioView *view, int i)
{
	Range r = dominant_range(l->layer->track, i);
	color cs = color(0.2f, 0,0.7f,0);
	float xx1, xx2;
	view->cam.range2screen(r, xx1, xx2);
	if (i == l->layer->track->fades.num - 1)
		xx2 += 50;
	if (i == -1)
		xx1 -= 50;
	float x1 = max(xx1, l->area.x1);
	float x2 = min(xx2, l->area.x2);
	c->set_color(cs);
	c->draw_rect(x1, l->area.y1, x2-x1, l->area.height());
	if (i == l->layer->track->fades.num - 1){
		cs.a *= 0.5f;
		c->set_color(cs);
		c->draw_rect(xx2, l->area.y1, 50, l->area.height());
	}else if (i == -1){
		cs.a *= 0.5f;
		c->set_color(cs);
		c->draw_rect(xx1 - 50, l->area.y1, 50, l->area.height());
	}
}

void draw_flare(Painter *c, float x1, float x2, float y1, float y2, bool inwards, float flare_w) {
	int N = 7;
	float a1, a2;
	if (inwards) {
		a1 = 0.3f;
		a2 = 0.05f;
		x2 += flare_w;
	} else {
		a1 = 0.05f;
		a2 = 0.3f;
		x1 -= flare_w;
	}
	for (int j=0; j<N; j++) {
		float t1 = (float)j / (float)N;
		float t2 = (float)(j+1) / (float)N;
		c->set_color(color(a1 + (a2-a1) * (t1+t2)/2,0,0.7f,0));
		c->draw_rect(rect(x1 + (x2-x1)*t1, x1 + (x2-x1)*t2, y1, y2));
	}

}

void AudioViewLayer::draw_fades(Painter *c) {
	int index_before = 0;
	int index_own = layer->version_number();

	/*if (index_own == 0 and l->layer->track->has_version_selection()){
		draw_fade_bg(c, l, view, -1);
	}*/

	c->set_line_width(2);
	foreachi (auto &f, layer->track->fades, i) {
		/*if (f.target == index_own){
			draw_fade_bg(c, l, view, i);
		}*/
		if (f.target == index_own or index_before == index_own) {
			float x1, x2;
			view->cam.range2screen(f.range(), x1, x2);
			c->set_color(color(1,0,0.7f,0));
			c->draw_line(x1, area.y1, x1, area.y2);
			c->draw_line(x2, area.y1, x2, area.y2);

			draw_flare(c, x1, x2, area.y1, area.y2, f.target == index_own, 20);
		}
		index_before = f.target;
	}
	c->set_line_width(1);
}


void AudioViewLayer::set_edit_pitch_min_max(int _min, int _max) {
	int diff = _max - _min;
	edit_pitch_min = clampi(_min, 0, MAX_PITCH - 1 - diff);
	edit_pitch_max = edit_pitch_min + diff;
	view->force_redraw();
}

bool AudioViewLayer::is_playable() {
	return view->get_playable_layers().contains(layer);
}

color AudioViewLayer::background_color() {
	return (view->sel.has(layer)) ? view->colors.background_track_selected : view->colors.background_track;
}

color AudioViewLayer::background_selection_color() {
	if (view->selection_mode == SelectionMode::RECT)
		return background_color(); // complex selection rect as overlay...
	return (view->sel.has(layer)) ? view->colors.background_track_selection : view->colors.background_track;
}

bool AudioView::editing_layer(AudioViewLayer *l) {
	if (cur_vlayer() != l)
		return false;
	if (session->in_mode("midi"))
		return true;
	if (session->in_mode("capture"))
		return true;
	return false;
}

void AudioViewLayer::set_solo(bool _solo) {
	solo = _solo;
	view->renderer->allow_layers(view->get_playable_layers());
	view->force_redraw();
	notify();
	view->notify(view->MESSAGE_SOLO_CHANGE);
}


void AudioViewLayer::draw(Painter *c) {
	if (represents_imploded)
		return;

	Track *t = layer->track;

	if (layer->type == SignalType::BEATS) {
		view->grid_painter->set_context(area, grid_colors());
		if (t->song->bars.num > 0)
			view->grid_painter->draw_bar_numbers(c);
	}


	// midi
	if (layer->type == SignalType::MIDI)
		draw_midi(c, layer->midi, false, 0);

	// audio buffer
	draw_track_buffers(c);

	// samples
	for (auto *s: layer->samples)
		draw_sample(c, s);

	if (layer->is_main())
		draw_markers(c, t->markers_sorted(), view->hover);

	draw_fades(c);
}



GridColors AudioViewLayer::grid_colors() {
	GridColors g;
	g.bg = background_color();
	g.bg_sel = background_selection_color();
	g.fg = view->colors.grid;
	g.fg_sel = (view->sel.has(layer)) ? view->colors.grid_selected : view->colors.grid;
	return g;
}

bool AudioViewLayer::on_screen() {
	if (hidden)
		return false;
	return (area.y1 < view->song_area.y2) and (area.y2 > view->song_area.y1);
}

void AudioViewLayer::update_header() {
	header->hidden = (layer->track->layers.num == 1);
	//header->children[0]->hidden = layer->track->has_version_selection();
}

bool AudioViewLayer::on_left_button_down() {
	view->sel_temp = view->sel; // for diff selection

	view->mode->left_click_handle(this);
	return true;
}

// TODO put somewhere more reasonable
bool AudioViewLayer::allow_handle_click_when_gaining_focus() {
	if (view->mode == view->mode_midi)
		if (view->hover.type == HoverData::Type::MIDI_PITCH)
			if (view->mode_midi->creation_mode != ViewModeMidi::CreationMode::SELECT)
				return true;
	if (view->hover_any_object())
		return true;
	return false;
}

bool AudioViewLayer::on_left_double_click() {
	//*hover = get_hover();
	//select_hover();

	int buffer_index = hover_buffer(view->hover);

	if (view->hover.sample){
		view->sel = view->mode->get_selection_for_range(view->hover.sample->range());
		view->update_selection();
	}else if (view->hover.marker){
		view->sel = view->mode->get_selection_for_range(view->hover.marker->range);
		view->update_selection();
	}else if (buffer_index >= 0){
		view->sel = view->mode->get_selection_for_range(layer->buffers[buffer_index].range());
		view->update_selection();
	}else if (view->hover.bar){
		view->sel = view->mode->get_selection_for_range(view->hover.bar->range());
		view->update_selection();
	}
	return true;
}

bool AudioViewLayer::on_right_button_down() {

	if (view->hover_any_object()) {
		if (!view->hover_selected_object()) {
			//select object exclusively
			view->sel.clear_data();

		}
		if (!view->sel.has(layer)) {
			view->exclusively_select_layer(this);
			view->sel.clear_data();
		}
		view->select_object();
	} else { // void
		if (!view->sel.has(layer))
			view->exclusively_select_layer(this);
		if (!view->sel.range.is_inside(view->hover.pos_snap))
			view->set_cursor_pos(view->hover.pos_snap);
		view->sel.clear_data();
	}

	if (view->hover.sample) {
		view->open_popup(view->menu_sample);
	} else if (view->hover.bar) {
		view->open_popup(view->menu_bar);
	} else if (view->hover.marker) {
		view->open_popup(view->menu_marker);
	} else if (view->hover.type == HoverData::Type::BAR_GAP) {
		view->open_popup(view->menu_bar_gap);
	} else if (hover_buffer(view->hover) >= 0) {
		view->open_popup(view->menu_buffer);
	} else { // void
		if (layer->is_main())
			view->open_popup(view->menu_track);
		else
			view->open_popup(view->menu_layer);
	}
	return true;
}

string AudioViewLayer::get_tip() {
	if (view->hover.sample)
		return _("sample ") + view->hover.sample->origin->name;
	if (view->hover.marker)
		return  _("marker ") + view->hover.marker->nice_text();
	if (view->hover.bar) {
		if (view->hover.bar->is_pause())
			return _("pause ") + view->song->get_time_str_long(view->hover.bar->length);
		else
			return _("bar ") + view->hover.bar->format_beats() + format(u8" \u2669=%.1f", view->hover.bar->bpm(view->song->sample_rate));
	}
	if (view->hover.type == HoverData::Type::BAR_GAP)
		return _("bar gap");
	return "";
}

HoverData AudioViewLayer::get_hover_data() {
	return view->mode->get_hover_data(this);
}

HoverData AudioViewLayer::get_hover_data_default() {
	HoverData s = ViewNode::get_hover_data();
	s.vlayer = this;
	foreachi(auto *l, view->vlayer, i)
		if (this == l)
			s.index = i;
	s.vtrack = view->get_track(layer->track);
	s.type = HoverData::Type::LAYER;

	// markers
	if (layer->is_main()){
		for (int i=0; i<min(layer->track->markers.num, marker_areas.num); i++){
			auto *m = layer->track->markers[i];
			if (marker_areas.contains(m) and marker_label_areas.contains(m))
			if (marker_areas[m].inside(view->mx, view->my) or marker_label_areas[m].inside(view->mx, view->my)){
				s.marker = m;
				s.type = HoverData::Type::MARKER;
				s.index = i;
				return s;
			}
		}
	}

	// TODO: prefer selected samples
	for (auto *ss: layer->samples){
		int offset = view->mouse_over_sample(ss);
		if (offset >= 0){
			s.sample = ss;
			s.type = HoverData::Type::SAMPLE;
			return s;
		}
	}

	// bars
	if (layer->track->type == SignalType::BEATS){

		// bar gaps
		if (view->cam.dsample2screen(view->session->sample_rate()) > 20){
			int offset = 0;
			for (int i=0; i<view->song->bars.num+1; i++){
				float x = view->cam.sample2screen(offset);
				if (fabs(x - view->mx) < view->SNAPPING_DIST){
					s.index = i;
					s.type = HoverData::Type::BAR_GAP;
					s.pos = offset;
					return s;
				}
				if (i < view->song->bars.num)
					offset += view->song->bars[i]->length;
			}
		}

		// bars
		auto bars = view->song->bars.get_bars(Range(s.pos, 1000000));
		for (auto *b: bars){
			float x = view->cam.sample2screen(b->range().offset);
			// test for label area...
			if ((view->mx >= x) and (view->mx < x + 36) and (view->my < area.y1 + 20)){
				//b.range.
				s.bar = b;
				s.index = b->index;
				s.type = HoverData::Type::BAR;
				return s;
			}
		}
	}

	return s;
}
