#include "ViewModeEditBars.h"
#include "../sidebar/AudioEditorConsole.h"
#include "../sidebar/SideBar.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../MouseDelayPlanner.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"
#include "../../TsunamiWindow.h"


ViewModeEditBars::ViewModeEditBars(AudioView *view) :
	ViewModeDefault(view)
{
	edit_mode = EditMode::SELECT;
}


void ViewModeEditBars::on_start() {
	set_side_bar(SideBar::BARS_EDITOR_CONSOLE);
}

void ViewModeEditBars::on_end() {
}

void ViewModeEditBars::on_key_down(int k) {
	view->force_redraw();
}

float ViewModeEditBars::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l))
		return 200;
	return ViewModeDefault::layer_suggested_height(l);
}

void ViewModeEditBars::draw_post(Painter *p) {
	float y1 = view->cur_vlayer()->area.y1;
	float y2 = view->cur_vlayer()->area.y2;
	float x1, x2;
	/*
	if (edit_mode == EditMode::CLONE) {
		// input
		view->cam.range2screen(range_source(), x1, x2);
		p->set_color(color(0.2f, 0, 1, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}
	
	if ((edit_mode == EditMode::CLONE) or (edit_mode == EditMode::SMOOTHEN)) {
		// output
		view->cam.range2screen(range_target(), x1, x2);
		p->set_color(color(0.2f, 1, 0, 0));
		p->draw_rect(rect(x1, x2, y1, y2));
	}

	if (edit_mode == EditMode::RUBBER) {
		foreachi (auto &q, rubber.points, i) {
			// source
			color c = theme.red.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::SOURCE) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::SOURCE) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x1 = view->cam.sample2screen(q.source);
			p->draw_line({x1, y1}, {x1, y2});

			// target
			c = theme.green.with_alpha(0.7f);
			p->set_line_width(2);
			if ((rubber.hover_type == RubberHoverType::TARGET) and (rubber.hover == i))
				c.a = 1; //theme.hoverify(c);
			if ((rubber.selected_type == RubberHoverType::TARGET) and (rubber.selected == i))
				p->set_line_width(4);
			p->set_color(c);
			x2 = view->cam.sample2screen(q.target);
			p->draw_line({x2, y1}, {x2, y2});

			p->set_line_width(2);
			p->set_color(theme.text);
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.25f), vec2(x2, y1 + (y2-y1)*0.25f));
			draw_arrow(p, vec2(x1, y1 + (y2-y1)*0.75f), vec2(x2, y1 + (y2-y1)*0.75f));
			p->set_line_width(1);
		}
	}*/
}

void ViewModeEditBars::set_edit_mode(EditMode mode) {
	edit_mode = mode;
	view->force_redraw();
	notify();
}



AudioViewLayer *ViewModeEditBars::cur_vlayer() {
	return view->cur_vlayer();
}
TrackLayer *ViewModeEditBars::cur_layer() {
	return view->cur_layer();
}
bool ViewModeEditBars::editing(AudioViewLayer *l) {
	return l == cur_vlayer();
}

void ViewModeEditBars::on_mouse_move() {
	float mx = view->m.x;
	int smx = view->cam.screen2sample(mx);

	if (!cur_vlayer()->is_cur_hover())
		return;

	/*if (edit_mode == EditMode::RUBBER) {
		foreachi(auto &q, rubber.points, i) {
			float sx = view->cam.sample2screen(q.source);
			float tx = view->cam.sample2screen(q.target);
			if (fabs(tx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::TARGET;
			} else if (fabs(sx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::SOURCE;
			}
		}
	}*/

	//view->force_redraw();
}

void ViewModeEditBars::left_click_handle_void(AudioViewLayer *vlayer) {
	if (!editing(vlayer)) {
		ViewModeDefault::left_click_handle_void(vlayer);
		return;
	} else if (!view->sel.has(vlayer->layer)) {
		ViewModeDefault::left_click_handle_void(vlayer);
		return;
	}
	
	if (edit_mode == EditMode::CLONE) {
	} else if (edit_mode == EditMode::RUBBER) {
	} else { // SELECT
		ViewModeDefault::left_click_handle_void(vlayer);
	}
}

string ViewModeEditBars::get_tip() {
	if (edit_mode == EditMode::RUBBER) {
		if (view->sel.range().is_empty())
			return "select range for stretching";
		return "EXPERIMENTAL    click in selection to add point    drag to move point    delete point [X]    clear [Shift+X]    apply [Return]    track [Alt+↑,↓]";
	}
	return "EXPERIMENTAL    radius [W,S]    track [Alt+↑,↓]";
}

