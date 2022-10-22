#include "ViewModeEditBars.h"
#include "../sidebar/AudioEditorConsole.h"
#include "../sidebar/SideBar.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../MouseDelayPlanner.h"
#include "../../action/Action.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"
#include "../../TsunamiWindow.h"

class ActionSongMoveBarGap : public Action {
	public:
	ActionSongMoveBarGap(Song *s, int i) {
		index = i;
		pos = pos0 = s->bars[index - 1]->range().end();
		if (index < s->bars.num)
			pos1 = s->bars[index]->range().end();
	}
	void set_param_and_notify(Data *d, int _param) {
		pos = _param;
		execute(d);
		d->notify();
	}
	void _set(Data *d, int p) {
		auto s = dynamic_cast<Song*>(d);
		auto b = weak(s->bars)[index - 1];
		b->length = p - b->offset;
		if (index < s->bars.num)
			s->bars[index]->length = pos1 - p;
	}
	void *execute(Data *d) override {
		_set(d, pos);
		return nullptr;
	}
	void undo(Data *d) override {
		_set(d, pos0);
	}
	void abort_and_notify(Data *d) {
		undo(d);
		d->notify();
	}
	bool is_trivial() override {
		return (pos == pos0);
	}
	int index;
	int pos0, pos, pos1 = 0;
};

class MouseDelayBarGapDnD : public MouseDelayAction {
public:
	AudioView *view;
	ActionSongMoveBarGap *action = nullptr;
	int index;
	Range allowed_range;
	MouseDelayBarGapDnD(AudioView *v, int _index) {
		view = v;
		index = _index;
		allowed_range = Range(view->song->bars[index - 1]->offset + 1, 0x70000000);
		if (index < view->song->bars.num)
			allowed_range.set_end(view->song->bars[index]->range().end() - 1);
	}
	void on_start(const vec2 &m) override {
		action = new ActionSongMoveBarGap(view->song, index);
	}
	void on_update(const vec2 &m) override {
		int p = view->get_mouse_pos();
		view->snap_to_grid(p);
		p = clamp(p, allowed_range.start(), allowed_range.end());
		action->set_param_and_notify(view->song, p);
	}
	void on_finish(const vec2 &m) override {
		view->song->execute(action);
	}
	void on_cancel() override {
		action->undo(view->song);
		delete action;
	}
};

ViewModeEditBars::ViewModeEditBars(AudioView *view) :
	ViewModeDefault(view)
{
	edit_mode = EditMode::SELECT;
}


void ViewModeEditBars::on_start() {
	set_side_bar(SideBar::BARS_EDITOR_CONSOLE);
	set_edit_mode(EditMode::RUBBER);
}

void ViewModeEditBars::on_end() {
}

void ViewModeEditBars::on_key_down(int k) {
	view->force_redraw();

	if (edit_mode == EditMode::RUBBER) {
		if (k == hui::KEY_RETURN)
			add_bar_at_cursor();
	}
}

void ViewModeEditBars::add_bar_at_cursor() {
	int smx = view->get_mouse_pos();
	int index = view->song->bars.get_bar_no(smx);

	if (index >= 0) {
		// inside bar
		auto r = song->bars[index]->range();
		auto bp = *(BarPattern*)(weak(song->bars)[index]);
		song->begin_action_group("add bar");
		bp.length = smx - r.start();
		song->edit_bar(index, bp, Bar::EditMode::IGNORE);
		bp.length = r.end() - smx;
		song->add_bar(index + 1, bp, Bar::EditMode::IGNORE);
		song->end_action_group();
	} else if (smx > 0) {
		BarPattern bp = {100, 4, 1};
		if (song->bars.num > 0)
			bp = *(BarPattern*)weak(song->bars).back();
		bp.length = smx - song->bars.range().end();
		song->add_bar(song->bars.num, bp, Bar::EditMode::IGNORE);
	}
}

float ViewModeEditBars::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l))
		return 120;
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

	if (edit_mode == EditMode::RUBBER) {
		/*foreachi(auto &q, rubber.points, i) {
			float sx = view->cam.sample2screen(q.source);
			float tx = view->cam.sample2screen(q.target);
			if (fabs(tx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::TARGET;
			} else if (fabs(sx - mx) < 10) {
				rubber.hover = i;
				rubber.hover_type = RubberHoverType::SOURCE;
			}
		}*/
	}

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
	
	if (edit_mode == EditMode::RUBBER) {
		auto h = view->hover();
		if (h.type == HoverData::Type::BAR_GAP) {
			if (h.index > 0)
				view->mdp_prepare(new MouseDelayBarGapDnD(view, h.index));
		} else {
			add_bar_at_cursor();
		}
	} else { // SELECT
		ViewModeDefault::left_click_handle_void(vlayer);
	}
}

string ViewModeEditBars::get_tip() {
	if (edit_mode == EditMode::RUBBER) {
		return "EXPERIMENTAL    insert bar [Return]    track [Alt+↑,↓]";
	}
	return "EXPERIMENTAL    track [Alt+↑,↓]";
}

