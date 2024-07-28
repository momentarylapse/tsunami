/*
 * LayerHeader.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "LayerHeader.h"
#include "AudioViewLayer.h"
#include "AudioViewTrack.h"
#include "../AudioView.h"
#include "../../helper/graph/Node.h"
#include "../../helper/Drawing.h"
#include "../../../data/base.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"

namespace tsunami {

class LayerHeaderButton : public scenegraph::NodeRel {
public:
	AudioViewLayer *vlayer;
	LayerHeader *header;
	LayerHeaderButton(LayerHeader *lh, float dx, float dy) : scenegraph::NodeRel({dx, dy}, 16, 16) {
		vlayer = lh->vlayer;
		header = lh;
	}
	HoverData get_hover_data(const vec2 &m) override {
		auto h = header->get_hover_data(m);
		h.node = this;
		return h;
	}
	virtual void on_click() {}
	bool on_left_button_down(const vec2 &m) override {
		on_click();
		return true;
	}
	color get_color() const {
		if (is_cur_hover())
			return header->color_text();
		return header->color_text().with_alpha(0.7f);
	}
};

class LayerButtonMute: public LayerHeaderButton {
public:
	LayerButtonMute(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		c->set_color(get_color());
		c->draw_mask_image({area.x1, area.y1}, vlayer->view->images.speaker.get());
	}
	void on_click() override {
		vlayer->layer->set_muted(!vlayer->layer->muted);
	}
	string get_tip() const override {
		return _("toggle mute");
	}
};

class LayerButtonSolo: public LayerHeaderButton {
public:
	LayerButtonSolo(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		c->set_color(get_color());
		//c->drawStr(area.x1, area.y1, "S");
		c->draw_mask_image({area.x1, area.y1}, vlayer->view->images.solo.get());
	}
	void on_click() override {
		vlayer->set_solo(!vlayer->solo);
	}
	string get_tip() const override {
		return _("toggle solo");
	}
};

//_("make main version")...

class LayerButtonExplode: public LayerHeaderButton {
public:
	LayerButtonExplode(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		c->set_color(get_color());
		if (vlayer->represents_imploded)
			c->draw_str({area.x1, area.y1}, u8"\u2b73");
		else
			c->draw_str({area.x1, area.y1}, u8"\u2b71");
	}
	void on_click() override {
		if (vlayer->represents_imploded)
			vlayer->view->explode_track(vlayer->layer->track);
		else
			vlayer->view->implode_track(vlayer->layer->track);
	}
	string get_tip() const override {
		return vlayer->represents_imploded ? _("explode") : _("implode");
	}
};

LayerHeader::LayerHeader(AudioViewLayer *l) : scenegraph::NodeRel({0, 0}, theme.LAYER_HANDLE_WIDTH, theme.TRACK_HANDLE_HEIGHT) {
	z = 70;
	align.horizontal = AlignData::Mode::Right;
	set_perf_name("header");
	vlayer = l;
	float x0 = 5;
	float dx = 17;
	add_child(new LayerButtonMute(this, x0, 22));
	add_child(new LayerButtonSolo(this, x0+dx, 22));
	add_child(new LayerButtonExplode(this, x0+dx*2, 22));
}

color LayerHeader::color_bg() const {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	color col;
	if (view->sel.has(layer))
		col = theme.blob_bg_selected;
	else
		col = theme.blob_bg_hidden;
	if (is_cur_hover())
		col = theme.hoverify(col);
	return col;
}

color LayerHeader::color_frame() const {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	color col;
	if (view->sel.has(layer))
		col = theme.blob_bg_selected;
	else
		col = theme.blob_bg;
	if (is_cur_hover())
		col = theme.hoverify(col);
	return col;
}

bool LayerHeader::playable() const {
	return vlayer->view->get_playable_layers().contains(vlayer->layer);
}

color LayerHeader::color_text() const {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	if (playable())
		return theme.blob_text;
	if (view->sel.has(layer))
		return theme.blob_text_soft;
	return theme.blob_text_soft.with_alpha(0.5f);
}

void LayerHeader::update_geometry_recursive(const rect &target_area) {
	auto *view = vlayer->view;
	bool _hover = is_cur_hover();
	bool extended = _hover or view->editing_layer(vlayer);

	align.h = extended ? theme.TRACK_HANDLE_HEIGHT : theme.TRACK_HANDLE_HEIGHT_SMALL;
	align.w = theme.LAYER_HANDLE_WIDTH;
	if (vlayer->represents_imploded)
		align.w = theme.LAYER_HANDLE_WIDTH * 1.6f;
		
		
	for (auto *c: weak(children))
		c->hidden = !extended or vlayer->represents_imploded;
	if (!vlayer->represents_imploded) {
		//children[1]->hidden |= (layer->track->layers.num == 1);
		//children[2]->hidden |= !layer->is_main();
	}
	
	Node::update_geometry_recursive(target_area);
}

void LayerHeader::on_draw(Painter *c) {
	auto *layer = vlayer->layer;
	bool _hover = is_cur_hover();

	c->set_antialiasing(true);
	draw_framed_box(c, area, color_bg(), color_frame(), 1.5f);
	c->set_antialiasing(false);
	
	
	c->set_font("", theme.FONT_SIZE, playable(), false);
	c->set_color(color_text());
	
	if (vlayer->represents_imploded) {
		draw_str_constrained(c, {area.x1 + 5, area.y1 + 5}, area.width() - 10, "explode");
		if (_hover)
			c->draw_str({area.x1 + 25, area.y1 + 25},  u8"\u2b73   \u2b73   \u2b73");
	} else {

		// track title
		string title = "v" + i2s(layer->version_number() + 1);
		if (vlayer->solo)
			title = u8"\u00bb " + title + u8" \u00ab";
		float ww = draw_str_constrained(c, {area.x1 + 5, area.y1 + 5}, area.width() - 10, title);
		if (!playable())
			c->draw_line({area.x1 + 5, area.y1+5+5}, {area.x1+5+ww, area.y1+5+5});
	}

	c->set_font("", -1, false, false);
}

bool LayerHeader::on_left_button_down(const vec2 &m) {
	if (vlayer->represents_imploded) {
		vlayer->view->explode_track(vlayer->layer->track);
		return true;
	}
	auto *view = vlayer->view;
	if (view->selecting_xor()) {
		view->toggle_select_layer_with_content_in_cursor(vlayer);
	} else {
		view->set_current(view->hover());
		if (view->exclusively_select_layer(vlayer)) {
			view->set_selection(view->sel.restrict_to_layer(vlayer->layer));
		} else {
			view->select_under_cursor();
		}
	}
	return true;
}

bool LayerHeader::on_right_button_down(const vec2 &m) {
	auto *view = vlayer->view;
	view->set_current(view->hover());
	if (!view->sel.has(vlayer->layer)) {
		view->exclusively_select_layer(vlayer);
		view->select_under_cursor();
	}
	view->open_popup(view->menu_layer.get());
	return true;
}

HoverData LayerHeader::get_hover_data(const vec2 &m) {
	auto h = Node::get_hover_data(m);
	h.vlayer = vlayer;
	return h;
}

}
