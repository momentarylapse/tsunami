/*
 * ViewModeDefault.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeDefault.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../AudioViewLayer.h"
#include "../SideBar/SideBar.h"
#include "../../TsunamiWindow.h"
#include "../../Device/OutputStream.h"
#include "../../Action/Song/ActionSongMoveSelection.h"
#include "math.h"
#include "../../Module/Audio/SongRenderer.h"
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
#include "../Helper/ScrollBar.h"
#include "../Painter/BufferPainter.h"
#include "../Painter/GridPainter.h"
#include "../Painter/MidiPainter.h"
#include "../../lib/hui/Controls/Control.h"

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = nullptr;
	moving_track = nullptr;
	dnd_selection = new SongSelection;
	dnd_ref_pos = 0;
	dnd_mouse_pos0 = 0;
}

ViewModeDefault::~ViewModeDefault()
{
	delete dnd_selection;
}


void ViewModeDefault::on_left_button_down()
{
	//bool track_hover_sel = view->sel.has(hover->track);
	select_under_mouse();

	view->snap_to_grid(hover->pos);

	// selection:
	//   start after lb down and moving
	if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::CLEF_POSITION)){
		set_cursor_pos(hover->pos, true);//track_hover_sel);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::Type::BAR){
		set_cursor_pos(hover->pos, true);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::Type::TIME){
		set_cursor_pos(hover->pos, true);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::Type::BAR_GAP){
		set_cursor_pos(hover->pos, true);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::Type::BACKGROUND){
		set_cursor_pos(hover->pos, false);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::Type::SELECTION_END){
		hover->range = view->sel.range;
		view->selection_mode = view->SelectionMode::TIME;
	}else if (hover->type == Selection::Type::SELECTION_START){
		// swap end / start
		hover->type = Selection::Type::SELECTION_END;
		hover->range = view->sel.range;
		hover->range.invert();
		view->selection_mode = view->SelectionMode::TIME;
	}else if (hover->type == Selection::Type::TRACK_BUTTON_MUTE){
		hover->vtrack->set_muted(!hover->track->muted);
	}else if (hover->type == Selection::Type::TRACK_BUTTON_SOLO){
		hover->vtrack->set_solo(!hover->vtrack->solo);
	}else if (hover->type == Selection::Type::TRACK_BUTTON_EDIT){
		view->win->side_bar->open(SideBar::TRACK_CONSOLE);
	}else if (hover->type == Selection::Type::TRACK_BUTTON_FX){
		view->win->side_bar->open(SideBar::FX_CONSOLE);
	}else if (hover->type == Selection::Type::TRACK_BUTTON_CURVE){
		view->win->side_bar->open(SideBar::CURVE_CONSOLE);
	}else if (hover->type == Selection::Type::LAYER_BUTTON_DOMINANT){

	}else if (hover->type == Selection::Type::LAYER_BUTTON_SOLO){
		hover->vlayer->set_solo(!hover->vlayer->solo);
	}else if (hover->type == Selection::Type::LAYER_BUTTON_IMPLODE){
		view->implode_track(hover->track);
	}else if (hover->type == Selection::Type::LAYER_BUTTON_EXPLODE){
		view->explode_track(hover->track);
	}else if (hover->type == Selection::Type::SAMPLE){
		dnd_start_soon(view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES));
	}else if (hover->type == Selection::Type::MARKER){
		dnd_start_soon(view->sel.filter(SongSelection::Mask::MARKERS | SongSelection::Mask::SAMPLES));
	}else if (hover->type == Selection::Type::TRACK_HEADER){
		view->msp.start(hover->pos, hover->y0);
	}else if (view->hover.type == Selection::Type::SCROLLBAR_GLOBAL){
		view->scroll->drag_start(view->mx, view->my);
	}else if (view->hover.type == Selection::Type::PLAYBACK_LOCK){
		if (view->playback_range_locked){
			view->playback_range_locked = false;
		}else{
			view->playback_lock_range = view->sel.range;
			view->playback_range_locked = true;
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
		//int orig = get_track_index(moving_track);
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
	foreachi (AudioBuffer &b, hover->layer->buffers, i)
		if (b.range().is_inside(hover->pos))
			return i;
	return -1;
}

void ViewModeDefault::on_left_double_click()
{
	if (view->selection_mode != view->SelectionMode::NONE)
		return;

	select_under_mouse();

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
	bool track_hover_sel = view->sel.has(hover->track);

	select_under_mouse();

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
		view->menu_sample->open_popup(view->win);
	}else if (hover->type == Selection::Type::BAR){
		view->menu_time_track->enable("delete_bars", true);
		view->menu_time_track->enable("edit_bars", true);
		view->menu_time_track->enable("scale_bars", true);
		view->menu_time_track->open_popup(view->win);
		//view->menu_bar->open_popup(view->win);
	}else if (hover->type == Selection::Type::MARKER){
		view->menu_marker->open_popup(view->win);
	}else if (hover->type == Selection::Type::BAR_GAP){
		view->menu_time_track->enable("delete_bars", false);
		view->menu_time_track->enable("edit_bars", false);
		view->menu_time_track->enable("scale_bars", false);
		view->menu_time_track->open_popup(view->win);
	}else if ((hover->type == Selection::Type::LAYER) and (hover->track->type == SignalType::BEATS)){
		view->menu_time_track->enable("delete_bars", false);
		view->menu_time_track->enable("edit_bars", false);
		view->menu_time_track->enable("scale_bars", false);
		view->menu_time_track->open_popup(view->win);
	}else if (hover->type == Selection::Type::LAYER_HEADER){
		view->menu_layer->enable("layer_midi_mode_tab", hover->track->instrument.string_pitch.num > 0);
		view->menu_layer->check("layer_midi_mode_linear", hover->vlayer->midi_mode == MidiMode::LINEAR);
		view->menu_layer->check("layer_midi_mode_classical", hover->vlayer->midi_mode == MidiMode::CLASSICAL);
		view->menu_layer->check("layer_midi_mode_tab", hover->vlayer->midi_mode == MidiMode::TAB);
		view->menu_layer->open_popup(view->win);
	}else if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::TRACK_HEADER)){
		view->menu_track->enable("layer_midi_mode_tab", hover->track->instrument.string_pitch.num > 0);
		view->menu_track->check("layer_midi_mode_linear", hover->vlayer->midi_mode == MidiMode::LINEAR);
		view->menu_track->check("layer_midi_mode_classical", hover->vlayer->midi_mode == MidiMode::CLASSICAL);
		view->menu_track->check("layer_midi_mode_tab", hover->vlayer->midi_mode == MidiMode::TAB);

		view->menu_track->enable("menu_midi_mode", hover->track->type == SignalType::MIDI);
		view->menu_track->enable("track_edit_midi", hover->track->type == SignalType::MIDI);
		view->menu_track->enable("track_add_marker", hover->type == Selection::Type::LAYER);
		view->menu_track->enable("track_convert_stereo", hover->track->channels == 1);
		view->menu_track->enable("track_convert_mono", hover->track->channels == 2);
		view->menu_track->enable("layer_merge", hover->layer->track->layers.num > 1);
		view->menu_track->enable("layer_mark_dominant", hover->layer->track->layers.num > 1);// and view->sel.layers.num == 1);
		//view->menu_track->enable("delete_layer", !hover->layer->is_main());
		view->menu_track->enable("menu_buffer", hover_buffer(hover) >= 0);
		view->menu_track->open_popup(view->win);
	}else  if ((hover->type == Selection::Type::SELECTION_START) or (hover->type == Selection::Type::SELECTION_END)){
	}else if (!hover->track){
		view->menu_song->open_popup(view->win);
	}
}

void ViewModeDefault::on_right_button_up()
{
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

		if (view->hover.type == Selection::Type::SCROLLBAR_GLOBAL){
			view->scroll->drag_update(view->mx, view->my);
			view->thm.update_immediately(view, view->song, view->song_area);
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
	}else if (hover->type == Selection::Type::PLAYBACK){
		view->renderer->seek(hover->pos);
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
	view->renderer->seek(pos);
	view->stream->clear_buffer();
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
		set_cursor_pos(view->song->range_with_time().start(), true);
	if (k == hui::KEY_END)
		set_cursor_pos(view->song->range_with_time().end(), true);

	// zoom
	if (k == hui::KEY_ADD)
		cam->zoom(exp(  view->ZoomSpeed), view->mx);
	if (k == hui::KEY_SUBTRACT)
		cam->zoom(exp(- view->ZoomSpeed), view->mx);

	if (k == hui::KEY_SPACE){
		if (view->is_playback_active()){
			view->pause(!view->is_paused());
		}else{
			win->on_play();
			//view->play(); unsafe for recording...
		}
	}

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

void ViewModeDefault::on_key_up(int k)
{
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



void ViewModeDefault::draw_midi(Painter *c, AudioViewLayer *l, const MidiNoteBuffer &midi, bool as_reference, int shift)
{
	view->midi_painter->set_context(l->area, l->layer->track->instrument, view->midi_scale, l->is_playable(), l->midi_mode);
	view->midi_painter->set_shift(shift);
	view->midi_painter->draw(c, midi);
}

void ViewModeDefault::draw_layer_background(Painter *c, AudioViewLayer *l)
{
	view->grid_painter->set_context(l->area, l->grid_colors());
	view->grid_painter->draw_empty_background(c);
	view->grid_painter->draw_whatever(c, 0);


	if (l->layer->type == SignalType::MIDI){
		view->midi_painter->set_context(l->area, l->layer->track->instrument, view->midi_scale, l->is_playable(), l->midi_mode);
		view->midi_painter->draw_background(c);
	}



	// parts
	c->set_line_width(2.0f);
	string prev_part = "";
	for (auto *m: l->layer->song()->get_parts()){
		color col = l->marker_color(m);
		col.a = 0.75f;
		float x0, x1;
		view->cam.range2screen(m->range, x0, x1);
		if (m->text == prev_part)
			col.a *= clampf((x1 - x0) / 200, 0.25f, 1.0f);
		c->set_color(col);
		c->draw_line(x0, l->area.y1, x0, l->area.y2);
		prev_part = m->text;
	}
	c->set_line_width(1.0f);
}

void draw_bar_selection(Painter *c, AudioViewTrack *t, AudioView *view)
{
	float y1 = t->area.y1;
	float y2 = t->area.y2;

	float lw = 3;

	c->set_line_width(lw*2);
	c->set_color(view->colors.selection_bars);
	c->set_fill(false);

	auto bars = view->song->bars.get_bars(Range::ALL);
	for (auto b: bars){
		if ((view->sel.has(b)) or (b == view->hover.bar)){
			color col = view->colors.hover;
			col.a = 0.3f;
			if (view->sel.has(b)){
				col = view->colors.selection_bars;
				if (b == view->hover.bar)
					col = view->colors.selection_bars_hover;
			}
			c->set_color(col);
			float x1, x2;
			view->cam.range2screen(b->range(), x1, x2);
			c->draw_rect(x1 + lw, y1 + lw, x2-x1 - 2*lw, y2-y1 - 2*lw);
		}
	}
	c->set_fill(true);
	c->set_line_width(1);

}

void ViewModeDefault::draw_track_data(Painter *c, AudioViewTrack *t)
{

	//if (t->track->type == SignalType::BEATS)
	//	draw_bar_selection(c, t, view);
}

void ViewModeDefault::draw_imploded_track_background(Painter *c, AudioViewTrack *t)
{
}

void ViewModeDefault::draw_imploded_track_data(Painter *c, AudioViewTrack *t)
{
	auto *l = view->get_layer(t->track->layers[0]);
	view->buffer_painter->set_context(l->area);


	if (t->track->has_version_selection()){
		Range r = Range(t->track->range().start(), 0);
		int index = 0;
		for (auto &f: t->track->fades){
			r = RangeTo(r.end(), f.position);
			view->buffer_painter->set_clip(r);

			for (AudioBuffer &b: t->track->layers[index]->buffers){
				view->buffer_painter->set_color(t->is_playable() ? view->colors.text : view->colors.text_soft3);
				view->buffer_painter->draw_buffer(c, b, b.offset);
			}

			index = f.target;
		}

		r = RangeTo(r.end(), t->track->range().end());
		view->buffer_painter->set_clip(r);
		for (AudioBuffer &b: t->track->layers[index]->buffers){
			view->buffer_painter->set_color(t->is_playable() ? view->colors.text : view->colors.text_soft3);
			view->buffer_painter->draw_buffer(c, b, b.offset);
		}
	}else{
		view->buffer_painter->set_color(t->is_playable() ? view->colors.text : view->colors.text_soft3);
		for (auto *layer: t->track->layers)
			for (AudioBuffer &b: layer->buffers)
				view->buffer_painter->draw_buffer(c, b, b.offset);

	}

	/*if (view->sel.has(layer)){
		// selection
		for (AudioBuffer &b: layer->buffers){
			draw_buffer_selection(c, b, view_pos_rel, view->colors.selection_boundary, view->sel.range);
		}
	}*/



	view->draw_boxed_str(c, t->area.x2 - 200, t->area.y1 + 10, "imploded...", view->colors.text, view->colors.background_track_selection);
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

