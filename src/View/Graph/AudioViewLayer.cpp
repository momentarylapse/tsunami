/*
 * AudioViewLayer.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "../Graph/AudioViewLayer.h"

#include "../AudioView.h"
#include "../MouseDelayPlanner.h"
#include "../Mode/ViewMode.h"
#include "../Mode/ViewModeMidi.h"
#include "../Mode/ViewModeCurve.h"
#include "../Mode/ViewModeEdit.h"
#include "../Painter/BufferPainter.h"
#include "../Painter/GridPainter.h"
#include "../Painter/MidiPainter.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../EditModes.h"
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
#include "../Graph/AudioViewTrack.h"
#include "../Graph/LayerHeader.h"
#include "../Graph/ScrollBar.h"
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

class LayerScrollBar : public ScrollBar {
public:
	AudioViewLayer *vlayer;
	LayerScrollBar(AudioViewLayer *l) : ScrollBar(l->view) {
		vlayer = l;
		align.horizontal = AlignData::Mode::RIGHT;
		hidden = true;
	}
	HoverData get_hover_data(float mx, float my) override {
		auto h = ScrollBar::get_hover_data(mx, my);
		h.vlayer = vlayer;
		return h;
	}
};


AudioViewLayer::AudioViewLayer(AudioView *_view, TrackLayer *_layer) : scenegraph::NodeFree() {
	align.horizontal = AlignData::Mode::FILL;
	view = _view;
	layer = _layer;
	solo = false;
	align.dz = 2;

	edit_pitch_min = 55;
	edit_pitch_max = edit_pitch_min + PITCH_SHOW_COUNT;

	height = 0;

	represents_imploded = false;

	if (layer) {
		layer->subscribe(this, [=]{ on_layer_change(); }, layer->MESSAGE_CHANGE);
		layer->track->subscribe(this, [=]{ on_track_change(); }, layer->track->MESSAGE_CHANGE);
		layer->track->subscribe(this, [=]{ layer->track->unsubscribe(this); layer=nullptr; }, layer->track->MESSAGE_DELETE);
		update_midi_key_changes();
		header = new LayerHeader(this);
		add_child(header);
		scroll_bar = new LayerScrollBar(this);
		add_child(scroll_bar);
	}
}

AudioViewLayer::~AudioViewLayer() {
	if (layer)
		layer->track->unsubscribe(this);
}

void AudioViewLayer::on_layer_change() {
	view->renderer->allow_layers(view->get_playable_layers());
	notify();
}

void AudioViewLayer::on_track_change() {
	if (layer)
		update_midi_key_changes();

	//notify(MESSAGE_CHANGE);
}

Track *AudioViewLayer::track() {
	return layer->track;
}

AudioViewTrack *AudioViewLayer::vtrack() {
	return view->get_track(layer->track);
}

MidiMode AudioViewLayer::midi_mode() {
	return vtrack()->midi_mode();
}

Array<MidiKeyChange> get_key_changes(const TrackLayer *l) {
	Array<MidiKeyChange> key_changes;
	for (auto *m: l->markers_sorted())
		if (marker_is_key(m->text)) {
			MidiKeyChange c;
			c.pos = m->range.offset;
			c.key = parse_marker_key(m->text);
			key_changes.add(c);
		}
	return key_changes;
}

void AudioViewLayer::update_midi_key_changes() {
	if (layer)
		midi_key_changes = get_key_changes(layer);
}

color AudioViewLayer::marker_color(const TrackMarker *m) {
	return MidiPainter::pitch_color(m->text.hash() % MAX_PITCH);
}


void AudioViewLayer::draw_track_buffers(Painter *c) {
	view->buffer_painter->set_context(area);
	if (is_playable() and layer->track->has_version_selection()) {
		auto active_ranges = layer->active_version_ranges();
		auto inactive_ranges = layer->inactive_version_ranges();
		for (auto &b: layer->buffers) {
			view->buffer_painter->set_color(view->colors.text);
			for (Range &r: active_ranges) {
				view->buffer_painter->set_clip(r);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}
			view->buffer_painter->set_color(view->colors.text_soft3);
			for (Range &r: inactive_ranges) {
				view->buffer_painter->set_clip(r);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}
		}
	} else {
		view->buffer_painter->set_color(is_playable() ? view->colors.text : view->colors.text_soft3);
		for (auto &b: layer->buffers)
			view->buffer_painter->draw_buffer(c, b, b.offset);

	}

	if (view->sel.has(layer)) {
		// selection
		view->buffer_painter->set_color(view->colors.selection_boundary);
		view->buffer_painter->set_clip(view->cursor_range());
		for (AudioBuffer &b: layer->buffers)
			view->buffer_painter->draw_buffer_selection(c, b, b.offset);
	}
}

void AudioViewLayer::draw_sample_frame(Painter *c, SampleRef *s, const color &col, int delay) {
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


void AudioViewLayer::draw_sample(Painter *c, SampleRef *s) {
	color col = view->colors.sample;
	if (view->sel.has(s))
		col = view->colors.sample_selected;
	if (view->hover().sample == s)
		col = view->colors.hoverify(col);

	draw_sample_frame(c, s, col, 0);

	// buffer
	if (s->type() == SignalType::AUDIO) {
		view->buffer_painter->set_context(area);
		view->buffer_painter->set_color(col);
		view->buffer_painter->draw_buffer(c, s->buf(), s->pos);
	} else if (s->type() == SignalType::MIDI) {
		draw_midi(c, s->midi(), true, s->pos);
	}
}


void AudioViewLayer::draw_midi(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift) {
	view->midi_painter->set_context(area, layer->track->instrument, is_playable(), midi_mode());
	view->midi_painter->set_key_changes(midi_key_changes);
	view->midi_painter->set_quality(view->high_details ? 1.0f : 0.4f, view->antialiasing);
	view->midi_painter->set_shift(shift);
	if (view->editing_layer(this))
		view->midi_painter->set_linear_range(edit_pitch_min, edit_pitch_max);
	view->midi_painter->draw(c, midi);
}

bool can_merge(TrackMarker *a, TrackMarker *b) {
	if (!a)
		return false;
	if (abs(a->range.end() - b->range.start()) > 100)
		return false;
	return a->text == b->text;
}

Array<Array<TrackMarker*>> group_markers(const Array<TrackMarker*> &markers) {
	if (markers.num == 0)
		return {};
	Array<Array<TrackMarker*>> groups;
	Array<TrackMarker*> group;
	for (auto *m: markers) {
		if (group.num > 0) {
			if (can_merge(group.back(), m)) {
				group.add(m);
			} else {
				groups.add(group);
				group = {m}; // new group
			}
		} else {
			group.add(m);
		}
	}
	groups.add(group);
	return groups;
}

void AudioViewLayer::draw_markers(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover) {
	marker_areas.clear();
	marker_label_areas.clear();
	auto groups = group_markers(markers);
	for (auto &g: groups)
		draw_marker_group(c, g, hover);
}

void AudioViewLayer::draw_marker_group(Painter *c, const Array<TrackMarker*> &markers, HoverData &hover) {
	Range group_range = RangeTo(markers[0]->range.start(), markers.back()->range.end());
	foreachi(auto *m, markers, i)
		draw_marker(c, m, (hover.type == HoverData::Type::MARKER) and (hover.marker == m), group_range, i == 0, i == markers.num-1);
}

float marker_alpha_factor(float w, float w_group, bool border) {
	if (w == 0)
		return 0.5f;
	if (border)
		return clamp((w_group - 30) / 100, 0.0f, 1.0f);
	return clamp((w - 40) / 80, 0.0f, 1.0f);
}


void AudioViewLayer::draw_marker(Painter *c, const TrackMarker *marker, bool hover, const Range &group_range, bool first, bool last) {
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
	float frame_height = 5;

	w = max(w, x1 - x0);

	color col = view->colors.text;
	color col_bg = color::interpolate(view->colors.blob_bg_hidden, marker_color(marker), 0.6f);
	color col_frame = marker_color(marker);
	if (sel) {
		col = view->colors.text;
		//col_bg = view->colors.blob_bg_selected;//color::interpolate(col_bg, view->colors.selection, 0.6f);
		//col_frame = view->colors.blob_bg_selected;//color::interpolate(col_frame, view->colors.selection, 0.6f);
		col_bg = color::interpolate(col_bg, view->colors.blob_bg_selected, 0.8f);
		col_frame = color::interpolate(col_frame, view->colors.blob_bg_selected, 0.8f);
		//col_frame = view->colors.selection;
	}
	if (hover) {
		col = view->colors.hoverify(col);
		col_frame = view->colors.hoverify(col_frame);
		col_bg = view->colors.hoverify(col_bg);
	}

	bool allow_label = ((!merged or first) and (gx1-gx0) > 40);
	if (marker->range.empty())
		allow_label = (view->cam.dsample2screen(2000) > 1);
	float lw = c->get_str_width(text);
	float dx = max(view->CORNER_RADIUS, (x1-x0)/2 - lw/2);
	if (allow_label) {
		view->draw_boxed_str(c,  x0 + dx, y0 + frame_height, text, col, col_bg);
	}


	c->set_line_width(2.0f);

	// left line
	if (first) {
		color cl = col_frame;
		cl.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, true);
		c->set_color(cl);
		c->draw_line(x0, area.y1, x0, area.y2);
	}
	// right line
	color cr = col_frame;
	cr.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, last);
	c->set_color(cr);
	c->draw_line(x1, area.y1, x1, area.y2);
	c->set_line_width(1.0f);

	// top
	c->set_color(col_frame);
	c->draw_rect(x0, y0, x1-x0, frame_height);

	marker_areas.set(marker, rect(x0, x0 + w, y0, y0 + frame_height));
	marker_label_areas.set(marker, view->get_boxed_str_rect(c,  x0 + dx, y0 + 8, text));

	c->set_font("", view->FONT_SIZE, false, false);
}


void draw_flare(Painter *c, float x1, float x2, float y1, float y2, bool inwards, float flare_w) {
	int N = 7;
	float a1, a2;
	if (inwards) {
		a1 = 0.6f;
		a2 = 0.05f;
		x2 += flare_w;
	} else {
		a1 = 0.05f;
		a2 = 0.6f;
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

	/*if (index_own == 0 and l->layer->track->has_version_selection()){
		draw_fade_bg(c, l, view, -1);
	}*/

	c->set_line_width(2);
	for (auto &f: layer->fades) {
		float x1, x2;
		view->cam.range2screen(f.range(), x1, x2);
		c->set_color(color(1,0,0.7f,0));
		c->draw_line(x1, area.y1, x1, area.y2);
		c->draw_line(x2, area.y1, x2, area.y2);

		draw_flare(c, x1, x2, area.y1, area.y2, f.mode == f.INWARD, 50);
	}
	c->set_line_width(1);
}


