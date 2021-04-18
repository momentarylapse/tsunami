/*
 * Cursor.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "../Graph/Cursor.h"

#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../Graph/AudioViewLayer.h"


Cursor::Cursor(AudioView *_view, bool end) : scenegraph::NodeFree() {
	align.dz = 50;
	set_perf_name("cursor");
	view = _view;
	is_end = end;
	drag_range = Range::NONE;
}

void Cursor::on_draw(Painter* c) {
	color col = color::interpolate(view->colors.selection_boundary, view->colors.background_track_selected, 0.2f);
	if (!is_end) {
		float x = view->cam.sample2screen(pos());
		float r = 4;
		c->set_line_width(4);
		c->set_color(col);//view->colors.selection_boundary);
		for (auto *v: view->vlayer)
			if (view->sel.has(v->layer)) {
				c->draw_line(x - r, v->area.y1, x + r, v->area.y1);
				c->draw_line(x, v->area.y1, x, v->area.y2);
				c->draw_line(x - r, v->area.y2, x + r, v->area.y2);
			}
		c->set_line_width(1);
	}
	view->draw_time_line(c, pos(), col, is_cur_hover(), false, true);
}

int Cursor::pos() {
	if (is_end)
		return view->sel.range_raw.end();
	else
		return view->sel.range_raw.start();
}

bool Cursor::hover(float mx, float my) {
	if (my < view->song_area().y2 - 20)
		return false;
	float x = view->cam.sample2screen(pos());
	return (fabs(x - mx) < 10);
}

string Cursor::get_tip() {
	return _("cursor");
}

bool Cursor::on_left_button_down(float mx, float my) {
	drag_range = view->sel.range_raw;
	/*if (!is_end)
		drag_range.invert();*/

	view->mdp_prepare([=]{
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
	for (auto *t: view->vlayer)
		if (t->layer->type == SignalType::BEATS) {
			p->draw_line(x2 - 5, t->area.y1, x2 + 5, t->area.y1);
			p->draw_line(x2, t->area.y1, x2, t->area.y2);
			p->draw_line(x2 - 5, t->area.y2, x2 + 5, t->area.y2);
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
		}else if (view->selection_mode == SelectionMode::RECT) {
			float x1, x2;
			view->cam.range2screen_clip(sel.range(), view->clip, x1, x2);
			p->set_color(view->colors.selection_internal);
			p->set_fill(false);
			p->draw_rect(rect(x1, x2, hover.y0, hover.y1));
			p->set_fill(true);
			p->draw_rect(rect(x1, x2, hover.y0, hover.y1));
		}
	}


	// bar gap selection
	if (view->cur_selection.type == HoverData::Type::BAR_GAP)
		draw_bar_gap_selector(p, view->cur_selection.index, view->colors.text_soft1);
	if (hover.type == HoverData::Type::BAR_GAP)
		draw_bar_gap_selector(p, view->hover().index, view->colors.hover);
}
