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




class LayerHeaderButton : public ViewNode
{
public:
	AudioViewLayer *vlayer;
	LayerHeaderButton(LayerHeader *lh, float dx, float dy) : ViewNode(lh, dx, dy, 16, 16) {
		vlayer = lh->vlayer;
	}
	color get_color() {
		if (hover())
			return view->colors.text;
		return ColorInterpolate(view->colors.text, view->colors.hover, 0.3f);
	}
};

class LayerButtonMute: public LayerHeaderButton
{
public:
	LayerButtonMute(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
	}
	bool on_left_button_down() override {
		return true;
	}
	string get_tip() override {
		return _("toggle mute");
	}
};

class LayerButtonSolo: public LayerHeaderButton
{
public:
	LayerButtonSolo(LayerHeader *th, float dx, float dy) : LayerHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		//c->drawStr(area.x1, area.y1, "S");
		c->draw_mask_image(area.x1, area.y1, *view->images.solo);
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

class LayerButtonExplode: public LayerHeaderButton
{
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
			view->explode_track(vlayer->layer->track);
		else
			view->implode_track(vlayer->layer->track);
		return true;
	}
	string get_tip() override {
		return vlayer->represents_imploded ? _("explode") : _("implode");
	}
};

LayerHeader::LayerHeader(AudioViewLayer *l) : ViewNode(l, 0, 0, AudioView::LAYER_HANDLE_WIDTH, AudioView::TRACK_HANDLE_HEIGHT) {
	z = 10;
	node_align_right = true;
	vlayer = l;
	float x0 = 5;
	float dx = 17;
	children.add(new LayerButtonMute(this, x0, 22));
	children.add(new LayerButtonSolo(this, x0+dx, 22));
	children.add(new LayerButtonExplode(this, x0+dx*2, 22));
}

void LayerHeader::draw(Painter *c) {

	auto *layer = vlayer->layer;
	bool _hover = hover();
	bool extended = _hover or view->editing_layer(vlayer);
	bool playable = view->get_playable_layers().contains(layer);

	color col = view->colors.background_track_selected;
	if (view->sel.has(vlayer->layer))
		col = ColorInterpolate(col, view->colors.selection, 0.2f);
	if (_hover)
		col = ColorInterpolate(col, view->colors.hover, 0.2f);
	c->set_color(col);
	float h = extended ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	c->set_roundness(view->CORNER_RADIUS);
	c->draw_rect(area.x1,  area.y1,  area.width(), h);
	c->set_roundness(0);

	// track title
	c->set_font("", view->FONT_SIZE, view->sel.has(layer) and playable, false);
	if (playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft2);
	string title;
	if (layer->track->has_version_selection()){
		if (layer->is_main())
			title = _("base");
		else
			title = "v" + i2s(layer->version_number() + 1);
	}else{
		title = "l" + i2s(layer->version_number() + 1);
	}
	if (vlayer->solo)
		title += " (solo)";
	c->draw_str(area.x2 - view->LAYER_HANDLE_WIDTH + 23, area.y1 + 5, title);

	c->set_font("", -1, false, false);

	// icons
	if (layer->type == SignalType::BEATS){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_time); // "⏱"
	}else if (layer->type == SignalType::MIDI){
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_midi); // "♫"
	}else{
		c->set_color(view->colors.text);
		c->draw_mask_image(area.x2 - view->LAYER_HANDLE_WIDTH + 5, area.y1 + 5, *view->images.track_audio); // "∿"
	}



	for (auto *c: children)
		c->hidden = !extended;
	children[1]->hidden |= (view->song->tracks.num == 1) or layer->track->has_version_selection();
	children[2]->hidden |= !layer->is_main();
}