void AudioViewLayer::set_edit_pitch_min_max(float _min, float _max) {
	float diff = _max - _min;
	edit_pitch_min = clamp(_min, 0.0f, MAX_PITCH - diff);
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
	if (session->in_mode(EditMode::EditTrack))
		return true;
	if (session->in_mode(EditMode::Capture))
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


void AudioViewLayer::on_draw(Painter *c) {
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
	for (auto *s: weak(layer->samples))
		draw_sample(c, s);

	draw_markers(c, layer->markers_sorted(), view->hover());

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
	return (area.y1 < view->song_area().y2) and (area.y2 > view->song_area().y1);
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
	if (view->mode == view->mode_edit)
		if (view->mode_edit->mode == view->mode_edit_midi)
			if (view->hover().type == HoverData::Type::MIDI_PITCH)
				if (view->mode_edit_midi->creation_mode != ViewModeMidi::CreationMode::SELECT)
					return true;
	if (view->mode == view->mode_curve)
		if (view->hover().type == HoverData::Type::CURVE_POINT)
			return true;
	if (view->hover_any_object())
		return true;
	return false;
}

bool AudioViewLayer::on_left_double_click() {
	auto &h = view->hover();
	int buffer_index = hover_buffer(h);

	if (h.sample) {
		view->sel = view->mode->get_selection_for_range(h.sample->range());
		view->update_selection();
	} else if (h.marker) {
		view->sel = view->mode->get_selection_for_range(h.marker->range);
		view->update_selection();
	} else if (buffer_index >= 0) {
		view->sel = view->mode->get_selection_for_range(layer->buffers[buffer_index].range());
		view->update_selection();
	} else if (h.bar) {
		view->sel = view->mode->get_selection_for_range(h.bar->range());
		view->update_selection();
	}
	return true;
}

bool AudioViewLayer::on_right_button_down() {
	auto &h = view->hover();
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
		if (!view->sel.range().is_inside(h.pos_snap))
			view->set_cursor_pos(h.pos_snap);
		view->sel.clear_data();
	}

	if (h.sample) {
		view->open_popup(view->menu_sample.get());
	} else if (h.bar) {
		view->open_popup(view->menu_bar.get());
	} else if (h.marker) {
		view->open_popup(view->menu_marker.get());
	} else if (h.type == HoverData::Type::BAR_GAP) {
		view->open_popup(view->menu_bar_gap.get());
	} else if (hover_buffer(h) >= 0) {
		view->open_popup(view->menu_buffer.get());
	} else { // void
		if (track()->layers.num == 1)
			view->open_popup(view->menu_track.get());
		else
			view->open_popup(view->menu_layer.get());
	}
	return true;
}

