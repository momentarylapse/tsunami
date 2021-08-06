/*
 * TrackHeader.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "../Graph/TrackHeader.h"

#include "../Helper/Graph/Node.h"
#include "../Helper/Graph/SceneGraph.h"
#include "../Helper/Drawing.h"
#include "../AudioView.h"
#include "../MouseDelayPlanner.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../Graph/AudioViewTrack.h"
#include "../../lib/math/vector.h"



class TrackHeaderButton : public scenegraph::NodeRel {
public:
	AudioView *view;
	AudioViewTrack *vtrack;
	TrackHeader *header;
	TrackHeaderButton(TrackHeader *th, float dx, float dy) : scenegraph::NodeRel({dx, dy}, 16, 16) {
		vtrack = th->vtrack;
		header = th;
		view = th->view;
	}
	HoverData get_hover_data(const vec2 &m) override {
		auto h = header->get_hover_data(m);
		h.node = this;
		return h;
	}
	color get_color() const {
		if (is_cur_hover())
			return theme.hoverify(header->color_text());
		return header->color_text();
	}
};

class TrackButtonMute: public TrackHeaderButton {
public:
	TrackButtonMute(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		auto *track = vtrack->track;

		c->set_color(get_color());
		//c->drawStr(area.x1, area.y1-2, "\U0001f50a");
		c->draw_mask_image({area.x1, area.y1}, view->images.speaker.get());
		if (track->muted) {
			c->set_color(color(1, 0.7f, 0, 0));
			c->draw_mask_image({area.x1, area.y1}, view->images.x.get());
		}
	}
	bool on_left_button_down(const vec2 &m) override {
		vtrack->set_muted(!vtrack->track->muted);
		return true;
	}
	string get_tip() const override {
		return _("toggle mute");
	}
};

class TrackButtonSolo: public TrackHeaderButton {
public:
	TrackButtonSolo(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		c->set_color(get_color());
		//c->drawStr(area.x1 + 5 + 17, area.y1 + 22-2, "S");
		c->draw_mask_image({area.x1, area.y1}, view->images.solo.get());
	}
	bool on_left_button_down(const vec2 &m) override {
		vtrack->set_solo(!vtrack->solo);
		return true;
	}
	string get_tip() const override {
		return _("toggle solo");
	}
};

class TrackButtonConfig: public TrackHeaderButton {
public:
	TrackButtonConfig(TrackHeader *th, float dx, float dy) : TrackHeaderButton(th, dx, dy) {}
	void on_draw(Painter *c) override {
		c->set_color(get_color());
		//c->draw_str(area.x1, area.y1, u8"\U0001f527");
		c->draw_mask_image({area.x1, area.y1}, view->images.config.get());
	}
	bool on_left_button_down(const vec2 &m) override {
		view->session->set_mode(EditMode::DefaultTrack);
		return true;
	}
	string get_tip() const override {
		return _("edit track properties");
	}
};

TrackHeader::TrackHeader(AudioViewTrack *t) : scenegraph::NodeRel({0, 0}, theme.TRACK_HANDLE_WIDTH, theme.TRACK_HANDLE_HEIGHT) {
	align.dz = 70;
	set_perf_name("header");
	vtrack = t;
	view = vtrack->view;
	float x0 = 5;
	float dx = 17;
	add_child(new TrackButtonMute(this, x0, 22));
	add_child(new TrackButtonSolo(this, x0+dx, 22));
	add_child(new TrackButtonConfig(this, x0+dx*2, 22));
}

int group_color(const Track *group) {
	int index = 0;
	for (auto *t: weak(group->song->tracks))
		if (t->type == SignalType::GROUP) {
			if (t == group)
				return (index * 7 + 10) % 12;
			index ++;
		}
	return -1;
}

Array<int> track_group_colors(Track *t) {
	Array<int> c;
	if (t->type == SignalType::GROUP)
		c.add(group_color(t));

	if (t->send_target)
		c.append(track_group_colors(t->send_target));
	return c;
}

color track_header_main_color(Track *track, AudioView *view) {
	auto group_colors = track_group_colors(track);
	if (group_colors.num > 0) {
		return theme.pitch_soft1[group_colors[0]];
	} else {
		return theme.blob_bg_selected;
	}
}

color TrackHeader::color_bg(bool allow_hover) const {
	auto *track = vtrack->track;
	color col = track_header_main_color(track, view);
	if (!view->sel.has(track)) {
		if ((track->type == SignalType::GROUP) or track->send_target)
			col = color::interpolate(col, theme.background, 0.6f);
		else
			col = theme.blob_bg_hidden;
	}
	if (is_cur_hover() and allow_hover)
		col = theme.hoverify(col);
	return col;
}


bool TrackHeader::playable() const {
	auto *track = vtrack->track;
	return view->get_playable_tracks().contains(track);
}

color TrackHeader::color_text() const {
	if (playable())
		return theme.text;
	if (view->sel.has(vtrack->track))
		return theme.text_soft1;
	else
		return theme.text_soft1.with_alpha(0.5f);
}


void TrackHeader::update_geometry_recursive(const rect &target_area) {
	auto *track = vtrack->track;
	bool _hover = is_cur_hover();
	bool extended = _hover or view->editing_track(track);

	align.h = extended ? theme.TRACK_HANDLE_HEIGHT : theme.TRACK_HANDLE_HEIGHT_SMALL;
		
	for (auto *c: weak(children))
		c->hidden = !extended;
	children[1]->hidden |= (view->song->tracks.num == 1);
	
	Node::update_geometry_recursive(target_area);
}

string TrackHeader::nice_title() const {
	string title = vtrack->track->nice_name();
	if (vtrack->solo)
		title = u8"\u00bb " + title + u8" \u00ab";
	return title;
}

void TrackHeader::on_draw(Painter *c) {
	auto *track = vtrack->track;

	auto group_colors = track_group_colors(track);
	foreachi (auto g, group_colors, i) {
		c->set_color(theme.pitch_soft1[g]);//(gc));
		float x = vtrack->area.x1 + 5*(group_colors.num - i - 1);
		c->draw_rect(rect(x, x + 5, vtrack->area.y1, vtrack->area.y2));
		//MidiPainter::pitch_color(track->send_target->nice_name().hash() % MAX_PITCH)
	}

	c->set_antialiasing(true);
	draw_box(c, area, color_bg());
	c->set_antialiasing(false);

	// track title
	c->set_font("", theme.FONT_SIZE, playable(), false);
	c->set_color(color_text());
	string title = nice_title();
	draw_str_constrained(c, {area.x1 + 23, area.y1 + 5}, area.width() - 25, title);
	if (!playable()) {
		float ww = c->get_str_width(title);
		c->set_line_width(1.7f);
		c->draw_line({area.x1 + 23, area.y1+5+5}, {area.x1+23+ww, area.y1+5+5});
		c->set_line_width(1);
	}
	c->set_font("", -1, false, false);

	// icons
	auto *icon = view->images.track_audio.get();
	if (track->type == SignalType::BEATS)
		icon = view->images.track_time.get();
	else if (track->type == SignalType::MIDI)
		icon = view->images.track_midi.get();
	else if (track->type == SignalType::GROUP)
		icon = view->images.track_group.get();
	c->draw_mask_image({area.x1 + 5, area.y1 + 5}, icon);
}

bool track_is_in_group(Track *t, Track *g);

class MouseDelayDndTrack : public MouseDelayAction {
public:
	AudioViewTrack *vtrack;
	AudioView *view;
	MouseDelayDndTrack(AudioViewTrack *t) {
		vtrack = t;
		view = vtrack->view;
	}
	void on_draw_post(Painter *c) override {
		scene_graph->update_hover();
		//int orig = get_track_index(moving_track);
		int t = get_track_move_target(true);
		float y = view->vtracks.back()->area.y2;
		if (t < view->vtracks.num)
			y = view->vtracks[t]->area.y1;

		c->set_color(theme.selection_boundary);
		c->set_line_width(2.0f);
		c->draw_line({view->area.x1, y}, {view->area.x2, y});
		c->set_line_width(1.0f);

		/*c->setColor(colors.selection_internal);
		rect r = view->vtrack[orig]->area;
		r.x2 = view->TRACK_HANDLE_WIDTH;
		c->drawRect(r);*/

		auto g = get_target_group();
		if (g) {
			c->set_color(theme.pitch[group_color(g)].with_alpha(0.2f));
			for (auto gt: weak(view->song->tracks))
				if (track_is_in_group(gt, g)) {
					auto gvt = view->get_track(gt);
					c->draw_rect(gvt->area);
				}
		}

		view->draw_cursor_hover(c, vtrack->track->nice_name());
	}
	void on_finish(const vec2 &m) override {
		int target = get_track_move_target(false);
		auto g = get_target_group();
		vtrack->track->song->begin_action_group();
		vtrack->track->move(target);
		vtrack->track->set_send_target(g);
		vtrack->track->song->end_action_group();
	}

	int get_track_move_target(bool visual) {
		int orig = get_track_index(vtrack->track);
		foreachi(auto vt, view->vtracks, i) {
			int y = (vt->area.y1 + vt->area.y2) / 2;
			if (y > view->m.y) {
				if (visual or (i <= orig))
					return i;
				else
					return i - 1;
			}
		}
		return visual ? view->song->tracks.num : (view->song->tracks.num-1);
	}
	Track *get_target_group() {
		int target = get_track_move_target(true);
		auto v = view->hover().vtrack;
		if (!v)
			return nullptr;
		if (v->track->type == SignalType::GROUP) {
			if (target == get_track_index(v->track))
				return v->track->send_target;
			return v->track;
		}
		if (v->track->send_target)
			return v->track->send_target;
		return nullptr;
	}
};

bool TrackHeader::on_left_button_down(const vec2 &m) {
	if (view->select_xor) {
		view->toggle_select_track_with_content_in_cursor(vtrack);
	} else {
		if (view->sel.has(vtrack->track)) {
			view->set_selection(view->sel.restrict_to_track(vtrack->track));
		} else {
			view->exclusively_select_track(vtrack);
			view->select_under_cursor();
		}
	}
	view->mdp_prepare(new MouseDelayDndTrack(vtrack));
	return true;
}
bool TrackHeader::on_left_double_click(const vec2 &m) {
	view->session->set_mode(EditMode::DefaultTrack);
	return true;
}

bool TrackHeader::on_right_button_down(const vec2 &m) {
	if (!view->sel.has(vtrack->track)) {
		view->exclusively_select_layer(vtrack->first_layer());
		view->select_under_cursor();
	}
	view->open_popup(view->menu_track.get());
	return true;
}

HoverData TrackHeader::get_hover_data(const vec2 &m) {
	auto h = Node::get_hover_data(m);
	h.vtrack = vtrack;
	h.vlayer = vtrack->first_layer();
	return h;
}
