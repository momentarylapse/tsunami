/*
 * Background.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "Background.h"
#include "AudioViewLayer.h"
#include "../AudioView.h"
#include "../Mode/ViewMode.h"
#include "../Painter/GridPainter.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"


MouseDelayAction* CreateMouseDelaySelect(AudioView *v, SelectionMode mode);

Background::Background(AudioView *_view) : ViewNodeFree() {
	view = _view;
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
}

bool Background::on_left_button_down() {
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

bool Background::on_right_button_down() {
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
	if (same_track)
		c->set_line_dash({3.0f,10.0f}, 0);

	c->set_color(AudioView::colors.grid);
	c->draw_line(area.x1, y, area.x2, y);
	c->set_line_dash({}, 0);

}

void Background::draw(Painter* c) {
	int yy = area.y1;
	if (view->vlayer.num > 0)
		yy = view->vlayer.back()->area.y2;

	// tracks
	for (auto *l: view->vlayer)
		if (l->on_screen())
			view->mode->draw_layer_background(c, l);

	// free space below tracks
	if (yy < area.y2) {
		c->set_color(AudioView::colors.background);
		rect rr = rect(area.x1, area.x2, yy, area.y2);
		GridColors g;
		g.bg = g.bg_sel = view->colors.background;
		g.fg = g.fg_sel = view->colors.grid;
		c->draw_rect(rr);
		view->grid_painter->set_context(rr, g);
		if (view->song->bars.num > 0)
			view->grid_painter->draw_bars(c, 0);
		else
			view->grid_painter->draw_time(c);
	}

	// lines between tracks
	AudioViewLayer *prev = nullptr;
	for (auto *l: view->vlayer) {
		draw_layer_separator(c, prev, l);
		prev = l;
	}
	draw_layer_separator(c, prev, nullptr);
}

HoverData Background::get_hover_data(float mx, float my) {
	auto h = view->hover_time(mx, my);
	h.node = this;
	return h;
}