string AudioViewLayer::get_tip() {
	auto &h = view->hover();
	if (h.sample)
		return _("sample ") + h.sample->origin->name;
	if (h.marker)
		return  _("marker ") + h.marker->nice_text();
	if (h.bar) {
		if (h.bar->is_pause())
			return _("pause ") + view->song->get_time_str_long(h.bar->length);
		else
			return _("bar ") + h.bar->format_beats() + format(u8" \u2669=%.1f", h.bar->bpm(view->song->sample_rate));
	}
	if (h.type == HoverData::Type::BAR_GAP)
		return _("bar gap");
	if (h.note)
		return _("note ") + pitch_name(h.note->pitch) + format(" %.0f%%", h.note->volume * 100);
	return "";
}

HoverData AudioViewLayer::get_hover_data(float mx, float my) {
	return view->mode->get_hover_data(this, mx, my);
}

HoverData AudioViewLayer::get_hover_data_default(float mx, float my) {
	auto s = view->hover_time(mx, my);
	s.vlayer = this;
	s.node = this;
	s.vtrack = view->get_track(layer->track);
	s.type = HoverData::Type::LAYER;

	// markers
	for (int i=0; i<min(layer->markers.num, marker_areas.num); i++) {
		auto *m = layer->markers[i].get();
		if (marker_areas.contains(m) and marker_label_areas.contains(m))
			if (marker_areas[m].inside(mx, my) or marker_label_areas[m].inside(mx, my)) {
				s.marker = m;
				s.type = HoverData::Type::MARKER;
				return s;
			}
	}

	// TODO: prefer selected samples
	for (auto *ss: weak(layer->samples)) {
		int offset = view->mouse_over_sample(ss);
		if (offset >= 0) {
			s.sample = ss;
			s.type = HoverData::Type::SAMPLE;
			return s;
		}
	}

	// bars
	if (layer->track->type == SignalType::BEATS) {

		// bar gaps
		if (view->cam.dsample2screen(view->session->sample_rate()) > 20) {
			int offset = 0;
			for (int i=0; i<view->song->bars.num+1; i++) {
				float x = view->cam.sample2screen(offset);
				if (fabs(x - mx) < view->SNAPPING_DIST) {
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
		auto bars = view->song->bars.get_bars(RangeTo(s.pos, Range::END));
		for (auto *b: bars) {
			float x = view->cam.sample2screen(b->range().offset);
			// test for label area...
			if ((mx >= x) and (mx < x + 36) and (my < area.y1 + 20)) {
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
