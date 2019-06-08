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
#include "../../TsunamiWindow.h"
#include "../../Session.h"
#include "../../Action/Song/ActionSongMoveSelection.h"
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
#include "../../Stream/AudioOutput.h"

float marker_alpha_factor(float w, float w_group, bool border);
Array<Array<TrackMarker*>> group_markers(const Array<TrackMarker*> &markers);

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = nullptr;
	moving_track = nullptr;
	dnd_selection = new SongSelection;
	dnd_ref_pos = 0;
	dnd_mouse_pos0 = 0;
}

ViewModeDefault::~ViewModeDefault() {
	delete dnd_selection;
}

bool ViewModeDefault::left_click_handle_special() {

	// special actions
	if (hover->type == Selection::Type::SELECTION_END){
		hover->range = view->sel.range;
		view->selection_mode = view->SelectionMode::TIME;
		return true;
	}else if (hover->type == Selection::Type::SELECTION_START){
		// swap end / start
		hover->type = Selection::Type::SELECTION_END;
		hover->range = view->sel.range;
		hover->range.invert();
		view->selection_mode = view->SelectionMode::TIME;
		return true;
	}
	return false;
}

void ViewModeDefault::left_click_handle() {

	if (view->is_playback_active()) {
		if (view->renderer->range().is_inside(hover->pos)) {
			session->signal_chain->set_pos(hover->pos);
			hover->type = Selection::Type::PLAYBACK_CURSOR;
			view->force_redraw();
			return;
		} else {
			view->stop();
		}
	} else {
		// "void"
		left_click_handle_void();
	}
}

void ViewModeDefault::left_click_handle_void() {
	view->set_cur_sample(nullptr);

	if (view->sel.has(hover->layer)) {
		// set cursor only when clicking on selected layers
		view->snap_to_grid(hover->pos);
		view->set_cursor_pos(hover->pos);
	}

	view->exclusively_select_layer();
	view->select_under_cursor();

	// start drag'n'drop?
	view->msp.start(hover->pos, hover->y0);

}

void ViewModeDefault::left_click_handle_xor() {
	view->toggle_select_layer_with_content_in_cursor();

	// diff selection rectangle
	view->msp.start(hover->pos, hover->y0);
}

void ViewModeDefault::on_left_button_down() {
	*hover = get_hover();
	view->sel_temp = view->sel; // for diff selection

	if (left_click_handle_special())
		return;


	if (view->select_xor) {
		// differential selection

		if (view->hover_any_object()) {
			view->toggle_object();
		} else {
			left_click_handle_xor();
		}
	} else {
		// normal click

		if (view->hover_any_object()) {

			view->exclusively_select_layer();
			if (!view->hover_selected_object())
				view->exclusively_select_object();

			// start drag'n'drop?
			//if ((hover->type == Selection::Type::SAMPLE) or (hover->type == Selection::Type::MARKER) or (hover->type == Selection::Type::MIDI_NOTE)){
				dnd_start_soon(view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES | SongSelection::Mask::MIDI_NOTES));
				//}
		} else {
			left_click_handle();
		}
	}
}

