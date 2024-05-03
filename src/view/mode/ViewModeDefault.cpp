/*
 * ViewModeDefault.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeDefault.h"
#include "../audioview/AudioView.h"
#include "../helper/graph/SceneGraph.h"
#include "../painter/BufferPainter.h"
#include "../painter/GridPainter.h"
#include "../painter/MidiPainter.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../helper/graph/ScrollBar.h"
#include "../MouseDelayPlanner.h"
#include "../TsunamiWindow.h"
#include "../../module/audio/SongRenderer.h"
#include "../../module/SignalChain.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/CrossFade.h"
#include "../../data/Sample.h"
#include "../../data/SampleRef.h"
#include "../../lib/hui/Controls/Control.h"
#include "../../action/song/ActionSongMoveSelection.h"
#include "../../device/stream/AudioOutput.h"
#include "../../Session.h"
#include "../../Playback.h"
#include <math.h>

float marker_alpha_factor(float w, float w_group, bool border);
Array<Array<TrackMarker*>> group_markers(const Array<TrackMarker*> &markers);



class MouseDelaySelect : public MouseDelayAction {
public:
	AudioView *view;
	SelectionMode mode;
	Range range;
	int start_pos;
	bool keep_start;
	MouseDelaySelect(AudioView *v, SelectionMode _mode, bool _keep_start) {
		view = v;
		mode = _mode;
		keep_start = _keep_start;
		if (keep_start) {
			start_pos = view->sel.range_raw.start();
		} else {
			start_pos = view->get_mouse_pos_snap(view->cursor());
		}
	}
	void on_start(const vec2 &m) override {
		range = Range::to(start_pos, view->get_mouse_pos_snap(m));
		if (keep_start)
			view->hover().y0 = view->cur_vlayer()->area.center().y;
		else
			view->hover().y0 = view->mdp()->y0;
		view->hover().y1 = m.y;
		view->selection_mode = mode;
		view->hover().type = view->cur_selection.type = HoverData::Type::TIME; // ignore BAR_GAP!
		//view->hide_selection = (mode == SelectionMode::RECT);
		view->set_selection(view->mode->get_selection(range, mode));
	}
	void on_update(const vec2 &m) override {
		// cheap auto scrolling
		if (m.x < 50)
			view->cam.move(-10 / view->cam.pixels_per_sample);
		if (m.x > view->area.width() - 50)
			view->cam.move(10 / view->cam.pixels_per_sample);

		range.set_end(view->get_mouse_pos_snap(m));
		view->hover().y1 = m.y;
		if (view->selecting_xor())
			view->set_selection(view->sel_temp or view->mode->get_selection(range, mode));
		else
			view->set_selection(view->mode->get_selection(range, mode));
	}
	void on_draw_post(Painter *c) override {
		int l = view->cursor_range().length;
		if (l > 0){
			string s = view->song->get_time_str_long(l);
			if (l < 1000)
				s += format(_(" (%d samples)"), l);
			if (view->sel._bars.num > 0)
				s = format(_("%d bars"), view->sel._bars.num) + ", " + s;
			view->draw_cursor_hover(c, s);
		}
	}
	void on_clean_up() override {
		view->selection_mode = SelectionMode::NONE;
		//view->hide_selection = false;
	}
};

class MouseDelayObjectsDnD : public MouseDelayAction {
public:
	AudioViewLayer *layer;
	AudioView *view;
	SongSelection sel;
	ActionSongMoveSelection *action = nullptr;
	int mouse_pos0;
	int ref_pos;
	MouseDelayObjectsDnD(AudioViewLayer *l, const SongSelection &s) {
		layer = l;
		view = layer->view;
		sel = s;
//		view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES | SongSelection::Mask::MIDI_NOTES);
		mouse_pos0 = view->hover().pos;
		ref_pos = hover_reference_pos(view->hover());
	}
	void on_start(const vec2 &m) override {
		action = new ActionSongMoveSelection(view->song, sel, false);
	}
	void on_update(const vec2 &m) override {
		int p = view->get_mouse_pos(m) + (ref_pos - mouse_pos0);
		view->snap_to_grid(p);
		int dpos = p - mouse_pos0 - (ref_pos - mouse_pos0);
		action->set_param_and_notify(view->song, dpos);
	}
	void on_finish(const vec2 &m) override {
		view->song->execute(action);
	}
	void on_cancel() override {
		action->undo(view->song);
		delete action;
	}

	int hover_reference_pos(HoverData &s) {
		if (s.marker)
			return s.marker->range.offset;
		if (s.note)
			return s.note->range.offset;
		if (s.sample)
			return s.sample->pos;
		if (s.note)
			return s.note->range.offset;
		return s.pos;
	}
};


MouseDelayAction* CreateMouseDelayObjectsDnD(AudioViewLayer *l, const SongSelection &s) {
	return new MouseDelayObjectsDnD(l, s);
}
MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode, bool keep_start) {
	return new MouseDelaySelect(v, mode, keep_start);
}




ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view, "default")
{
}

void ViewModeDefault::left_click_handle(AudioViewLayer *vlayer, const vec2 &m) {
	if (view->selecting_xor()) {

		// differential selection
		if (view->hover_any_object()) {
			left_click_handle_object_xor(vlayer, m);
		} else {
			left_click_handle_void_xor(vlayer, m);
		}
	} else if (view->selecting_or()) {

		// extending selection
		if (view->hover_any_object()) {
			left_click_handle_object_or(vlayer, m);
		} else {
			left_click_handle_void_or(vlayer, m);
		}
	} else if (view->is_playback_active()) {

		view->playback_click();
	} else {

		// really normal click
		if (view->hover_any_object()) {
			left_click_handle_object(vlayer, m);
		} else {
			left_click_handle_void(vlayer, m);
		}
	}
}

void ViewModeDefault::start_selection_rect(SelectionMode mode, bool keep_start) {
	view->mdp_prepare(CreateMouseDelaySelect(view, mode, keep_start));
}

void ViewModeDefault::left_click_handle_void_or(AudioViewLayer *vlayer, const vec2 &m) {
	auto range = Range::to(view->sel.range_raw.start(), view->get_mouse_pos_snap(m));

	view->hover().y0 = view->cur_vlayer()->area.center().y;
	view->hover().y1 = m.y;
	view->selection_mode = SelectionMode::TRACK_RECT;
	view->hover().type = view->cur_selection.type = HoverData::Type::TIME; // ignore BAR_GAP!

	view->set_selection(view->mode->get_selection(range, SelectionMode::TRACK_RECT));
	start_selection_rect(SelectionMode::TRACK_RECT, true);
}

void ViewModeDefault::left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) {
	view->set_current(view->hover());

	if (view->sel.has(vlayer->layer)) {
		// set cursor only when clicking on selected layers
		view->set_cursor_pos(hover().pos_snap);
	}

	view->exclusively_select_layer(vlayer);
	view->select_under_cursor();

	start_selection_rect(SelectionMode::TRACK_RECT);
}

void ViewModeDefault::left_click_handle_object(AudioViewLayer *vlayer, const vec2 &m) {
	view->exclusively_select_layer(vlayer);
	if (!view->hover_selected_object()) {
		view->exclusively_select_object();
		view->set_current(hover());
	}

	// start drag'n'drop?
	//if ((hover->type == Selection::Type::SAMPLE) or (hover->type == Selection::Type::MARKER) or (hover->type == Selection::Type::MIDI_NOTE)){
	view->mdp_prepare(CreateMouseDelayObjectsDnD(vlayer, view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES | SongSelection::Mask::MIDI_NOTES)));
		//}
}

void ViewModeDefault::left_click_handle_void_xor(AudioViewLayer *vlayer, const vec2 &m) {
	view->toggle_select_layer_with_content_in_cursor(vlayer);

	// diff selection rectangle
	start_selection_rect(SelectionMode::TRACK_RECT);
}

void ViewModeDefault::left_click_handle_object_xor(AudioViewLayer *vlayer, const vec2 &m) {
	view->toggle_object();
}

void scroll_y(AudioView *view, float dy) {
	view->scroll_bar_y->move_view(dy);
	//if (view->scroll_bar_y->drag_update())
}


void ViewModeDefault::on_mouse_wheel(const vec2 &scroll) {
	if (fabs(scroll.y) > 0.1f) {
		if (win->get_key(hui::KEY_CONTROL)) {
			scroll_y(view, scroll.y * view->scroll_speed);
		} else if (win->get_key(hui::KEY_SHIFT)) {
			cam->move(scroll.y / cam->pixels_per_sample * view->scroll_speed);
		} else {
			cam->zoom(exp(scroll.y * view->zoom_speed), view->cursor().x);
		}
	}

	// horizontal scroll
	if (fabs(scroll.x) > 0.1f)
		cam->move(scroll.x / cam->pixels_per_sample * view->scroll_speed);
}

void expand_sel_range(AudioView *view, int pos) {
	view->sel.range_raw.set_end(pos);

	view->select_under_cursor();
	view->cam.make_sample_visible(pos, 0);
}

void ViewModeDefault::on_key_down(int k) {
}

static float _zoom_0_ = 1;

AudioViewLayer *first_selected_layer_in_block(AudioView *view, bool up) {
	auto *first_layer = view->cur_vlayer();
	while (true) {
		AudioViewLayer *v = nullptr;
		if (!up)
			v = view->next_layer(first_layer);
		else
			v = view->prev_layer(first_layer);
		if (v == first_layer or !view->sel.has(v->layer))
			return first_layer;
		first_layer = v;
	}
}

void expand_layer_selection(AudioView *view, bool up) {
	auto *vlayer = view->cur_vlayer();

	// block
	auto *first_layer = first_selected_layer_in_block(view, !up);
	auto *last_layer = first_selected_layer_in_block(view, up);

	if (vlayer == first_layer)
		first_layer = last_layer;

	if (!up)
		vlayer = view->next_layer(vlayer);
	else
		vlayer = view->prev_layer(vlayer);

	// select only between
	float y0 = min(vlayer->area.y1, first_layer->area.y1);
	float y1 = max(vlayer->area.y2, first_layer->area.y2);
	for (auto *l: view->vlayers)
		view->sel.set(l->layer, l->area.center().y > y0 and l->area.center().y < y1);

	view->set_current(vlayer->get_hover_data({0,0}));
	//exclusively_select_layer(vlayer);
	view->select_under_cursor();

}

void ViewModeDefault::on_command(const string &id) {

	if (view->mode == view->mode_default) {
		if (id == "layer-up")
			view->move_to_layer(-1);
		if (id == "layer-down")
			view->move_to_layer(1);
		if (id == "layer-expand-up")
			expand_layer_selection(view, true);
		if (id == "layer-expand-down")
			expand_layer_selection(view, false);
	}

	// playback
	if (view->is_playback_active()) {
		if (id == "cursor-move-right")
			session->playback->seek_relative(5);
		if (id == "cursor-move-left")
			session->playback->seek_relative(-5);
	} else {
		if (id == "cursor-move-right")
			view->set_cursor_pos(suggest_move_cursor(view->cursor_range(), true));
		if (id == "cursor-move-left")
			view->set_cursor_pos(suggest_move_cursor(view->cursor_range(), false));
		if (id == "cursor-expand-right")
			expand_sel_range(view, suggest_move_cursor(Range(view->sel.range_raw.end(), 0), true));
		if (id == "cursor-expand-left")
			expand_sel_range(view, suggest_move_cursor(Range(view->sel.range_raw.end(), 0), false));
		if (id == "cursor-jump-start")
			view->set_cursor_pos(song->range().start());
		if (id == "cursor-jump-end")
			view->set_cursor_pos(song->range().end());
		if (id == "cursor-expand-start")
			expand_sel_range(view, song->range().start());
		if (id == "cursor-expand-end")
			expand_sel_range(view, song->range().end());
	}
}

void ViewModeDefault::on_gesture(const string &id, const vec2 &m, const vec2 &param) {
	if (id == hui::EventID::GESTURE_ZOOM_BEGIN) {
		_zoom_0_ = 1;//view->cam.scale;
	}
	if (id == hui::EventID::GESTURE_ZOOM_END) {
	}
	if (id == hui::EventID::GESTURE_ZOOM) {
		view->cam.zoom(hui::get_event()->scroll.x/_zoom_0_, m.x);
		_zoom_0_ = hui::get_event()->scroll.x;
	}
}

float ViewModeDefault::layer_suggested_height(AudioViewLayer *l) {
	int n_ch = l->layer->channels;
	float scale = 1.0f;
	if (l->track()->layers.num > 1)
		scale = 0.7f;
	if (l->layer->type == SignalType::AUDIO)
		return theme.MAX_TRACK_CHANNEL_HEIGHT * n_ch * scale;
	else if (l->layer->type == SignalType::MIDI)
		return theme.MAX_TRACK_CHANNEL_HEIGHT * 2 * scale;
	else
		return theme.TIME_SCALE_HEIGHT * 2;
}

Bar *song_bar_at(Song *s, int pos);

int ViewModeDefault::suggest_move_cursor(const Range &cursor, bool forward) {
	int PIXELS = 30;

	int pos = cursor.start();
	if (forward)
		pos = cursor.end();

	if (cursor.length > 0)
		return pos;

	Bar *b = song_bar_at(view->song, pos);
	if (!forward)
		b = song_bar_at(view->song, pos - 1);
	if (b) {
		// 1 bar
		if (view->cam.dsample2screen(b->length) < PIXELS * 1.2) {
			if (forward)
				return b->range().end();
			else
				return b->range().start();
		}

		// 1 beat
		int pp = pos;
		if (forward)
			pp = view->song->bars.get_next_beat(pos);
		else
			pp = view->song->bars.get_prev_beat(pos);
		if (view->cam.dsample2screen(fabs(pp - pos)) < PIXELS * 1.5)
			return pp;
	}

	// N pixels
	int d = view->cam.dscreen2sample(PIXELS);

	// 1 sample
	if (d < 10)
		d = 1;

	if (!forward)
		d = -d;
	return pos + d;
}

void ViewModeDefault::draw_selected_layer_highlight(Painter *p, const rect &area) {
	float d = 12;
	p->set_color(theme.text.with_alpha(0.1f));
	p->draw_rect(rect(view->song_area().x1, area.x2, area.y1-d, area.y1));
	p->draw_rect(rect(view->song_area().x1, area.x2, area.y2, area.y2+d));
	d = 2;
	p->set_color(theme.text.with_alpha(0.7f));
	p->draw_rect(rect(view->song_area().x1, area.x2, area.y1-d, area.y1));
	p->draw_rect(rect(view->song_area().x1, area.x2, area.y2, area.y2+d));
}

void ViewModeDefault::draw_post(Painter *p) {
	if (session->mode.match("default/track*"))
		draw_selected_layer_highlight(p, view->cur_vtrack()->area);
}

void ViewModeDefault::draw_layer_background(Painter *c, AudioViewLayer *l) {
	view->grid_painter->set_context(l->area, l->grid_colors());
	view->grid_painter->draw_empty_background(c);
	view->grid_painter->draw_whatever(c, 0);


	if (l->layer->type == SignalType::MIDI) {
		auto *mp = l->midi_context();
		mp->draw_background(c);
	}



	// parts
	auto groups = group_markers(l->layer->song()->get_parts());
	c->set_line_width(2.0f);
	for (auto &g: groups) {
		float gx0, gx1;
		view->cam.range2screen(Range::to(g[0]->range.start(), g.back()->range.end()), gx0, gx1);
		for (auto *m: g) {
			color col = l->marker_color(m);
			col.a = 0.75f;
			float x0, x1;
			view->cam.range2screen(m->range, x0, x1);
			col.a *= marker_alpha_factor(x1 - x0, (x1-x0)*2, m == g[0]);
			c->set_color(col);
			c->draw_line({x0, l->area.y1}, {x0, l->area.y2});
		}
	}
	c->set_line_width(1.0f);
}

HoverData ViewModeDefault::get_hover_data(AudioViewLayer *vlayer, const vec2 &m) {
	return vlayer->get_hover_data_default(m);
}

SongSelection ViewModeDefault::get_selection_for_range(const Range &r) {
	return SongSelection::from_range(song, r).filter(view->sel.layers());
}

SongSelection ViewModeDefault::get_selection_for_rect(const Range &_r, int y0, int y1) {
	SongSelection s;
	s.range_raw = _r;
	Range r = _r.canonical();
	if (y0 > y1) {
		int t = y0;
		y0 = y1;
		y1 = t;
	}

	for (auto vl: view->vlayers) {
		TrackLayer *l = vl->layer;
		if ((y1 < vl->area.y1) or (y0 > vl->area.y2))
			continue;
		s.add(l);

		// samples
		for (SampleRef *sr: weak(l->samples))
			s.set(sr, r.overlaps(sr->range()));
			

		// markers
		for (TrackMarker *m: weak(l->markers))
			s.set(m, r.overlaps(m->range));

		// midi

		auto *mp = vl->midi_context();
		float d = mp->note_r();
		for (MidiNote *n: weak(l->midi))
			if ((n->y + d >= y0) and (n->y - d <= y1))
				//s.set(n, s.range.is_inside(n->range.center()));
				s.set(n, r.overlaps(n->range));
	}
	return s;
}

SongSelection ViewModeDefault::get_selection_for_track_rect(const Range &r, int y0, int y1) {
	if (y0 > y1) {
		int t = y0;
		y0 = y1;
		y1 = t;
	}
	base::set<const TrackLayer*> _layers;
	for (auto vt: view->vlayers) {
		if ((y1 >= vt->area.y1) and (y0 <= vt->area.y2))
			_layers.add(vt->layer);
	}
	return SongSelection::from_range(song, r).filter(_layers);
}

