/*
 * TimeScale.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "TimeScale.h"
#include "../AudioView.h"
#include "../Painter/GridPainter.h"
#include "../../Module/Audio/SongRenderer.h"


MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode);





class PlaybackRange : public ViewNode {
public:
	AudioView *view;
	PlaybackRange(TimeScale *time_scale) : ViewNode(time_scale, 0, 0, 0, 0) {
		view = time_scale->view;
		align.fit_w = true;
	}
	void update_area() override {
		area = parent->area;
		view->cam.range2screen_clip(view->playback_wish_range, parent->area, area.x1, area.x2);
		hidden = (view->playback_wish_range.length == 0);
	}
	void draw(Painter *p) override {
		p->set_color(AudioView::colors.preview_marker);
		p->draw_rect(area.x1, area.y1, area.width(), 5);
	}
	string get_tip() override {
		return _("playback range");
	}
	bool on_left_button_down() override {
		return parent->on_left_button_down();
	}
	bool on_right_button_down() override {
		view->open_popup(view->menu_playback_range);
		return true;
	}
	bool on_left_double_click() override {
		view->sel = SongSelection::from_range(view->song, view->playback_wish_range).filter(view->sel.layers);
		view->update_selection();
		return true;
	}
	HoverData get_hover_data(float mx, float my)  override {
		auto h = view->hover_time(mx, my);
		h.node = this;
		return h;
	}
};

class PlaybackLockSymbol : public ViewNode {
public:
	AudioView *view;
	PlaybackLockSymbol(TimeScale *time_scale) : ViewNode(time_scale, 0, 0, 0, 0) {
		view = time_scale->view;
	}
	void update_area() override {
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		area = rect(x, x + 20, area.y1, area.y1 + AudioView::TIME_SCALE_HEIGHT);
		hidden = !view->playback_range_locked;
	}
	void draw(Painter *p) override {
		//p->set_color((view->hover.type == Selection::Type::PLAYBACK_SYMBOL_LOCK) ? AudioView::colors.hover : AudioView::colors.preview_marker);
		p->set_font_size(AudioView::FONT_SIZE * 0.7f);
		p->draw_str(area.x1 + 8, area.y1 + 3, u8"🔒");
		p->set_font_size(AudioView::FONT_SIZE);
	}
	string get_tip() override {
		return _("locked");
	}
	bool on_left_button_down() override {
		view->set_playback_range_locked(false);
		return true;
	}
	bool on_right_button_down() override {
		view->open_popup(view->menu_playback_range);
		return true;
	}
};

class PlaybackLoopSymbol : public ViewNode {
public:
	AudioView *view;
	PlaybackLoopSymbol(TimeScale *time_scale) : ViewNode(time_scale, 0, 0, 0, 0) {
		view = time_scale->view;
	}
	void update_area() override {
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		if (view->playback_range_locked)
			x += 20;
		area = rect(x, x + 20, area.y1, area.y1 + AudioView::TIME_SCALE_HEIGHT);
		hidden = !view->playback_loop;
	}
	void draw(Painter *p) override {
		//p->set_color((view->hover.type == Selection::Type::PLAYBACK_SYMBOL_LOOP) ? AudioView::colors.hover : AudioView::colors.preview_marker);
		//p->set_font_size(FONT_SIZE * 0.7f);
		p->draw_str(area.x1 + 8, area.y1 + 3, u8"⏎");
		p->set_font_size(AudioView::FONT_SIZE);
	}
	string get_tip() override {
		return _("looping");
	}
	bool on_left_button_down() override {
		view->set_playback_loop(false);
		return true;
	}
	bool on_right_button_down() override {
		view->open_popup(view->menu_playback_range);
		return true;
	}
};




TimeScale::TimeScale(AudioView *_view) {
	align.w = 100;
	align.h = AudioView::TIME_SCALE_HEIGHT;
	z = 20;
	view = _view;

	children.add(new PlaybackRange(this));
	children.add(new PlaybackLockSymbol(this));
	children.add(new PlaybackLoopSymbol(this));
}

void TimeScale::draw(Painter* c) {
	area = view->area;
	area.y2 = align.h;

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
}

bool TimeScale::on_left_button_down() {
	if (view->is_playback_active()) {
		view->playback_click();
	} else {
		int pos = view->hover.pos_snap;
		view->set_cursor_pos(pos);
		view->hover.range = Range(pos, 0);
		view->mdp_prepare(CreateMouseDelaySelect(view, SelectionMode::TIME));
	}
	return true;
}

bool TimeScale::on_right_button_down() {
	view->open_popup(view->menu_song);
	return true;
}

HoverData TimeScale::get_hover_data(float mx, float my) {
	auto h = view->hover_time(mx, my);
	h.node = this;
	return h;
}
