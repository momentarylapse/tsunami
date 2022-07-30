/*
 * Cursor.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "Cursor.h"
#include "AudioViewLayer.h"
#include "../AudioView.h"
#include "../../../data/base.h"
#include "../../../data/Song.h"
#include "../../../data/TrackLayer.h"
#include "../../../Session.h"
#include "../../../TsunamiWindow.h"

bool view_has_focus(AudioView *view);

Cursor::Cursor(AudioView *_view, bool end) : scenegraph::NodeFree() {
	align.dz = 50;
	set_perf_name("cursor");
	view = _view;
	is_end = end;
	drag_range = Range::NONE;
}

void Cursor::on_draw(Painter* c) {
	color col = theme.selection_boundary;//color::interpolate(colors.selection_boundary, colors.background_track_selected, 0.2f);

	if (!view_has_focus(view))
		col = color::interpolate(theme.selection_boundary, theme.background_track_selected, 0.5f);

	if (is_end) {
		float x = view->cam.sample2screen(pos());
		float r = 4;
		c->set_line_width(4);
		c->set_color(col);//colors.selection_boundary);
		for (auto *v: view->vlayers)
			if (view->sel.has(v->layer)) {
				c->draw_line({x - r, v->area.y1}, {x + r, v->area.y1});
				c->draw_line({x, v->area.y1}, {x, v->area.y2});
				c->draw_line({x - r, v->area.y2}, {x + r, v->area.y2});
			}
		c->set_line_width(1);
	}
	view->draw_time_line(c, pos(), col, is_cur_hover(), false);


	float x = view->cam.sample2screen(pos());
	if ((x >= view->song_area().x1) and (x <= view->song_area().x2)) {
		color cc = col;
		if (is_cur_hover())
			cc = theme.selection_boundary_hover;
		for (auto *v: view->vlayers)
			if (view->sel.has(v->layer)) {
				c->draw_circle({x, v->area.my()}, 6);
			}
	}
}

int Cursor::pos() const {
	if (is_end)
		return view->sel.range_raw.end();
	else
		return view->sel.range_raw.start();
}

bool Cursor::hover(const vec2 &m) const {
	float x = view->cam.sample2screen(pos());
	for (auto *v: view->vlayers)
		if (view->sel.has(v->layer)) {
			if ((m - vec2(x, v->area.my())).length() < 10)
				return true;
		}
	return false;
}

string Cursor::get_tip() const {
	return _("cursor");
}

bool Cursor::on_left_button_down(const vec2 &m) {
	drag_range = view->sel.range_raw;

	view->mdp_prepare([this] {
		if (is_end)
			drag_range.set_end(view->get_mouse_pos_snap());
		else
			drag_range.set_start(view->get_mouse_pos_snap());
		view->sel.range_raw = drag_range;
		view->update_selection();
		view->select_under_cursor();
	});
	return true;
}


SelectionMarker::SelectionMarker(AudioView *_view) : scenegraph::NodeFree() {
	view = _view;
	align.dz = 49;
	set_perf_name("selection");
}

void SelectionMarker::draw_bar_gap_selector(Painter* p, int bar_gap, const color &col) {

	float x2 = view->cam.sample2screen(view->song->bar_offset(bar_gap));
	p->set_color(col);
	p->set_line_width(2.5f);
	for (auto *t: view->vlayers)
		if (t->layer->type == SignalType::BEATS) {
			p->draw_line({x2 - 5, t->area.y1}, {x2 + 5, t->area.y1});
			p->draw_line({x2, t->area.y1}, {x2, t->area.y2});
			p->draw_line({x2 - 5, t->area.y2}, {x2 + 5, t->area.y2});
	}
	p->set_line_width(1.0f);
}

void SelectionMarker::on_draw(Painter* p) {
	float x1, x2;
	view->cam.range2screen_clip(view->cursor_range(), view->song_area(), x1, x2);

	auto &hover = view->hover();
	auto &sel = view->sel;

	if (!view->hide_selection) {
		if ((view->selection_mode == SelectionMode::TIME) or (view->selection_mode == SelectionMode::TRACK_RECT)) {
			// drawn as background...

			/*c->setColor(colors.selection_internal);
			for (AudioViewLayer *l: vlayer)
				if (sel.has(l->layer))
					c->draw_rect(rect(sxx1, sxx2, l->area.y1, l->area.y2));*/
		} else if (view->selection_mode == SelectionMode::RECT) {
			float x1, x2;
			view->cam.range2screen_clip(sel.range(), view->clip, x1, x2);
			p->set_color(theme.selection_internal);
			p->set_fill(false);
			p->draw_rect(rect(x1, x2, hover.y0, hover.y1));
			p->set_fill(true);
			p->draw_rect(rect(x1, x2, hover.y0, hover.y1));
		}
	}


	// bar gap selection
	if (view->cur_selection.type == HoverData::Type::BAR_GAP)
		draw_bar_gap_selector(p, view->cur_selection.index, theme.text_soft1);
	if (hover.type == HoverData::Type::BAR_GAP)
		draw_bar_gap_selector(p, view->hover().index, theme.hover);
}
