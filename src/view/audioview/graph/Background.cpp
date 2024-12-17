/*
 * Background.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "Background.h"
#include "AudioViewLayer.h"
#include "../AudioView.h"
#include "../../ColorScheme.h"
#include "../../mode/ViewMode.h"
#include "../../painter/GridPainter.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../lib/image/Painter.h"

namespace tsunami {

scenegraph::MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode, bool keep_start);

Background::Background(AudioView *_view) : scenegraph::NodeFree() {
	view = _view;
	align.horizontal = AlignData::Mode::Fill;
	align.vertical = AlignData::Mode::Fill;
	set_perf_name("background");
}

bool Background::on_left_button_down(const vec2 &m) {
	if (view->is_playback_active()) {
		view->playback_click();
	} else {
		int pos = view->hover().pos_snap;
		view->set_cursor_pos(pos);
		view->hover().range = Range(pos, 0);

		view->mdp_prepare(CreateMouseDelaySelect(view, SelectionMode::Time, false), m);
	}
	return true;
}

bool Background::on_right_button_down(const vec2 &m) {
	view->open_popup(view->menu_song.get());
	return true;
}

void Background::draw_layer_separator(Painter *c, AudioViewLayer *l1, AudioViewLayer *l2) {
	float y = 0;
	bool sel_any = false;
	bool same_track = false;
	if (l1) {
		y = l1->area.y2;
		sel_any |= view->sel.has(l1->layer);
	}
	if (l2) {
		y = l2->area.y1;
		sel_any |= view->sel.has(l2->layer);
	}
	if (l1 and l2) {
		same_track = (l1->layer->track == l2->layer->track);
	}

	float sx0, sx1;
	view->cam.range2screen_clip(view->sel.range(), area, sx0, sx1);

	c->set_color(theme.grid);

	if (sel_any and !view->sel.range().is_empty()) {
		if (same_track)
			c->set_line_dash({3.0f,10.0f}, 0);
		c->draw_line({area.x1, y}, {sx0, y});

		c->set_color(theme.grid_selected);
		if (same_track)
			c->set_line_dash({3.0f,10.0f}, loop(sx0, 0.0f, 13.0f));
		c->draw_line({sx0, y}, {sx1, y});

		c->set_color(theme.grid);
		if (same_track)
			c->set_line_dash({3.0f,10.0f}, loop(sx1, 0.0f, 13.0f));
		c->draw_line({sx1, y}, {area.x2, y});
	} else {
		if (same_track)
			c->set_line_dash({3.0f,10.0f}, 0);
		c->draw_line({area.x1, y}, {area.x2, y});
	}

	c->set_line_dash({}, 0);

}

void Background::on_draw(Painter* c) {
	int yy = area.y1;
	if (view->vlayers.num > 0)
		yy = view->vlayers.back()->area.y2;

	// tracks
	for (auto *l: view->vlayers)
		if (l->on_screen())
			view->mode->draw_layer_background(c, l);

	// free space below tracks
	if (yy < area.y2) {
		c->set_color(theme.background);
		rect rr = rect(area.x1, area.x2, yy, area.y2);
		GridColors g;
		g.bg = g.bg_sel = theme.background;
		g.fg = g.fg_sel = theme.grid;
		c->draw_rect(rr);
		view->grid_painter->set_context(rr, g);
		if (view->song->bars.num > 0)
			view->grid_painter->draw_bars(c);
		else
			view->grid_painter->draw_time(c);
	}

	// lines between tracks
	AudioViewLayer *prev = nullptr;
	for (auto *l: view->vlayers) {
		draw_layer_separator(c, prev, l);
		prev = l;
	}
	draw_layer_separator(c, prev, nullptr);
}

HoverData Background::get_hover_data(const vec2 &m) {
	auto h = view->hover_time(m);
	h.node = this;
	return h;
}

}