int hover_reference_pos(Selection &s)
{
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

void ViewModeDefault::dnd_start_soon(const SongSelection &sel)
{
	*dnd_selection = sel;
	dnd_mouse_pos0 = view->hover.pos;
	dnd_ref_pos = hover_reference_pos(view->hover);
	cur_action = new ActionSongMoveSelection(view->song, sel);
}

void ViewModeDefault::dnd_stop()
{
	song->execute(cur_action);
	cur_action = nullptr;
}

void ViewModeDefault::on_left_button_up()
{
	if (cur_action)
		dnd_stop();

	if (moving_track){
		int target = get_track_move_target(false);
		moving_track->move(target);
		moving_track = nullptr;
	}

	view->selection_mode = view->SelectionMode::NONE;
	view->msp.stop();
}

int hover_buffer(Selection *hover)
{
	if (!hover->layer)
		return -1;
	foreachi (auto &b, hover->layer->buffers, i)
		if (b.range().is_inside(hover->pos))
			return i;
	return -1;
}

void ViewModeDefault::on_left_double_click()
{
	if (view->selection_mode != view->SelectionMode::NONE)
		return;

	*hover = get_hover();
	select_hover();

	int buffer_index = hover_buffer(hover);

	if (hover->type == Selection::Type::SAMPLE){
		view->sel = get_selection_for_range(view->cur_sample->range());
		view->update_selection();
	}else if (hover->type == Selection::Type::MARKER){
		view->sel = get_selection_for_range(hover->marker->range);
		view->update_selection();
	}else if ((hover->type == Selection::Type::LAYER) and (buffer_index >= 0)){
		view->sel = get_selection_for_range(hover->layer->buffers[buffer_index].range());
		view->update_selection();
	}else if (hover->type == Selection::Type::BAR){
		view->sel = get_selection_for_range(hover->bar->range());
		view->update_selection();
	}
}


void ViewModeDefault::on_right_button_down()
{
	*hover = get_hover();
	bool track_hover_sel = view->sel.has(hover->track);

	select_hover();

	// click outside sel.range -> select new position
	if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::TIME) or (hover->type == Selection::Type::BACKGROUND)){
		if (!view->sel.range.is_inside(hover->pos)){
			view->snap_to_grid(hover->pos);
			set_cursor_pos(hover->pos, track_hover_sel);
		}
	}
	view->hover_before_leave = *hover;

	// pop up menu...
	view->update_menu();

	if (hover->type == Selection::Type::SAMPLE){
		view->open_popup(view->menu_sample);
	}else if (hover->type == Selection::Type::BAR){
		view->open_popup(view->menu_bar);
	}else if (hover->type == Selection::Type::MARKER){
		view->open_popup(view->menu_marker);
	}else if (hover->type == Selection::Type::BAR_GAP){
		view->open_popup(view->menu_bar_gap);
	}else if (hover_buffer(hover) >= 0){
		view->open_popup(view->menu_buffer);
	}else if (hover->type == Selection::Type::LAYER_HEADER){
		view->open_popup(view->menu_layer);
	}else if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::TRACK_HEADER)){
		view->open_popup(view->menu_track);
	}else  if ((hover->type == Selection::Type::SELECTION_START) or (hover->type == Selection::Type::SELECTION_END)){
	}else if (!hover->track){
		view->menu_song->open_popup(view->win);
	}
}

void ViewModeDefault::on_mouse_move()
{
	bool _force_redraw_ = false;

	auto e = hui::GetEvent();

	if (e->lbut){

		// cheap auto scrolling
		if (hover->allow_auto_scroll() or (view->selection_mode != view->SelectionMode::NONE)){
			if (view->mx < 50)
				cam->move(-10 / cam->scale);
			if (view->mx > view->area.width() - 50)
				cam->move(10 / cam->scale);
		}

		view->selection_update_pos(view->hover);
	}else{
		//Selection hover_old = *hover;
		*hover = get_hover();
		/*if (hover_changed(*hover, hover_old))
			view->forceRedraw();*/
		return;
	}


	// drag & drop
	if (view->selection_mode == view->SelectionMode::TIME){

		/*Selection mo = getHover();
		if (mo.track)
			view->sel.add(mo.track);*/

		//_force_redraw_ = true;
		/*_force_redraw_ = false;
		int x, w;
		int r = 4;
		if (e->dx < 0){
			x = view->mx - r;
			w = - e->dx + 2*r;
		}else{
			x = view->mx + r;
			w = - e->dx - 2*r;
		}
		win->redrawRect("area", x, view->area.y1, w, view->area.height());*/
	}else if (hover->type == Selection::Type::PLAYBACK_CURSOR){
		view->renderer->set_pos(hover->pos);
		_force_redraw_ = true;
	}else if (hover->type == Selection::Type::SAMPLE){
		/*view->applyBarriers(hover->pos);
		int dpos = (float)hover->pos - hover->sample_offset - hover->sample->pos;
		if (cur_action)
			cur_action->set_param_and_notify(view->song, dpos);
		_force_redraw_ = true;*/
	}

	if (cur_action){
		int p = hover->pos + (dnd_ref_pos - dnd_mouse_pos0);
		view->snap_to_grid(p);
		int dpos = p - dnd_mouse_pos0 - (dnd_ref_pos - dnd_mouse_pos0);
		cur_action->set_param_and_notify(view->song, dpos);
		_force_redraw_ = true;

	}

	if (_force_redraw_)
		view->force_redraw();
}

void ViewModeDefault::on_mouse_wheel()
{
	auto e = hui::GetEvent();
	if (fabs(e->scroll_y) > 0.1f){
		if (win->get_key(hui::KEY_CONTROL))
			cam->zoom(exp(e->scroll_y * view->mouse_wheel_speed * view->ZoomSpeed * 0.3f), view->mx);
		else
			cam->move(e->scroll_y * view->mouse_wheel_speed / cam->scale * view->ScrollSpeed * 0.03f);
	}
	if (fabs(e->scroll_x) > 0.1f)
		cam->move(e->scroll_x * view->mouse_wheel_speed / cam->scale * view->ScrollSpeed * 0.03f);
}

