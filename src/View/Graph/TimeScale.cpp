/*
 * TimeScale.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "../Graph/TimeScale.h"

#include "../AudioView.h"
#include "../Painter/GridPainter.h"
#include "../../Module/Audio/SongRenderer.h"


MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode);





class PlaybackRange : public scenegraph::NodeRel {
public:
	AudioView *view;
	PlaybackRange(TimeScale *time_scale) : scenegraph::NodeRel(0, 0, 0, 0) {
		view = time_scale->view;
		align.horizontal = AlignData::Mode::FILL;
	}
	void update_geometry(const rect &target_area) override {
		area = target_area;
		Range r = view->playback_wish_range;
		if (view->is_playback_active())
			r = view->renderer->range();
		view->cam.range2screen_clip(r, parent->area, area.x1, area.x2);
		hidden = (view->playback_wish_range.length == 0) and !view->is_playback_active();
	}
	void on_draw(Painter *p) override {
		p->set_color(theme.blob_bg_selected);
		if (view->is_playback_active())
			p->set_color(theme.preview_marker);
		p->draw_rect(area.x1, area.y1, area.width(), 5);
	}
	string get_tip() override {
		return _("playback range");
	}
	bool on_left_button_down(float mx, float my) override {
		return parent->on_left_button_down(mx, my);
	}
	bool on_right_button_down(float mx, float my) override {
		view->open_popup(view->menu_playback_range.get());
		return true;
	}
	bool on_left_double_click(float mx, float my) override {
		view->sel = SongSelection::from_range(view->song, view->playback_wish_range).filter(view->sel.layers());
		view->update_selection();
		return true;
	}
	HoverData get_hover_data(float mx, float my)  override {
		auto h = view->hover_time(mx, my);
		h.node = this;
		return h;
	}
};

class PlaybackLockSymbol : public scenegraph::NodeRel {
public:
	AudioView *view;
	PlaybackLockSymbol(TimeScale *time_scale) : scenegraph::NodeRel(0, 0, 20, 20) {
		view = time_scale->view;
	}
	void update_geometry(const rect &target_area) override {
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		area = rect(x, x + 20, area.y1, area.y1 + theme.TIME_SCALE_HEIGHT);
		hidden = !view->playback_range_locked;
	}
	void on_draw(Painter *p) override {
		p->set_color(theme.text);
		if (this->is_cur_hover())
			p->set_color(theme.hoverify(theme.text));
		p->draw_str(area.x1 + 8, area.y1 + 3, u8"\U0001f512");
	}
	string get_tip() override {
		return _("locked");
	}
	bool on_left_button_down(float mx, float my) override {
		view->set_playback_range_locked(false);
		return true;
	}
	bool on_right_button_down(float mx, float my) override {
		view->open_popup(view->menu_playback_range.get());
		return true;
	}
};

class PlaybackLoopSymbol : public scenegraph::NodeRel {
public:
	AudioView *view;
	PlaybackLoopSymbol(TimeScale *time_scale) : scenegraph::NodeRel(0, 0, 20, 20) {
		view = time_scale->view;
	}
	void update_geometry(const rect &target_area) override {
		float x = view->cam.sample2screen(view->get_playback_selection(false).end());
		if (view->playback_range_locked)
			x += 20;
		area = rect(x, x + 20, area.y1, area.y1 + theme.TIME_SCALE_HEIGHT);
		hidden = !view->looping();
	}
	void on_draw(Painter *p) override {
		p->set_color(theme.text);
		if (this->is_cur_hover())
			p->set_color(theme.hoverify(theme.text));
		p->draw_str(area.x1 + 8, area.y1 + 3, u8"\u21bb");
	}
	string get_tip() override {
		return _("looping");
	}
	bool on_left_button_down(float mx, float my) override {
		view->set_playback_loop(false);
		return true;
	}
	bool on_right_button_down(float mx, float my) override {
		view->open_popup(view->menu_playback_range.get());
		return true;
	}
};




TimeScale::TimeScale(AudioView *_view) : scenegraph::NodeRel(0, 0, 100, theme.TIME_SCALE_HEIGHT) {
	align.horizontal = AlignData::Mode::FILL;
	align.dz = 120;
	view = _view;
	set_perf_name("time");

	add_child(new PlaybackRange(this));
	add_child(new PlaybackLockSymbol(this));
	add_child(new PlaybackLoopSymbol(this));
}

void TimeScale::on_draw(Painter* c) {
	GridColors g;
	g.bg = theme.background_track;
	g.bg_sel = g.bg;//colors.background_track_selection;
	g.fg = g.fg_sel = theme.grid;

	auto *gp = view->grid_painter.get();
	gp->set_context(area, g);
	gp->draw_empty_background(c);
	gp->draw_time(c);

	gp->draw_time_numbers(c);
}

bool TimeScale::on_left_button_down(float mx, float my) {
	if (view->is_playback_active()) {
		view->playback_click();
	} else {
		int pos = view->hover().pos_snap;
		view->set_cursor_pos(pos);
		view->hover().range = Range(pos, 0);
		view->mdp_prepare(CreateMouseDelaySelect(view, SelectionMode::TIME));
	}
	return true;
}

bool TimeScale::on_right_button_down(float mx, float my) {
	view->open_popup(view->menu_song.get());
	return true;
}

HoverData TimeScale::get_hover_data(float mx, float my) {
	auto h = view->hover_time(mx, my);
	h.node = this;
	return h;
}
