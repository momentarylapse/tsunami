/*
 * Cursor.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "Cursor.h"
#include "AudioViewLayer.h"
#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"


Cursor::Cursor(AudioView* view, bool end) : ViewNode(view) {
	z = 50;
	is_end = end;
	drag_range = Range::EMPTY;
}

void Cursor::draw(Painter* c) {
	view->draw_time_line(c, pos(), view->colors.selection_boundary, view_hover(view->hover), false, true);
}

int Cursor::pos() {
	if (is_end)
		return view->sel.range.end();
	else
		return view->sel.range.start();
}

bool Cursor::hover(float mx, float my) {
	if (my < view->area.y2 - 20)
		return false;
	float x = view->cam.sample2screen(pos());
	return (fabs(x - mx) < 10);
}

string Cursor::get_tip() {
	return _("cursor");
}

bool Cursor::on_left_button_down() {
	drag_range = view->sel.range;
	if (!is_end)
		drag_range.invert();

	view->mdp_prepare([=]{
		drag_range.set_end(view->get_mouse_pos_snap());
		view->sel.range = drag_range;
		view->update_selection();
		view->select_under_cursor();
	});
	return true;
}


SelectionMarker::SelectionMarker(AudioView* view) : ViewNode(view) {
	z = 49;
}

void SelectionMarker::draw(Painter* p) {
	float x1, x2;
	view->cam.range2screen_clip(view->sel.range, view->song_area, x1, x2);

	if (!view->hide_selection){
		if ((view->selection_mode == SelectionMode::TIME) or (view->selection_mode == SelectionMode::TRACK_RECT)){
			// drawn as background...

			/*c->setColor(colors.selection_internal);
			for (AudioViewLayer *l: vlayer)
				if (sel.has(l->layer))
					c->draw_rect(rect(sxx1, sxx2, l->area.y1, l->area.y2));*/
		}else if (view->selection_mode == SelectionMode::RECT){
			float x1, x2;
			view->cam.range2screen_clip(view->hover.range, view->clip, x1, x2);
			p->set_color(view->colors.selection_internal);
			p->set_fill(false);
			p->draw_rect(rect(x1, x2, view->hover.y0, view->hover.y1));
			p->set_fill(true);
			p->draw_rect(rect(x1, x2, view->hover.y0, view->hover.y1));
		}
	}


	// bar gap selection
	if (view->cur_selection.type == HoverData::Type::BAR_GAP){
		x2 = view->cam.sample2screen(view->song->bar_offset(view->cur_selection.index));
		p->set_color(view->colors.text_soft1);
		p->set_line_width(2.5f);
		for (auto *t: view->vlayer)
			if (t->layer->type == SignalType::BEATS){
				p->draw_line(x2 - 5, t->area.y1, x2 + 5, t->area.y1);
				p->draw_line(x2, t->area.y1, x2, t->area.y2);
				p->draw_line(x2 - 5, t->area.y2, x2 + 5, t->area.y2);
		}
		p->set_line_width(1.0f);
	}
	if (view->hover.type == HoverData::Type::BAR_GAP){
		x2 = view->cam.sample2screen(view->song->bar_offset(view->hover.index));
		p->set_color(view->colors.hover);
		p->set_line_width(2.5f);
		for (auto *t: view->vlayer)
			if (t->layer->type == SignalType::BEATS){
				p->draw_line(x2 - 5, t->area.y1, x2 + 5, t->area.y1);
				p->draw_line(x2, t->area.y1, x2, t->area.y2);
				p->draw_line(x2 - 5, t->area.y2, x2 + 5, t->area.y2);
		}
		p->set_line_width(1.0f);
	}
}