void playback_seek_relative(AudioView *view, float dt)
{
	int pos = view->playback_pos();
	pos += dt * view->song->sample_rate;
	pos = max(pos, view->renderer->range().offset);
	view->signal_chain->set_pos(pos);
}

void ViewModeDefault::on_key_down(int k)
{

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
	if (view->is_playback_active()){
		if (k == hui::KEY_CONTROL + hui::KEY_RIGHT)
			playback_seek_relative(view, 5);
		if (k == hui::KEY_CONTROL + hui::KEY_LEFT)
			playback_seek_relative(view, -5);
	}

	// action
	if (k == hui::KEY_ESCAPE){
		if (cur_action)
			dnd_stop();
	}

	view->update_menu();
}

float ViewModeDefault::layer_min_height(AudioViewLayer *l)
{
	if (l->layer->type == SignalType::MIDI)
		return view->TIME_SCALE_HEIGHT * 3;
	return view->TIME_SCALE_HEIGHT * 2;
}

float ViewModeDefault::layer_suggested_height(AudioViewLayer *l)
{
	int n_ch = l->layer->channels;
	if (l->layer->is_main()){
		if (l->layer->type == SignalType::AUDIO)
			return view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
		else if (l->layer->type == SignalType::MIDI)
			return view->MAX_TRACK_CHANNEL_HEIGHT * 2;
		else
			return view->TIME_SCALE_HEIGHT * 2;
	}
	return view->TIME_SCALE_HEIGHT * 2;
}


