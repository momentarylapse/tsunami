/*
 * ViewModeDefault.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeDefault.h"
#include "../AudioView.h"
#include "../Node/AudioViewTrack.h"
#include "../Node/AudioViewLayer.h"
#include "../Node/SceneGraph.h"
#include "../MouseDelayPlanner.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"
#include "math.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../../Module/SignalChain.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/CrossFade.h"
#include "../../Data/Sample.h"
#include "../../Data/SampleRef.h"
#include "../Painter/BufferPainter.h"
#include "../Painter/GridPainter.h"
#include "../Painter/MidiPainter.h"
#include "../../lib/hui/Controls/Control.h"
#include "../../Action/Song/ActionSongMoveSelection.h"
#include "../../Device/Stream/AudioOutput.h"

float marker_alpha_factor(float w, float w_group, bool border);
Array<Array<TrackMarker*>> group_markers(const Array<TrackMarker*> &markers);



class MouseDelaySelect : public MouseDelayAction {
public:
	AudioView *view;
	SelectionMode mode;
	MouseDelaySelect(AudioView *v, SelectionMode _mode) {
		view = v;
		mode = _mode;
	}
	void on_start() override {
		view->hover().range.set_start(view->cur_selection.pos_snap);
		view->hover().range.set_end(view->get_mouse_pos());
		view->hover().y0 = view->mdp()->y0;
		view->hover().y1 = view->my;
		view->selection_mode = mode;
		view->hover().type = view->cur_selection.type = HoverData::Type::TIME; // ignore BAR_GAP!
		//view->hide_selection = (mode == SelectionMode::RECT);
		view->set_selection(view->mode->get_selection(view->hover().range, mode));
	}
	void on_update() override {
		// cheap auto scrolling
		if (view->mx < 50)
			view->cam.move(-10 / view->cam.scale);
		if (view->mx > view->area.width() - 50)
			view->cam.move(10 / view->cam.scale);

		view->hover().range.set_end(view->get_mouse_pos_snap());
		view->hover().y1 = view->my;
		if (view->select_xor)
			view->set_selection(view->sel_temp or view->mode->get_selection(view->hover().range, mode));
		else
			view->set_selection(view->mode->get_selection(view->hover().range, mode));
	}
	void on_draw_post(Painter *c) override {
		if (view->sel.range.length > 0){
			string s = view->song->get_time_str_long(view->sel.range.length);
			if (view->sel.range.length < 1000)
				s += format(_(" (%d samples)"), view->sel.range.length);
			if (view->sel.bars.num > 0)
				s = format(_("%d bars"), view->sel.bars.num) + ", " + s;
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
	void on_start() override {
		action = new ActionSongMoveSelection(view->song, sel);
	}
	void on_update() override {
		int p = view->get_mouse_pos() + (ref_pos - mouse_pos0);
		view->snap_to_grid(p);
		int dpos = p - mouse_pos0 - (ref_pos - mouse_pos0);
		action->set_param_and_notify(view->song, dpos);
	}
	void on_finish() override {
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
MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode) {
	return new MouseDelaySelect(v, mode);
}




ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
}

ViewModeDefault::~ViewModeDefault() {
}

void ViewModeDefault::left_click_handle(AudioViewLayer *vlayer) {


	if (view->select_xor) {
		// differential selection

		if (view->hover_any_object()) {
			left_click_handle_object_xor(vlayer);
		} else {
			left_click_handle_void_xor(vlayer);
		}
	} else {
		// normal click


		if (view->is_playback_active()) {
			view->playback_click();
		} else {


			// really normal click

			if (view->hover_any_object()) {
				left_click_handle_object(vlayer);
			} else {
				left_click_handle_void(vlayer);
			}
		}

	}
}

void ViewModeDefault::start_selection_rect(SelectionMode mode) {
	view->mdp_prepare(CreateMouseDelaySelect(view, mode));
}

void ViewModeDefault::left_click_handle_void(AudioViewLayer *vlayer) {
	view->set_current(view->hover());

	if (view->sel.has(vlayer->layer)) {
		// set cursor only when clicking on selected layers
		view->set_cursor_pos(hover().pos_snap);
	}

	view->exclusively_select_layer(vlayer);
	view->select_under_cursor();

	start_selection_rect(SelectionMode::TRACK_RECT);
}

void ViewModeDefault::left_click_handle_object(AudioViewLayer *vlayer) {
	view->exclusively_select_layer(vlayer);
	if (!view->hover_selected_object())
		view->exclusively_select_object();

	// start drag'n'drop?
	//if ((hover->type == Selection::Type::SAMPLE) or (hover->type == Selection::Type::MARKER) or (hover->type == Selection::Type::MIDI_NOTE)){
	view->mdp_prepare(CreateMouseDelayObjectsDnD(vlayer, view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES | SongSelection::Mask::MIDI_NOTES)));
		//}
}

void ViewModeDefault::left_click_handle_void_xor(AudioViewLayer *vlayer) {
	view->toggle_select_layer_with_content_in_cursor(vlayer);

	// diff selection rectangle
	start_selection_rect(SelectionMode::TRACK_RECT);
}

void ViewModeDefault::left_click_handle_object_xor(AudioViewLayer *vlayer) {
	view->toggle_object();
}



void ViewModeDefault::on_mouse_wheel() {
	auto e = hui::GetEvent();
	if (fabs(e->scroll_y) > 0.1f) {
		if (win->get_key(hui::KEY_CONTROL))
			cam->zoom(exp(e->scroll_y * view->mouse_wheel_speed * view->ZoomSpeed * 0.3f), view->mx);
		else
			cam->move(e->scroll_y * view->mouse_wheel_speed / cam->scale * view->ScrollSpeed * 0.03f);
	}
	if (fabs(e->scroll_x) > 0.1f)
		cam->move(e->scroll_x * view->mouse_wheel_speed / cam->scale * view->ScrollSpeed * 0.03f);
}

void playback_seek_relative(AudioView *view, float dt) {
	int pos = view->playback_pos();
	pos += dt * view->song->sample_rate;
	pos = max(pos, view->renderer->range().offset);
	view->signal_chain->set_pos(pos);
}

void ViewModeDefault::on_key_down(int k) {

// view
	// moving
	float dt = 0.05f;
	if (k == hui::KEY_RIGHT)
		cam->move(view->ScrollSpeed * dt / cam->scale);
	if (k == hui::KEY_LEFT)
		cam->move(- view->ScrollSpeed * dt / cam->scale);
	if (k == hui::KEY_NEXT)
		cam->move(view->ScrollSpeedFast * dt / cam->scale);
	if (k == hui::KEY_PRIOR)
		cam->move(- view->ScrollSpeedFast * dt / cam->scale);
	if (k == hui::KEY_HOME)
		view->set_cursor_pos(view->song->range_with_time().start());
	if (k == hui::KEY_END)
		view->set_cursor_pos(view->song->range_with_time().end());

	// zoom
	if (k == hui::KEY_ADD)
		cam->zoom(exp(  view->ZoomSpeed), view->mx);
	if (k == hui::KEY_SUBTRACT)
		cam->zoom(exp(- view->ZoomSpeed), view->mx);

	// playback
	if (view->is_playback_active()) {
		if (k == hui::KEY_CONTROL + hui::KEY_RIGHT)
			playback_seek_relative(view, 5);
		if (k == hui::KEY_CONTROL + hui::KEY_LEFT)
			playback_seek_relative(view, -5);
	}

	/*if (k == hui::KEY_ALT + hui::KEY_M)
		view->cur_track()->set_muted(!view->cur_track()->muted);
	if (k == hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_M)
		view->cur_layer()->set_muted(!view->cur_layer()->muted);
	if (k == hui::KEY_ALT + hui::KEY_S)
		view->cur_vtrack()->set_solo(!view->cur_vtrack()->solo);
	if (k == hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_S)
		view->cur_vlayer()->set_solo(!view->cur_vlayer()->solo);

	if (k == hui::KEY_ALT + hui::KEY_X) {
		if (view->cur_track()->layers.num > 0) {
			if (view->cur_vtrack()->imploded)
				view->explode_track(view->cur_track());
			else
				view->implode_track(view->cur_track());
		}
	}

	if (view->mode == view->mode_default) {
		if (k == hui::KEY_UP)
			move_to_layer(view, -1);
		if (k == hui::KEY_DOWN)
			move_to_layer(view, 1);
	}*/

	view->update_menu();
}

