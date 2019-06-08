/*
 * TrackHeader.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "TrackHeader.h"
#include "ViewNode.h"
#include "AudioViewTrack.h"
#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Session.h"







class TrackHeaderButton : public ViewNode
{
public:
	AudioViewTrack *vtrack;
	TrackHeaderButton(TrackHeader *th, float dx, float dy) : ViewNode(th, dx, dy, 16, 16) {
		vtrack = th->vtrack;
	}
	color get_color() {
		if (hover())
			return view->colors.text;
		return ColorInterpolate(view->colors.text, view->colors.hover, 0.3f);
	}
};

class TrackButtonMute: public TrackHeaderButton
{
public:
	TrackButtonMute(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		auto *track = vtrack->track;

		c->set_color(get_color());
		//c->drawStr(area.x1, area.y1-2, "\U0001f50a"); // U+1F50A "🔊"
		c->draw_mask_image(area.x1, area.y1, *view->images.speaker);
		if (track->muted)
			c->draw_image(area.x1, area.y1, *view->images.x);
	}
	bool on_left_button_down() override {
		vtrack->set_muted(!vtrack->track->muted);
		return true;
	}
	string get_tip() override {
		return _("toggle mute");
	}
};

class TrackButtonSolo: public TrackHeaderButton
{
public:
	TrackButtonSolo(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		//c->drawStr(area.x1 + 5 + 17, area.y1 + 22-2, "S");
		c->draw_mask_image(area.x1, area.y1, *view->images.solo);
	}
	bool on_left_button_down() override {
		vtrack->set_solo(!vtrack->solo);
		return true;
	}
	string get_tip() override {
		return _("toggle solo");
	}
};

class TrackButtonConfig: public TrackHeaderButton
{
public:
	TrackButtonConfig(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void draw(Painter *c) override {
		c->set_color(get_color());
		c->draw_str(area.x1, area.y1, u8"\U0001f527"); // U+1F527 "🔧"

		/*c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_FX))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*3, area.y1 + 22-2, "⚡"); // ...*/

		/*c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::Type::TRACK_BUTTON_CURVE))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*4, area.y1 + 22-2, "☊"); // ... */
	}
	bool on_left_button_down() override {
		view->set_cur_layer(view->get_layer(vtrack->track->layers[0]));
		view->session->set_mode("default/track");
		return true;
	}
	string get_tip() override {
		return _("edit track properties");
	}
};

TrackHeader::TrackHeader(AudioViewTrack *t) : ViewNode(t, 0, 0, AudioView::TRACK_HANDLE_WIDTH, AudioView::TRACK_HANDLE_HEIGHT) {
	z = 10;
	vtrack = t;
	float x0 = 5;
	float dx = 17;
	children.add(new TrackButtonMute(this, x0, 22));
	children.add(new TrackButtonSolo(this, x0+dx, 22));
	children.add(new TrackButtonConfig(this, x0+dx*2, 22));
}

void TrackHeader::draw(Painter *c) {
	auto *track = vtrack->track;
	bool _hover = hover();//(view->hover.track == track) and view->hover.is_in(Selection::Type::TRACK_HEADER);
	bool extended = _hover or view->editing_track(track);
	bool playable = view->get_playable_tracks().contains(track);

	color col = view->colors.background_track_selected;
	if (view->sel.has(track))
		col = ColorInterpolate(col, view->colors.selection, 0.4f);
	if (_hover)
		col = ColorInterpolate(col, view->colors.hover, 0.2f);
	c->set_color(col);
	node_height = extended ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	update_area();
	c->set_roundness(view->CORNER_RADIUS);
	c->draw_rect(area);
	c->set_roundness(0);

	// track title
	c->set_font("", view->FONT_SIZE, view->sel.has(track) and playable, false);
	if (playable)
		c->set_color(view->colors.text);
	else
		c->set_color(view->colors.text_soft2);
	c->draw_str(area.x1 + 23, area.y1 + 5, track->nice_name() + (vtrack->solo ? " (solo)" : ""));

	c->set_font("", -1, false, false);

	// icons
	auto *icon = view->images.track_audio;
	if (track->type == SignalType::BEATS)
		icon = view->images.track_time; // "⏱"
	else if (track->type == SignalType::MIDI)
		icon = view->images.track_midi; // "♫"
	c->set_color(view->colors.text);
	c->draw_mask_image(area.x1 + 5, area.y1 + 5, *icon);
//	if (track->muted and !extended)
//		c->draw_image(area.x1 + 5, area.y1 + 5, *view->images.x);

	for (auto *c: children)
		c->hidden = !extended;
	children[1]->hidden |= (view->song->tracks.num == 1);
}
