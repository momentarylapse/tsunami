/*
 * BottomBarExpandButton.cpp
 *
 *  Created on: 7 May 2023
 *      Author: michi
 */

#include "BottomBarExpandButton.h"
#include "../AudioView.h"
#include "../../bottombar/BottomBar.h"
#include "../../TsunamiWindow.h"
#include "../../../Session.h"


BottomBarExpandButton::BottomBarExpandButton(AudioView *_view) : Node(50, 50) {
	align.dz = 200;
	align.horizontal = AlignData::Mode::LEFT;
	align.vertical = AlignData::Mode::BOTTOM;
	set_perf_name("button");
	view = _view;
}
BottomBar *BottomBarExpandButton::bottom_bar() const {
	return view->session->win->bottom_bar.get();
}
void BottomBarExpandButton::on_draw(Painter *p) {
	color bg = theme.background_overlay;
	color fg = theme.text_soft3;
	if (is_cur_hover()) {
		bg = theme.hoverify(bg);
		fg = theme.text;
	}
	p->set_color(bg);
	p->draw_circle(area.center(), 40);
	p->set_color(fg);
	p->set_font_size(theme.FONT_SIZE_HUGE);
	if (bottom_bar()->visible)
		p->draw_str(area.center() - vec2(10,10),  "\u25bc");
	else
		p->draw_str(area.center() - vec2(10,10),  "\u25b2");
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



