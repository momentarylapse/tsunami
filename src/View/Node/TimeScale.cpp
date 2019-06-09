/*
 * TimeScale.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "TimeScale.h"
#include "../AudioView.h"
#include "../ViewPort.h"
#include "../Painter/GridPainter.h"
#include "../../Module/Audio/SongRenderer.h"




TimeScale::TimeScale(AudioView* view) : ViewNode(view) {
	node_width = 100;
	node_height = AudioView::TIME_SCALE_HEIGHT;
	z = 20;
}

void TimeScale::draw(Painter* c) {
	area = view->area;
	area.y2 = node_height;

	GridColors g;
	g.bg = view->colors.background_track;
	g.bg_sel = view->colors.background_track_selection;
	g.fg = g.fg_sel = view->colors.grid;

	auto *gp = view->grid_painter;
	gp->set_context(area, g);
	gp->draw_empty_background(c);
	gp->draw_time(c);

	if (view->is_playback_active()){
		c->set_color(AudioView::colors.preview_marker_internal);
		float x0, x1;
		view->cam.range2screen(view->renderer->range(), x0, x1);
		c->draw_rect(x0, area.y1, x1 - x0, area.y1 + AudioView::TIME_SCALE_HEIGHT);
	}
	view->grid_painter->draw_time_numbers(c);

	// playback lock range
	if (view->playback_wish_range.length > 0){
		c->set_color(AudioView::colors.preview_marker);
		float x0, x1;
		view->cam.range2screen_clip(view->playback_wish_range, area, x0, x1);
		c->draw_rect(x0, area.y1, x1-x0, 5);
	}

	// playback lock symbol
	playback_lock_button = rect::EMPTY;
	if (view->playback_range_locked){
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		playback_lock_button = rect(x, x + 20, area.y1, area.y1 + AudioView::TIME_SCALE_HEIGHT);

		//c->set_color((view->hover.type == Selection::Type::PLAYBACK_SYMBOL_LOCK) ? AudioView::colors.hover : AudioView::colors.preview_marker);
		c->set_font_size(AudioView::FONT_SIZE * 0.7f);
		c->draw_str(playback_lock_button.x1 + 5, playback_lock_button.y1 + 3, u8"🔒");
		c->set_font_size(AudioView::FONT_SIZE);
	}

	// playback loop symbol
	playback_loop_button = rect::EMPTY;
	if (view->playback_loop){
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		playback_loop_button = rect(x + 20, x + 40, area.y1, area.y1 + AudioView::TIME_SCALE_HEIGHT);

		//c->set_color((view->hover.type == Selection::Type::PLAYBACK_SYMBOL_LOOP) ? AudioView::colors.hover : AudioView::colors.preview_marker);
		//c->set_font_size(FONT_SIZE * 0.7f);
		c->draw_str(playback_loop_button.x1 + 5, playback_loop_button.y1 + 3, u8"⏎");
		c->set_font_size(AudioView::FONT_SIZE);
	}
}

string TimeScale::get_tip() {
	if (hover_lock_button())
		return _("locked");
	if (hover_loop_button())
		return _("looping");
	if (hover_playback())
		return _("playback range");
	return "time?";
}

bool TimeScale::on_left_button_down() {
	view->snap_to_grid(view->hover.pos);
	view->set_cursor_pos(view->hover.pos);
//	view->msp.start(view->hover.pos, view->hover.y0);
	return true;
}

bool TimeScale::hover_lock_button() {
	return playback_lock_button.inside(view->mx, view->my);
}

bool TimeScale::hover_loop_button() {
	return playback_loop_button.inside(view->mx, view->my);
}

bool TimeScale::hover_playback() {
	int pos = view->cam.screen2sample(view->mx);
	return view->playback_wish_range.is_inside(pos);
}

bool TimeScale::on_right_button_down() {
	if (hover_playback())
		view->open_popup(view->menu_playback_range);
	else
		view->open_popup(view->menu_song);
	return true;
}