void ViewModeDefault::draw_layer_data(Painter *c, AudioViewLayer *l)
{
	Track *t = l->layer->track;



	if (l->layer->type == SignalType::BEATS){
		view->grid_painter->set_context(l->area, l->grid_colors());
		if (song->bars.num > 0)
			view->grid_painter->draw_bar_numbers(c);
	}


	// midi
	if (l->layer->type == SignalType::MIDI)
		draw_midi(c, l, l->layer->midi, false, 0);

	// audio buffer
	l->draw_track_buffers(c);

	// samples
	for (SampleRef *s: l->layer->samples)
		l->draw_sample(c, s);

	if (l->layer->is_main()){

		// marker
		l->marker_areas.resize(t->markers.num);
		l->marker_label_areas.resize(t->markers.num);
		foreachi(TrackMarker *m, l->layer->track->markers, i)
			l->draw_marker(c, m, i, (hover->type == Selection::Type::MARKER) and (hover->track == t) and (hover->index == i));
	}

	c->set_line_width(1.0f);

	int index_before = 0;
	int index_own = l->layer->version_number();

	/*if (index_own == 0 and l->layer->track->has_version_selection()){
		draw_fade_bg(c, l, view, -1);
	}*/

	c->set_line_width(2);
	foreachi (auto &f, l->layer->track->fades, i){
		/*if (f.target == index_own){
			draw_fade_bg(c, l, view, i);
		}*/
		if (f.target == index_own or index_before == index_own){
			float x1, x2;
			view->cam.range2screen(f.range(), x1, x2);
			c->set_color(color(1,0,0.7f,0));
			c->draw_line(x1, l->area.y1, x1, l->area.y2);
			c->draw_line(x2, l->area.y1, x2, l->area.y2);
		}
		index_before = f.target;
	}
	c->set_line_width(1);
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

	if (hover->type == Selection::Type::SAMPLE)
		view->draw_cursor_hover(c, _("sample ") + hover->sample->origin->name);
	else if (hover->type == Selection::Type::MARKER)
		view->draw_cursor_hover(c, _("marker ") + hover->marker->text);
	else if (hover->type == Selection::Type::TRACK_BUTTON_EDIT)
		view->draw_cursor_hover(c, _("edit track properties"));
	else if (hover->type == Selection::Type::TRACK_BUTTON_MUTE)
		view->draw_cursor_hover(c, _("toggle mute"));
	else if (hover->type == Selection::Type::TRACK_BUTTON_SOLO)
		view->draw_cursor_hover(c, _("toggle solo"));
	else if (hover->type == Selection::Type::TRACK_BUTTON_CURVE)
		view->draw_cursor_hover(c, _("edit curves"));
	else if (hover->type == Selection::Type::TRACK_BUTTON_FX)
		view->draw_cursor_hover(c, _("edit effects"));
	else if (hover->type == Selection::Type::LAYER_BUTTON_DOMINANT)
		view->draw_cursor_hover(c, _("make main version"));
	else if (hover->type == Selection::Type::LAYER_BUTTON_SOLO)
		view->draw_cursor_hover(c, _("toggle solo"));
	else if (hover->type == Selection::Type::LAYER_BUTTON_IMPLODE)
		view->draw_cursor_hover(c, _("implode"));
	else if (hover->type == Selection::Type::LAYER_BUTTON_EXPLODE)
		view->draw_cursor_hover(c, _("explode"));
	else if (hover->type == Selection::Type::BAR)
		view->draw_cursor_hover(c, _("bar ") + hover->bar->format_beats() + format(" \u2669=%.1f", hover->bar->bpm(song->sample_rate)));
	else if (hover->type == Selection::Type::BAR_GAP)
		{}//view->draw_cursor_hover(c, _("bar gap"));
	else if (hover->type == Selection::Type::PLAYBACK_LOCK)
		view->draw_cursor_hover(c, _("lock playback range"));

	if (view->selection_mode != view->SelectionMode::NONE){
		if (view->sel.bars.num > 0)
			view->draw_cursor_hover(c, format(_("%d bars"), view->sel.bars.num));
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

	if (view->scroll->area.inside(mx, my)){
		s.type = s.Type::SCROLLBAR_GLOBAL;
		return s;
	}

	// layer?
	foreachi(AudioViewLayer *l, view->vlayer, i){
		if (l->mouse_over()){
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
				s.type = Selection::Type::PLAYBACK;
				return s;
			}
		}
	}
	if (view->playback_lock_button.inside(mx, my)){
		s.type = Selection::Type::PLAYBACK_LOCK;
		return s;
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::Type::TIME;
		return s;
	}

	// track header buttons?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		int x = 5;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::Type::TRACK_BUTTON_MUTE;
			return s;
		}
		x += 17;
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::Type::TRACK_BUTTON_SOLO;
			return s;
		}
		x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34) and editable){
			s.type = Selection::Type::TRACK_BUTTON_EDIT;
			return s;
		}
		/*x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::Type::TRACK_BUTTON_FX;
			return s;
		}*/
		/*x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::Type::TRACK_BUTTON_CURVE;
			return s;
		}*/
	}


	// layer header buttons?
	if (s.vlayer and (s.type == s.Type::LAYER_HEADER) and !s.track->has_version_selection()){
		AudioViewLayer *l = s.vlayer;
		int x = l->area.width() - view->LAYER_HANDLE_WIDTH + 5;
		/*if ((mx >= l->area.x1 + x) and (mx < l->area.x1 + x+12) and (my >= l->area.y1 + 22) and (my < l->area.y1 + 34)){
			s.type = Selection::Type::LAYER_BUTTON_DOMINANT;
			return s;
		}*/
		x += 17;
		if ((mx >= l->area.x1 + x) and (mx < l->area.x1 + x+12) and (my >= l->area.y1 + 22) and (my < l->area.y1 + 34)){
			s.type = Selection::Type::LAYER_BUTTON_SOLO;
			return s;
		}
	}
	if (s.vlayer and (s.type == s.Type::LAYER_HEADER) and s.layer->is_main()){
		AudioViewLayer *l = s.vlayer;
		int x = l->area.width() - view->LAYER_HANDLE_WIDTH + 5 + 17 + 17;
		if ((mx >= l->area.x1 + x) and (mx < l->area.x1 + x+12) and (my >= l->area.y1 + 22) and (my < l->area.y1 + 34)){
			if (l->represents_imploded)
				s.type = Selection::Type::LAYER_BUTTON_EXPLODE;
			else
				s.type = Selection::Type::LAYER_BUTTON_IMPLODE;
			return s;
		}
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
				if (s.vlayer->marker_areas[i].inside(mx, my) or s.vlayer->marker_label_areas[i].inside(mx, my)){
					s.marker = s.track->markers[i];
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
			auto bars = view->song->bars.get_bars(Range(s.pos, 0));
			for (auto *b: bars){
				//b.range.
				s.bar = b;
				s.index = b->index;
				s.type = Selection::Type::BAR;
				return s;
			}
		}
	}

	return s;
}

