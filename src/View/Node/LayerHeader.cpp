/*
 * LayerHeader.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "LayerHeader.h"
#include "ViewNode.h"
#include "AudioViewLayer.h"
#include "AudioViewTrack.h"
#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"




class LayerHeaderButton : public ViewNodeRel {
public:
	AudioViewLayer *vlayer;
	LayerHeader *header;
	LayerHeaderButton(LayerHeader *lh, float dx, float dy) : ViewNodeRel(dx, dy, 16, 16) {
		vlayer = lh->vlayer;
		header = lh;
	}
	HoverData get_hover_data(float mx, float my) override {
		auto h = header->get_hover_data(mx, my);
		h.node = this;
		return h;
	}
	color get_color() {
		if (is_cur_hover())
			return vlayer->view->colors.hoverify(header->color_text());
		return header->color_text();
	}
};

class LayerButtonMute: public LayerHeaderButton {
public:
	LayerButtonMute(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		c->draw_mask_image(area.x1, area.y1, *vlayer->view->images.speaker);
	}
	bool on_left_button_down() override {
		//vlayer->set_mute
		return true;
	}
	string get_tip() override {
		return _("toggle mute");
	}
};

class LayerButtonSolo: public LayerHeaderButton {
public:
	LayerButtonSolo(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		//c->drawStr(area.x1, area.y1, "S");
		c->draw_mask_image(area.x1, area.y1, *vlayer->view->images.solo);
	}
	bool on_left_button_down() override {
		vlayer->set_solo(!vlayer->solo);
		return true;
	}
	string get_tip() override {
		return _("toggle solo");
	}
};

//_("make main version")...

class LayerButtonExplode: public LayerHeaderButton {
public:
	LayerButtonExplode(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		if (vlayer->represents_imploded)
			c->draw_str(area.x1, area.y1, u8"\u2b73");
		else
			c->draw_str(area.x1, area.y1, u8"\u2b71");
	}
	bool on_left_button_down() override {
		if (vlayer->represents_imploded)
			vlayer->view->explode_track(vlayer->layer->track);
		else
			vlayer->view->implode_track(vlayer->layer->track);
		return true;
	}
	string get_tip() override {
		return vlayer->represents_imploded ? _("explode") : _("implode");
	}
};

LayerHeader::LayerHeader(AudioViewLayer *l) : ViewNodeRel(0, 0, AudioView::LAYER_HANDLE_WIDTH, AudioView::TRACK_HANDLE_HEIGHT) {
	z = 10;
	align.horizontal = AlignData::Mode::RIGHT;
	vlayer = l;
	float x0 = 5;
	float dx = 17;
	add_child(new LayerButtonMute(this, x0, 22));
	add_child(new LayerButtonSolo(this, x0+dx, 22));
	add_child(new LayerButtonExplode(this, x0+dx*2, 22));
}

color LayerHeader::color_bg() {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	color col;
	if (view->sel.has(layer))
		col = view->colors.blob_bg_selected;
	else
		col = view->colors.blob_bg_hidden;
	if (is_cur_hover())
		col = view->colors.hoverify(col);
	return col;
}

color LayerHeader::color_frame() {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	color col;
	if (view->sel.has(layer))
		col = view->colors.blob_bg_selected;
	else
		col = view->colors.blob_bg;
	if (is_cur_hover())
		col = view->colors.hoverify(col);
	return col;
}

bool LayerHeader::playable() {
	return vlayer->view->get_playable_layers().contains(vlayer->layer);
}

color LayerHeader::color_text() {
	auto *layer = vlayer->layer;
	auto *view = vlayer->view;
	if (playable())
		return view->colors.text;
	if (view->sel.has(layer)) {
		return view->colors.text_soft1;
	} else {
		return view->colors.text_soft2;
	}
}

void LayerHeader::update_geometry_recursive(const rect &target_area) {
	auto *view = vlayer->view;
	auto *layer = vlayer->layer;
	bool _hover = is_cur_hover();
	bool extended = _hover or view->editing_layer(vlayer);

	align.h = extended ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	align.w = AudioView::LAYER_HANDLE_WIDTH;
	if (vlayer->represents_imploded)
		align.w = AudioView::LAYER_HANDLE_WIDTH * 1.6f;
		
		
	for (auto *c: children)
		c->hidden = !extended or vlayer->represents_imploded;
	if (!vlayer->represents_imploded) {
		children[1]->hidden |= (view->song->tracks.num == 1);
		//children[2]->hidden |= !layer->is_main();
	}
	
	ViewNode::update_geometry_recursive(target_area);
}

void LayerHeader::draw(Painter *c) {
	auto *view = vlayer->view;
	auto *layer = vlayer->layer;
	bool _hover = is_cur_hover();

	float h = area.height();
	c->set_antialiasing(true);
	AudioView::draw_framed_box(c, area, color_bg(), color_frame(), 1.5f);
	c->set_antialiasing(false);
	
	
	c->set_font("", view->FONT_SIZE, playable(), false);
	c->set_color(color_text());
	
	if (vlayer->represents_imploded) {
		AudioView::draw_str_constrained(c, area.x1 + 5, area.y1 + 5, area.width() - 10, "explode");
		if (_hover)
			c->draw_str(area.x1 + 25, area.y1 + 25,  u8"\u2b73   \u2b73   \u2b73");
	} else {

		// track title
		string title = "v" + i2s(layer->version_number() + 1);
		if (vlayer->solo)
			title += " (solo)";
		float ww = AudioView::draw_str_constrained(c, area.x1 + 5, area.y1 + 5, area.width() - 10, title);
		if (!playable())
			c->draw_line(area.x1 + 5, area.y1+5+5, area.x1+5+ww, area.y1+5+5);
	}

	c->set_font("", -1, false, false);
}

bool LayerHeader::on_left_button_down() {
	if (vlayer->represents_imploded) {
		vlayer->view->explode_track(vlayer->layer->track);
		return true;
	}
	auto *view = vlayer->view;
	if (view->select_xor) {
		view->toggle_select_layer_with_content_in_cursor(vlayer);
	} else {
		if (view->exclusively_select_layer(vlayer)) {
			view->set_selection(view->sel.restrict_to_layer(vlayer->layer));
		} else {
			view->select_under_cursor();
		}
	}
	return true;
}

bool LayerHeader::on_right_button_down() {
	auto *view = vlayer->view;
	if (!view->sel.has(vlayer->layer)) {
		view->exclusively_select_layer(vlayer);
		view->select_under_cursor();
	}
	view->open_popup(view->menu_layer);
	return true;
}
HoverData LayerHeader::get_hover_data(float mx, float my) {
	auto *view = vlayer->view;
	auto h = ViewNode::get_hover_data(mx, my);
	h.vtrack = view->get_track(vlayer->layer->track);
	h.vlayer = vlayer;
	return h;
}