float ViewModeDefault::layer_min_height(AudioViewLayer *l) {
	if (l->layer->type == SignalType::MIDI)
		return view->TIME_SCALE_HEIGHT * 3;
	return view->TIME_SCALE_HEIGHT * 2;
}

float ViewModeDefault::layer_suggested_height(AudioViewLayer *l) {
	int n_ch = l->layer->channels;
	if (l->layer->is_main()) {
		if (l->layer->type == SignalType::AUDIO)
			return view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
		else if (l->layer->type == SignalType::MIDI)
			return view->MAX_TRACK_CHANNEL_HEIGHT * 2;
		else
			return view->TIME_SCALE_HEIGHT * 2;
	}
	return view->TIME_SCALE_HEIGHT * 2;
}

MidiPainter* midi_context(AudioViewLayer *l)
{
	auto *mp = l->view->midi_painter;
	mp->set_context(l->area, l->layer->track->instrument, l->is_playable(), l->midi_mode());
	mp->set_key_changes(l->midi_key_changes);
	mp->set_linear_range(l->edit_pitch_min, l->edit_pitch_max);
	return mp;
}

void ViewModeDefault::draw_layer_background(Painter *c, AudioViewLayer *l)
{
	view->grid_painter->set_context(l->area, l->grid_colors());
	view->grid_painter->draw_empty_background(c);
	view->grid_painter->draw_whatever(c, 0);


	if (l->layer->type == SignalType::MIDI){
		auto *mp = midi_context(l);
		mp->draw_background(c);
	}



	// parts
	auto groups = group_markers(l->layer->song()->get_parts());
	c->set_line_width(2.0f);
	for (auto &g: groups){
		float gx0, gx1;
		view->cam.range2screen(RangeTo(g[0]->range.start(), g.back()->range.end()), gx0, gx1);
		for (auto *m: g){
			color col = l->marker_color(m);
			col.a = 0.75f;
			float x0, x1;
			view->cam.range2screen(m->range, x0, x1);
			col.a *= marker_alpha_factor(x1 - x0, (x1-x0)*2, m == g[0]);
			c->set_color(col);
			c->draw_line(x0, l->area.y1, x0, l->area.y2);
		}
	}
	c->set_line_width(1.0f);
}

