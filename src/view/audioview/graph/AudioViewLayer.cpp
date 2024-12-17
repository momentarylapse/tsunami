/*
 * AudioViewLayer.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewLayer.h"
#include "AudioViewTrack.h"
#include "LayerHeader.h"
#include "../AudioView.h"
#include "../../MouseDelayPlanner.h"
#include "../../ColorScheme.h"
#include "../../mode/ViewMode.h"
#include "../../mode/ViewModeEditMidi.h"
#include "../../mode/ViewModeCurve.h"
#include "../../mode/ViewModeEdit.h"
#include "../../painter/BufferPainter.h"
#include "../../painter/GridPainter.h"
#include "../../painter/MidiPainter.h"
#include "../../helper/graph/ScrollBar.h"
#include "../../helper/Drawing.h"
#include "../../../Tsunami.h"
#include "../../../EditModes.h"
#include "../../../data/base.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/TrackMarker.h"
#include "../../../data/SampleRef.h"
#include "../../../data/Sample.h"
#include "../../../data/CrossFade.h"
#include "../../../data/audio/AudioBuffer.h"
#include "../../../data/rhythm/Bar.h"
#include "../../../data/rhythm/BarCollection.h"
#include "../../../data/midi/MidiData.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/hui/language.h"

namespace tsunami {



const int PITCH_SHOW_COUNT = 30;

color hash_color(int h);



int hover_buffer(HoverData &hover) {
	if (!hover.vlayer)
		return -1;
	foreachi (auto &b, hover.layer()->buffers, i)
		if (b.range().is_inside(hover.pos))
			return i;
	return -1;
}

class LayerScrollBar : public ScrollBar {
public:
	AudioViewLayer *vlayer;
	LayerScrollBar(AudioViewLayer *l) {
		vlayer = l;
		align.horizontal = AlignData::Mode::Right;
		hidden = true;
	}
	HoverData get_hover_data(const vec2 &m) override {
		auto h = ScrollBar::get_hover_data(m);
		h.vlayer = vlayer;
		return h;
	}
};


AudioViewLayer::AudioViewLayer(AudioView *_view, TrackLayer *_layer) : scenegraph::NodeFree() {
	align.horizontal = AlignData::Mode::Fill;
	view = _view;
	layer = _layer;
	solo = false;
	align.dz = 2;

	set_perf_name("vlayer");

	edit_pitch_min = 55;
	edit_pitch_max = edit_pitch_min + PITCH_SHOW_COUNT;

	height = 0;

	represents_imploded = false;

	if (layer) {
		layer->out_changed >> create_sink([this] {
			on_layer_change();
		});
		layer->track->out_changed >> create_sink([this] {
			on_track_change();
		});
		layer->track->out_death >> create_sink([this] {
			layer->track->unsubscribe(this);
			layer = nullptr;
		});
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
	view->update_playback_layers();
}

void AudioViewLayer::on_track_change() {
	if (layer)
		update_midi_key_changes();

	//out_changed.notify();
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
	return hash_color(m->text.hash());
}


void AudioViewLayer::draw_track_buffers(Painter *c) {
	view->buffer_painter->set_context(area, vtrack()->audio_mode);

	//auto text_soft4 = color::interpolate(colors.text_soft3, colors.background, 0.3f);
	if (is_playable() and layer->track->has_version_selection()) {
		auto active_ranges = layer->active_version_ranges();
		auto inactive_ranges = layer->inactive_version_ranges();
		for (auto &b: layer->buffers) {
			view->buffer_painter->set_color(theme.text_soft1, background_color());
			for (Range &r: active_ranges) {
				view->buffer_painter->set_clip(r);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}
			view->buffer_painter->set_color(theme.text_soft3, background_color());
			for (Range &r: inactive_ranges) {
				view->buffer_painter->set_clip(r);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}
		}
	} else {
		if (is_playable())
			view->buffer_painter->set_color(theme.text_soft1, background_color());
		else
			view->buffer_painter->set_color(theme.text_soft3, background_color());
		for (auto &b: layer->buffers)
			view->buffer_painter->draw_buffer(c, b, b.offset);

	}

	if (view->sel.has(layer)) {
		// selection
		//view->buffer_painter->set_color(theme.selection_boundary, background_selection_color());
		//view->buffer_painter->set_color(theme.text_soft1, background_selection_color());
		view->buffer_painter->set_color(theme.text_soft1, theme.selection_boundary);//, background_selection_color());
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
	c->draw_rect(rect(asx, aex, area.y1,                             area.y1 + theme.SAMPLE_FRAME_HEIGHT));
	c->draw_rect(rect(asx, aex, area.y2 - theme.SAMPLE_FRAME_HEIGHT, area.y2));

	c->set_color(col);
	c->set_line_width(2);
	c->draw_line({asx, area.y1}, {asx, area.y2});
	c->draw_line({aex, area.y1}, {aex, area.y2});
	c->draw_line({asx, area.y1}, {aex, area.y1});
	c->draw_line({asx, area.y2}, {aex, area.y2});
}


void AudioViewLayer::draw_sample(Painter *c, SampleRef *s) {
	color col = theme.sample;
	if (view->sel.has(s))
		col = theme.sample_selected;
	if (view->hover().sample == s)
		col = theme.hoverify(col);

	draw_sample_frame(c, s, col, 0);

	// buffer
	if (s->type() == SignalType::Audio) {
		view->buffer_painter->set_context(area, vtrack()->audio_mode);
		view->buffer_painter->set_color(col, background_color());
		view->buffer_painter->draw_buffer(c, s->buf(), s->pos);
	} else if (s->type() == SignalType::Midi) {
		draw_midi(c, s->midi(), true, s->pos);
	}
}


void AudioViewLayer::draw_midi(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift) {
	auto mp = midi_context();//view->midi_painter.get();
	//mp->set_context(area, layer->track->instrument, is_playable(), midi_mode());
	//mp->set_key_changes(midi_key_changes);
	mp->set_quality(view->high_details ? 1.0f : 0.4f, view->antialiasing);
	mp->set_shift(shift);
	if (view->editing_layer(this)) {
		mp->set_linear_range(edit_pitch_min, edit_pitch_max);
		mp->set_force_shadows(true);
	}
	mp->draw(c, midi);
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
	Range group_range = Range::to(markers[0]->range.start(), markers.back()->range.end());
	foreachi(auto *m, markers, i)
		draw_marker(c, m, (hover.type == HoverData::Type::Marker) and (hover.marker == m), group_range, i == 0, i == markers.num-1);
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

	float w = c->get_str_width(text) + theme.CORNER_RADIUS * 2;
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

	color col = theme.text;
	color col_bg = color::interpolate(theme.blob_bg_hidden, marker_color(marker), 0.6f);
	color col_frame = marker_color(marker);
	if (sel) {
		col = theme.text;
		//col_bg = colors.blob_bg_selected;//color::interpolate(col_bg, colors.selection, 0.6f);
		//col_frame = colors.blob_bg_selected;//color::interpolate(col_frame, colors.selection, 0.6f);
		col_bg = color::interpolate(col_bg, theme.blob_bg_selected, 0.8f);
		col_frame = color::interpolate(col_frame, theme.blob_bg_selected, 0.8f);
		//col_frame = colors.selection;
	}
	if (hover) {
		col = theme.hoverify(col);
		col_frame = theme.hoverify(col_frame);
		col_bg = theme.hoverify(col_bg);
	}

	bool allow_label = ((!merged or first) and (gx1-gx0) > 40);
	if (marker->range.is_empty())
		allow_label = (view->cam.dsample2screen(2000) > 1);
	float lw = c->get_str_width(text);
	float dx = max(theme.CORNER_RADIUS, (x1-x0)/2 - lw/2);
	if (allow_label) {
		draw_boxed_str(c,  {x0 + dx, y0 + frame_height}, text, col, col_bg);
	}


	c->set_line_width(2.0f);

	// left line
	if (first) {
		color cl = col_frame;
		cl.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, true);
		c->set_color(cl);
		c->draw_line({x0, area.y1}, {x0, area.y2});
	}
	// right line
	color cr = col_frame;
	cr.a *= marker_alpha_factor(x1 - x0, gx1 - gx0, last);
	c->set_color(cr);
	c->draw_line({x1, area.y1}, {x1, area.y2});
	c->set_line_width(1.0f);

	// top
	c->set_color(col_frame);
	c->draw_rect(rect(x0, x1, y0, y0 + frame_height));

	marker_areas.set(marker, rect(x0, x0 + w, y0, y0 + frame_height));
	marker_label_areas.set(marker, get_boxed_str_rect(c,  {x0 + dx, y0 + 8}, text));

	c->set_font("", theme.FONT_SIZE, false, false);
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
		c->draw_line({x1, area.y1}, {x1, area.y2});
		c->draw_line({x2, area.y1}, {x2, area.y2});

		draw_flare(c, x1, x2, area.y1, area.y2, f.mode == f.Inward, 50);
	}
	c->set_line_width(1);
}


void AudioViewLayer::set_edit_pitch_min_max(float _min, float _max) {
	float diff = _max - _min;
	edit_pitch_min = clamp(_min, 0.0f, MaxPitch - diff);
	edit_pitch_max = edit_pitch_min + diff;
	request_redraw();
}

bool AudioViewLayer::is_playable() const {
	return view->get_playable_layers().contains(layer);
}

color AudioViewLayer::background_color() const {
	return (view->sel.has(layer)) ? theme.background_track_selected : theme.background_track;
}

color AudioViewLayer::background_selection_color() const {
	if (view->selection_mode == SelectionMode::Rect)
		return background_color(); // complex selection rect as overlay...
	return (view->sel.has(layer)) ? theme.background_track_selection : theme.background_track;
}

bool AudioView::editing_layer(AudioViewLayer *l) {
	if (cur_vlayer() != l)
		return false;
	if (in_mode(EditMode::EditTrack))
		return true;
	if (in_mode(EditMode::Capture))
		return true;
	return false;
}

void AudioViewLayer::set_solo(bool _solo) {
	solo = _solo;
	out_changed.notify();
	out_solo_changed.notify();
}


void AudioViewLayer::on_draw(Painter *c) {
	if (represents_imploded)
		return;
	//PerformanceMonitor::start_busy(perf_channel);

	Track *t = layer->track;

	if (layer->type == SignalType::Beats) {
		view->grid_painter->set_context(area, grid_colors());
		if (t->song->bars.num > 0)
			view->grid_painter->draw_bar_numbers(c);
	}


	auto clip_prev = c->clip();
	c->set_clip(area and c->area());

	// midi
	if (layer->type == SignalType::Midi)
		draw_midi(c, layer->midi, false, 0);

	// audio buffer
	draw_track_buffers(c);

	c->set_clip(clip_prev);

	// samples
	for (auto *s: weak(layer->samples))
		draw_sample(c, s);

	draw_markers(c, layer->markers_sorted(), view->hover());

	draw_fades(c);
	//PerformanceMonitor::end_busy(perf_channel);
}



GridColors AudioViewLayer::grid_colors() {
	GridColors g;
	g.bg = background_color();
	g.bg_sel = background_selection_color();
	g.fg = theme.grid;
	g.fg_sel = (view->sel.has(layer)) ? theme.grid_selected : theme.grid;
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

bool AudioViewLayer::on_left_button_down(const vec2 &m) {
	view->sel_temp = view->sel; // for diff selection

	view->mode->left_click_handle(this, m);
	return true;
}

// TODO put somewhere more reasonable
bool AudioViewLayer::allow_handle_click_when_gaining_focus() const {
	if (view->mode == view->mode_edit)
		if (view->mode_edit->mode == view->mode_edit_midi)
			if (view->hover().type == HoverData::Type::MidiPitch)
				if (view->mode_edit_midi->creation_mode != ViewModeEditMidi::CreationMode::Select)
					return true;
	if (view->mode == view->mode_curve)
		if (this == view->cur_vlayer())
			return true;
	if (view->hover_any_object())
		return true;
	return false;
}

bool AudioViewLayer::on_left_double_click(const vec2 &m) {
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

bool AudioViewLayer::on_right_button_down(const vec2 &m) {
	auto &h = view->hover();
	view->set_current(h);
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
	} else if (h.type == HoverData::Type::BarGap) {
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

string AudioViewLayer::get_tip() const {
	auto &h = view->hover();
	if (h.sample)
		return _("sample ") + h.sample->origin->name;
	if (h.marker)
		return _("marker ") + h.marker->nice_text();
	if (h.bar) {
		if (h.bar->is_pause())
			return _("pause ") + view->song->get_time_str_long(h.bar->length);
		else
			return _("bar ") + h.bar->format_beats() + format(u8" \u2669=%.1f", h.bar->bpm(view->song->sample_rate));
	}
	if (h.type == HoverData::Type::BarGap)
		return _("bar gap");
	if (h.note)
		return _("note ") + pitch_name(h.note->pitch) + format(" %.0f%%", h.note->volume * 100);
	return "";
}

HoverData AudioViewLayer::get_hover_data(const vec2 &m) {
	return view->mode->get_hover_data(this, m);
}

HoverData AudioViewLayer::get_hover_data_default(const vec2 &m) {
	auto s = view->hover_time(m);
	s.vlayer = this;
	s.node = this;
	s.type = HoverData::Type::Layer;

	// markers
	for (int i=0; i<min(layer->markers.num, marker_areas.num); i++) {
		auto *mm = layer->markers[i].get();
		if (marker_areas.contains(mm) and marker_label_areas.contains(mm))
			if (marker_areas[mm].inside(m) or marker_label_areas[mm].inside(m)) {
				s.marker = mm;
				s.type = HoverData::Type::Marker;
				return s;
			}
	}

	// TODO: prefer selected samples
	for (auto *ss: weak(layer->samples)) {
		int offset = view->mouse_over_sample(ss, m);
		if (offset >= 0) {
			s.sample = ss;
			s.type = HoverData::Type::Sample;
			return s;
		}
	}

	// bars
	if (layer->track->type == SignalType::Beats) {

		// bars
		auto bars = view->song->bars.get_bars(Range::to(s.pos, Range::END));
		for (auto *b: bars) {
			float x = view->cam.sample2screen(b->range().offset);
			// test for label area...
			if ((m.x >= x) and (m.x < x + 36) and (m.y < area.y1 + 20)) {
				//b.range.
				s.bar = b;
				s.index = b->index;
				s.type = HoverData::Type::Bar;
				return s;
			}
		}

		// bar gaps
		/*if (view->cam.dsample2screen(view->session->sample_rate()) > 20)*/ {
			int offset = 0;
			for (int i=0; i<view->song->bars.num+1; i++) {
				float x = view->cam.sample2screen(offset);
				if (fabs(x - m.x) < view->SNAPPING_DIST) {
					s.index = i;
					s.type = HoverData::Type::BarGap;
					s.pos = offset;
					return s;
				}
				if (i < view->song->bars.num)
					offset += view->song->bars[i]->length;
			}
		}
	}

	return s;
}

MidiPainter* AudioViewLayer::midi_context() {
	if (!_midi_painter)
		_midi_painter = new MidiPainter(track()->song, &view->cam, &view->sel, &view->hover(), theme);
	auto *mp = _midi_painter.get();
	mp->set_context(area, layer->track->instrument, is_playable(), midi_mode());
	mp->set_min_font_size(10);
	mp->set_key_changes(midi_key_changes);
	mp->set_linear_range(edit_pitch_min, edit_pitch_max);
	return mp;
}

}
