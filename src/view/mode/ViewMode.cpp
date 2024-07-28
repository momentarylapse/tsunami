/*
 * ViewMode.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewMode.h"
#include "../audioview/AudioView.h"
#include "../sidebar/SideBar.h"
#include "../TsunamiWindow.h"

namespace tsunami {

ViewMode::ViewMode(AudioView *_view, const string &name) {
	view = _view;
	mode_name = name;
	cam = &view->cam;
	session = view->session;
	win = view->win;
	song = view->song;
}

HoverData &ViewMode::hover() {
	return view->hover();
}

/*ViewPort &ViewMode::cam() {
	return view->cam;
}*/

HoverData ViewMode::get_hover_data(AudioViewLayer *vlayer, const vec2 &m) {
	return HoverData();
}

SongSelection ViewMode::get_selection(const Range &r, SelectionMode mode) {
	if (mode == SelectionMode::Time)
		return get_selection_for_range(r);
	if (mode == SelectionMode::Rect)
		return get_selection_for_rect(r, hover().y0, hover().y1);
	if (mode == SelectionMode::TrackRect)
		return get_selection_for_track_rect(r, hover().y0, hover().y1);
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_range(const Range &r) {
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_rect(const Range &r, int y0, int y1) {
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_track_rect(const Range &r, int y0, int y1) {
	return SongSelection();
}

void ViewMode::set_side_bar(SideBarIndex console) {
	if ((int)console >= 0)
		win->side_bar->open(console);
	else
		win->side_bar->_hide();
}

}