void ViewModeDefault::set_cursor_pos(int pos, bool keep_track_selection)
{
	if (view->is_playback_active()){
		if (view->renderer->range().is_inside(pos)){
			view->renderer->seek(pos);
			view->stream->clear_buffer();
			hover->type = Selection::Type::PLAYBACK;
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

void ViewModeDefault::select_under_mouse()
{
	view->sel_temp = view->sel;
	*hover = get_hover();
	TrackLayer *l = hover->layer;
	SampleRef *s = hover->sample;
	bool control = win->get_key(hui::KEY_CONTROL);

	// track
	if (hover->vlayer)
		view->set_cur_layer(hover->vlayer);
	if ((hover->type == Selection::Type::LAYER) or (hover->type == Selection::Type::LAYER_HEADER))
		selectLayer(this, l, control, false);
	if (hover->type == Selection::Type::TRACK_HEADER)
		selectLayer(this, l, control, false);

	view->set_cur_sample(s);


	if (hover->type == Selection::Type::BAR_GAP){
		view->sel.clear_data();
		view->sel.bar_gap = hover->index;
		selectLayer(this, l, control, true);
	}else if (hover->type == Selection::Type::BAR){
		auto b = hover->bar;
		if (control){
			view->sel.set(b, !view->sel.has(b));
		}else{
			if (view->sel.has(b)){
			}else{
				selectLayer(this, l, control, true);
				view->sel.clear_data();
				view->sel.add(b);
			}
		}
	}else if (hover->type == Selection::Type::MARKER){
		auto m = hover->marker;
		if (control){
			view->sel.set(m, !view->sel.has(m));
		}else{
			if (view->sel.has(m)){
			}else{
				selectLayer(this, l, control, true);
				view->sel.clear_data();
				view->sel.add(m);
			}
		}
	}else if (hover->type == Selection::Type::SAMPLE){
		if (control){
			view->sel.set(s, !view->sel.has(s));
		}else{
			if (view->sel.has(s)){
			}else{
				selectLayer(this, l, control, true);
				view->sel.clear_data();
				view->sel.add(s);
			}
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
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();
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
		mp->set_context(vl->area, l->track->instrument, view->midi_scale, vl->is_playable(), vl->midi_mode);
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

