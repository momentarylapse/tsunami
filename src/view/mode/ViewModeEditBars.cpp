#include "ViewModeEditBars.h"
#include "../sidebar/AudioEditorConsole.h"
#include "../sidebar/SideBar.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../MouseDelayPlanner.h"
#include "../TsunamiWindow.h"
#include "../../action/Action.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"

void draw_arrow(Painter *p, const vec2 &a, const vec2 &b);

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


class MouseDelayDragRubberEndPoint : public MouseDelayAction {
public:
	AudioView *view;
	int *end;
	MouseDelayDragRubberEndPoint(AudioView *v, int *_end) {
		view = v;
		end = _end;
	}
	void on_update(const vec2 &m) override {
		*end = view->cam.screen2sample(m.x);
	}
};

ViewModeEditBars::ViewModeEditBars(AudioView *view) :
	ViewModeDefault(view)
{
	edit_mode = EditMode::SELECT;
}


void ViewModeEditBars::on_start() {
	set_side_bar(SideBar::BARS_EDITOR_CONSOLE);
	view->subscribe(this, [this] {
		rubber_end_target = selected_bar_range().end();
	}, AudioView::MESSAGE_SELECTION_CHANGE);
	rubber_end_target = selected_bar_range().end();
}

void ViewModeEditBars::on_end() {
	unsubscribe(view);
}

void ViewModeEditBars::on_key_down(int k) {
	view->force_redraw();

	if (edit_mode == EditMode::RUBBER) {
		if (k == hui::KEY_RETURN) {
			auto scaling_range_orig = selected_bar_range();
			auto target_range = Range::to(scaling_range_orig.offset, rubber_end_target);

			float factor = (float)target_range.length / (float)scaling_range_orig.length;

			song->begin_action_group("scale bars");
			auto indices = view->sel.bar_indices(song);
			foreachb(int i, indices) {
				BarPattern bb = *song->bars[i];
				bb.length = (int)((float)bb.length * factor);
				song->edit_bar(i, bb, Bar::EditMode::STRETCH);
			}
			song->end_action_group();
		}
	} else if (edit_mode == EditMode::ADD_AND_SPLIT) {
		if (k == hui::KEY_DELETE) {
			auto h = view->hover();
			if (h.type != HoverData::Type::BAR_GAP)
				return;
			int index = h.index;
			if (index < 1)
				return;

			if (index == song->bars.num) {
				// last one
				song->delete_bar(index - 1, false);
			} else {
				song->begin_action_group("delete bar");
				auto bp = *(BarPattern*)(weak(song->bars)[index - 1]);
				bp.length += song->bars[index]->length;
				song->delete_bar(index, false);
				song->edit_bar(index - 1, bp, Bar::EditMode::IGNORE);
				song->end_action_group();
			}
		}
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
Range ViewModeEditBars::selected_bar_range() const {
		auto id = view->sel.bar_indices(song);
		if (id.num == 0)
			return Range::NONE;
		return Range::to(song->bars[id[0]]->offset, song->bars[id.back()]->range().end());
}

float ViewModeEditBars::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l))
		return 120;
	return ViewModeDefault::layer_suggested_height(l);
}

void ViewModeEditBars::draw_post(Painter *p) {
	float y1 = view->cur_vlayer()->area.y1;
	float y2 = view->cur_vlayer()->area.y2;

	if (edit_mode == EditMode::RUBBER) {
		auto r = selected_bar_range();
		if (r.is_empty())
			return;
		float x1, x2;
		view->cam.range2screen(r, x1, x2);
		p->set_color(theme.green.with_alpha(0.2f));
		p->draw_rect(rect(x1, x2, y1, y2));
		p->set_line_width(2);
		p->set_color(theme.green);
		p->draw_line({x1, y1}, {x1, y2});
		p->set_line_dash({5,8}, 0);
		p->draw_line({x2, y1}, {x2, y2});
		p->draw_line({x1, (y1+y2)/2 + 7}, {x2, (y1+y2)/2 + 7});

		p->set_line_dash({}, 0);
		if (rubber_hover)
			p->set_line_width(4);
		float x3 = view->cam.sample2screen_f(rubber_end_target);
		p->draw_line({x3, y1}, {x3, y2});
		p->set_line_width(2);
		p->draw_line({x1, (y1+y2)/2}, {x3, (y1+y2)/2});
		p->set_color(theme.green.with_alpha(0.8f));
		draw_arrow(p, {x2, y1 + (y2-y1)*0.2f}, {x3, y1 + (y2-y1)*0.2f});
		draw_arrow(p, {x2, y2 - (y2-y1)*0.2f}, {x3, y2 - (y2-y1)*0.2f});
	} else if (edit_mode == EditMode::ADD_AND_SPLIT) {
		auto h = view->hover();
		if (h.type == HoverData::Type::BAR_GAP) {
		} else if (cur_vlayer()->is_cur_hover() and (h.pos > 0)) {
			p->set_line_width(2);
			p->set_color(theme.green);
			p->draw_line({view->m.x, y1}, {view->m.x, y2});
		}
	}
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
		rubber_hover = false;
		if (view->sel.bar_indices(song).num > 0) {
			float sx = view->cam.sample2screen(rubber_end_target);
			rubber_hover = (fabs(sx - mx) < 10);
		}
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
		if (rubber_hover)
			view->mdp_run(new MouseDelayDragRubberEndPoint(view, &rubber_end_target));
		else
			ViewModeDefault::left_click_handle_void(vlayer);
	} else if (edit_mode == EditMode::ADD_AND_SPLIT) {
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
	if (edit_mode == EditMode::RUBBER)
		return "1. select bars    2. move handle on the right    3. scale [Return]    track [Alt+↑,↓]";
	if (edit_mode == EditMode::ADD_AND_SPLIT)
		return "split/insert bar [click]    move gap [drag'n'drop]    delete/merge [delete]    track [Alt+↑,↓]";
	//if (edit_mode == EditMode::SELECT)
	return "track [Alt+↑,↓]";
}