void ViewModeDefault::draw_layer_background(Painter *c, AudioViewLayer *l)
{
	view->grid_painter->set_context(l->area, l->grid_colors());
	view->grid_painter->draw_empty_background(c);
	view->grid_painter->draw_whatever(c, 0);


	if (l->layer->type == SignalType::MIDI){
		view->midi_painter->set_context(l->area, l->layer->track->instrument, l->is_playable(), l->midi_mode);
		view->midi_painter->set_key_changes(l->midi_key_changes);
		view->midi_painter->draw_background(c);
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

int ViewModeDefault::get_track_move_target(bool visual)
{
	int orig = get_track_index(moving_track);
	foreachi(auto vt, view->vtrack, i){
		int y = (vt->area.y1 + vt->area.y2) / 2;
		if (y > view->my){
			if (visual or (i <= orig))
				return i;
			else
				return i - 1;
		}
	}
	return visual ? song->tracks.num : (song->tracks.num-1);
}

void ViewModeDefault::draw_post(Painter *c)
{
	if (moving_track){
		//int orig = get_track_index(moving_track);
		int t = get_track_move_target(true);
		int y = view->vtrack.back()->area.y2;
		if (t < view->vtrack.num)
			y = view->vtrack[t]->area.y1;

		c->set_color(view->colors.selection_boundary);
		c->set_line_width(2.0f);
		c->draw_line(view->area.x1,  y,  view->area.x2,  y);
		c->set_line_width(1.0f);

		/*c->setColor(view->colors.selection_internal);
		rect r = view->vtrack[orig]->area;
		r.x2 = view->TRACK_HANDLE_WIDTH;
		c->drawRect(r);*/

		view->draw_cursor_hover(c, moving_track->nice_name());
	}

	if (hover->type == Selection::Type::SAMPLE) {
		view->draw_cursor_hover(c, _("sample ") + hover->sample->origin->name);
	} else if (hover->type == Selection::Type::MARKER) {
		view->draw_cursor_hover(c, _("marker ") + hover->marker->nice_text());
	} else if (hover->type == Selection::Type::BAR) {
		if (hover->bar->is_pause())
			view->draw_cursor_hover(c, _("pause ") + song->get_time_str_long(hover->bar->length));
		else
			view->draw_cursor_hover(c, _("bar ") + hover->bar->format_beats() + format(u8" \u2669=%.1f", hover->bar->bpm(song->sample_rate)));
	} else if (hover->type == Selection::Type::BAR_GAP) {
		view->draw_cursor_hover(c, _("bar gap"));
	}

	if (view->selection_mode != view->SelectionMode::NONE){
		if (view->sel.range.length > 0){
			string s = view->song->get_time_str_long(view->sel.range.length);
			if (view->sel.bars.num > 0)
				s = format(_("%d bars"), view->sel.bars.num) + ", " + s;
			view->draw_cursor_hover(c, s);
		}
	}

}

Selection ViewModeDefault::get_hover_basic(bool editable)
{
	Selection s;
	int mx = view->mx;
	int my = view->my;
	s.pos = view->cam.screen2sample(mx);
	s.range = Range(s.pos, 0);
	s.y0 = s.y1 = my;
	s.type = s.Type::BACKGROUND;

	// layer?
	foreachi(auto *l, view->vlayer, i){
		if (l->hover()){
			s.vlayer = l;
			s.index = i;
			s.layer = l->layer;
			s.track = l->layer->track;
			s.vtrack = view->get_track(s.track);
			s.type = Selection::Type::LAYER;
			if (l->layer->is_main())
				if ((view->mx < l->area.x1 + view->TRACK_HANDLE_WIDTH) and (view->my < l->area.y1 + view->TRACK_HANDLE_HEIGHT))
					s.type = Selection::Type::TRACK_HEADER;
			if (l->layer->track->layers.num > 1)
				if ((view->mx > l->area.x2 - view->LAYER_HANDLE_WIDTH) and (view->my < l->area.y1 + view->TRACK_HANDLE_HEIGHT))
					s.type = Selection::Type::LAYER_HEADER;
		}
	}

	// selection boundaries?
	if ((my >= view->area.y2-20) or (view->win->get_key(hui::KEY_SHIFT))){
		if (view->mouse_over_time(view->sel.range.end())){
			s.type = Selection::Type::SELECTION_END;
			return s;
		}
		if (view->mouse_over_time(view->sel.range.start())){
			s.type = Selection::Type::SELECTION_START;
			return s;
		}
	}
	if ((my <= view->TIME_SCALE_HEIGHT) or (view->win->get_key(hui::KEY_SHIFT))){
		if (view->is_playback_active()){
			if (view->mouse_over_time(view->playback_pos())){
				s.type = Selection::Type::PLAYBACK_CURSOR;
				return s;
			}
		}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		if (view->playback_wish_range.is_inside(s.pos))
			s.type = Selection::Type::PLAYBACK_RANGE;
		else
			s.type = Selection::Type::TIME;
		return s;
	}

	return s;
}

Selection ViewModeDefault::get_hover()
{
	Selection s = get_hover_basic(true);

	// already found important stuff?
	if ((s.type != Selection::Type::BACKGROUND) and (s.type != Selection::Type::LAYER) and (s.type != Selection::Type::TIME))
		return s;

	int mx = view->mx;
	int my = view->my;

	if (s.layer){

		// markers
		if (s.layer->is_main()){
			for (int i=0; i<min(s.track->markers.num, s.vlayer->marker_areas.num); i++){
				auto *m = s.track->markers[i];
				if (s.vlayer->marker_areas.contains(m) and s.vlayer->marker_label_areas.contains(m))
				if (s.vlayer->marker_areas[m].inside(mx, my) or s.vlayer->marker_label_areas[m].inside(mx, my)){
					s.marker = m;
					s.type = Selection::Type::MARKER;
					s.index = i;
					return s;
				}
			}
		}

		// TODO: prefer selected samples
		for (SampleRef *ss: s.layer->samples){
			int offset = view->mouse_over_sample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::Type::SAMPLE;
				return s;
			}
		}
	}

	if (s.track){

		// bars
		if (s.track->type == SignalType::BEATS){

			// bar gaps
			if (view->cam.dsample2screen(session->sample_rate()) > 20){
				int offset = 0;
				for (int i=0; i<view->song->bars.num+1; i++){
					float x = view->cam.sample2screen(offset);
					if (fabs(x - mx) < view->SNAPPING_DIST){
						s.index = i;
						s.type = Selection::Type::BAR_GAP;
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
				if ((mx >= x) and (mx < x + 36) and (my < s.vlayer->area.y1 + 20)){
					//b.range.
					s.bar = b;
					s.index = b->index;
					s.type = Selection::Type::BAR;
					return s;
				}
			}
		}
	}

	return s;
}

void ViewModeDefault::set_cursor_pos(int pos, bool keep_track_selection)
{
	if (view->is_playback_active()){
		if (view->renderer->range().is_inside(pos)){
			session->signal_chain->set_pos(pos);
			hover->type = Selection::Type::PLAYBACK_CURSOR;
			view->force_redraw();
			return;
		}else{
			view->stop();
		}
	}
	//view->msp.start(hover->pos, hover->y0);
	view->sel.clear_data();
	if (!keep_track_selection)
		view->sel.tracks = view->cur_track();
		//view->sel.all_tracks(view->song);
	view->set_selection(get_selection_for_range(Range(pos, 0)));

	view->cam.make_sample_visible(pos, 0);
}


void selectLayer(ViewModeDefault *m, TrackLayer *l, bool diff, bool soft)
{
	if (!l)
		return;
	auto &sel = m->view->sel;
	if (diff){
		bool is_only_selected = true;
		for (Track *tt: l->track->song->tracks)
			for (TrackLayer *ll: tt->layers)
				if (sel.has(ll) and (ll != l))
					is_only_selected = false;
		sel.set(l, !sel.has(l) or is_only_selected);
	}else if (soft){
		if (sel.has(l))
			return;
		sel.track_layers.clear();
		sel.add(l);
		sel.tracks.clear();
		sel.add(l->track);
	}else{
		sel.track_layers.clear();
		sel.add(l);
		sel.add(l->track);
		sel.tracks.clear();
	}

	sel.make_consistent(m->song);

	// TODO: what to do???
	m->view->set_selection(m->view->mode->get_selection_for_range(sel.range));
}

void ViewModeDefault::select_hover()
{
	view->sel_temp = view->sel;
	TrackLayer *l = hover->layer;
	SampleRef *s = hover->sample;

	// track
	if (hover->vlayer)
		view->set_cur_layer(hover->vlayer);
	if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::LAYER_HEADER))
		selectLayer(this, l, false, false);
	if (hover->type == Selection::Type::TRACK_HEADER)
		selectLayer(this, l, false, false);

	view->set_cur_sample(s);


	if (hover->type == Selection::Type::BAR_GAP){
		view->sel.clear_data();
		view->sel.bar_gap = hover->index;
		selectLayer(this, l, false, true);
	}else if (hover->type == Selection::Type::BAR){
		auto b = hover->bar;
		if (view->sel.has(b)){
		}else{
			selectLayer(this, l, false, true);
			view->sel.clear_data();
			view->sel.add(b);
		}
	}else if (hover->type == Selection::Type::MARKER){
		auto m = hover->marker;
		if (view->sel.has(m)){
		}else{
			selectLayer(this, l, false, true);
			view->sel.clear_data();
			view->sel.add(m);
		}
	}else if (hover->type == Selection::Type::SAMPLE){
		if (view->sel.has(s)){
		}else{
			selectLayer(this, l, false, true);
			view->sel.clear_data();
			view->sel.add(s);
		}
	}
}

SongSelection ViewModeDefault::get_selection_for_range(const Range &r)
{
	return SongSelection::from_range(song, r, view->sel.tracks, view->sel.track_layers);
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
		s.add(t);

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

		auto *mp = view->midi_painter;
		mp->set_context(vl->area, l->track->instrument, vl->is_playable(), vl->midi_mode);
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
	Set<const Track*> _tracks;
	for (auto vt: view->vtrack){
		if ((y1 >= vt->area.y1) and (y0 <= vt->area.y2))
			_tracks.add(vt->track);
	}
	Set<const TrackLayer*> _layers;
	for (auto vt: view->vlayer){
		if ((y1 >= vt->area.y1) and (y0 <= vt->area.y2))
			_layers.add(vt->layer);
	}
	return SongSelection::from_range(song, r, _tracks, _layers);
}

void ViewModeDefault::start_selection()
{
	hover->range.set_start(view->msp.start_pos);
	hover->range.set_end(hover->pos);
	if (hover->type == Selection::Type::TRACK_HEADER){
		moving_track = hover->track;
	}else if (hover->type == Selection::Type::TIME){
		hover->type = Selection::Type::SELECTION_END;
		view->selection_mode = view->SelectionMode::TIME;
	}else{
		hover->y0 = view->msp.start_y;
		hover->y1 = view->my;
		view->selection_mode = view->SelectionMode::TRACK_RECT;
	}
	view->set_selection(get_selection(hover->range));
}

MidiMode ViewModeDefault::which_midi_mode(Track *t)
{
	if (view->midi_view_mode == MidiMode::LINEAR)
			return MidiMode::LINEAR;
	if ((view->midi_view_mode == MidiMode::TAB) and (t->instrument.string_pitch.num > 0))
		return MidiMode::TAB;
	return MidiMode::CLASSICAL;
}