HoverData ViewModeDefault::get_hover_data(AudioViewLayer *vlayer, float mx, float my) {
	return vlayer->get_hover_data_default(mx, my);
}

SongSelection ViewModeDefault::get_selection_for_range(const Range &r) {
	return SongSelection::from_range(song, r).filter(view->sel.layers);
}

SongSelection ViewModeDefault::get_selection_for_rect(const Range &r, int y0, int y1)
{
	SongSelection s;
	s.range = r.canonical();
	if (y0 > y1){
		int t = y0;
		y0 = y1;
		y1 = t;
	}

	for (auto vt: view->vtrack){
		Track *t = vt->track;
		if ((y1 < vt->area.y1) or (y0 > vt->area.y2))
			continue;

		// markers
		for (TrackMarker *m: t->markers)
			s.set(m, s.range.overlaps(m->range));
	}

	for (auto vl: view->vlayer){
		TrackLayer *l = vl->layer;
		if ((y1 < vl->area.y1) or (y0 > vl->area.y2))
			continue;
		s.add(l);

		// subs
		for (SampleRef *sr: l->samples)
			s.set(sr, s.range.overlaps(sr->range()));

		// midi

		auto *mp = midi_context(vl);
		float r = mp->rr;
		for (MidiNote *n: l->midi)
			if ((n->y + r >= y0) and (n->y - r <= y1))
				//s.set(n, s.range.is_inside(n->range.center()));
				s.set(n, s.range.overlaps(n->range));
	}
	return s;
}

SongSelection ViewModeDefault::get_selection_for_track_rect(const Range &r, int y0, int y1)
{
	if (y0 > y1){
		int t = y0;
		y0 = y1;
		y1 = t;
	}
	Set<const TrackLayer*> _layers;
	for (auto vt: view->vlayer){
		if ((y1 >= vt->area.y1) and (y0 <= vt->area.y2))
			_layers.add(vt->layer);
	}
	return SongSelection::from_range(song, r).filter(_layers);
}

