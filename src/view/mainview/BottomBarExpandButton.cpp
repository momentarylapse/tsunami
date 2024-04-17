/*
 * BottomBarExpandButton.cpp
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#include "BottomBarExpandButton.h"
#include "../audioview/AudioView.h"
#include "../helper/Drawing.h"
#include "../bottombar/BottomBar.h"
#include "../TsunamiWindow.h"
#include "../../Session.h"


BottomBarExpandButton::BottomBarExpandButton(Session *_session) : Node(50, 50) {
	align.dz = 200;
	align.horizontal = AlignData::Mode::LEFT;
	align.vertical = AlignData::Mode::BOTTOM;
	set_perf_name("button");
	session = _session;
}
BottomBar *BottomBarExpandButton::bottom_bar() const {
	return session->win->bottom_bar.get();
}
void BottomBarExpandButton::on_draw(Painter *p) {
	color bg = theme.background_overlay;
	color fg = theme.text_soft3;
	float radius = 32;
	if (is_cur_hover()) {
		bg = theme.hoverify(bg);
		fg = theme.text;
		radius = 40;
	}
	p->set_color(bg);
	p->draw_circle(area.center(), radius);
	p->set_color(fg);
	p->set_font_size(theme.FONT_SIZE_HUGE);
	if (bottom_bar()->visible)
		draw_str_centered(p, area.center(),  "\u25bc");
	else
		draw_str_centered(p, area.center(),  "\u25b2");
	p->set_font_size(theme.FONT_SIZE);
}
bool BottomBarExpandButton::on_left_button_down(const vec2 &m) {
	if (bottom_bar()->visible)
		bottom_bar()->_hide();
	else
		bottom_bar()->_show();
	return true;
}
string BottomBarExpandButton::get_tip() const {
	if (bottom_bar()->visible)
		return _("hide control panel");
	else
		return _("show control panel");
}



