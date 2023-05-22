/*
 * HeaderBar.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "HeaderBar.h"
#include "TsunamiWindow.h"
#include "sidebar/SideBar.h"
#include "../Session.h"
#include "../EditModes.h"

HeaderBar::HeaderBar(TsunamiWindow* _win) {
	win = _win;

	win->_add_headerbar();
	win->set_target(":header:");
	win->add_grid("!box,linked", 0, 0, "file-box");
		win->set_target("file-box");
		win->add_button("!ignorefocus", 0, 0, "new");
		win->set_image("new", "hui:new");
		win->add_button("!ignorefocus\\Open", 1, 0, "open");
		win->add_button("!ignorefocus", 1, 0, "save");
		win->set_image("save", "hui:save");
		win->add_menu_button("!ignorefocus,width=10", 2, 0, "save-menu");
		win->set_options("save-menu", "menu=header-save-menu");
	win->set_target(":header:");
	win->add_grid("!box,linked", 1, 0, "undo-redo-box");
		win->set_target("undo-redo-box");
		win->add_button("!ignorefocus", 0, 0, "undo");
		win->set_image("undo", "hui:undo");
		win->add_button("!ignorefocus", 1, 0, "redo");
		win->set_image("redo", "hui:redo");
	win->set_target(":header:");
	win->add_grid("!box,linked", 2, 0, "copy-paste-box");
		win->set_target("copy-paste-box");
		win->add_button("!ignorefocus", 0, 0, "copy");
		win->set_image("copy", "hui:copy");
		win->add_menu_button("!ignorefocus,width=10", 1, 0, "paste-menu");
		win->set_options("paste-menu", "menu=header-paste-menu");
		win->set_image("paste-menu", "hui:paste");
	win->set_target(":header:");


	win->add_menu_button("!menu=header-menu,arrow=no", 2, 1, "menu-x");
	win->set_image("menu-x", "hui:open-menu");
	win->add_button("!ignorefocus", 1, 1, "mode-edit-check");
	win->set_image("mode-edit-check", "hui:edit");
	win->add_grid("!box,linked", 0, 1, "sound-box");
		win->set_target("sound-box");
		win->add_button("!flat,ignorefocus", 0, 1, "play");
		win->set_image("play", "hui:media-play");
		win->add_button("!flat,ignorefocus", 1, 1, "pause");
		win->set_image("pause", "hui:media-pause");
		win->add_button("!flat,ignorefocus", 2, 1, "stop");
		win->set_image("stop", "hui:media-stop");
		win->add_button("!flat,ignorefocus", 3, 1, "record");
		win->set_image("record", "hui:media-record");
		win->add_menu_button("!flat,ignorefocus,width=10", 2, 0, "sound-menu");
		win->set_options("sound-menu", "menu=header-sound-menu");
	win->set_target(":header:");
}

void HeaderBar::update() {
	bool editing = win->session->in_mode(EditMode::EditTrack);
	bool recording = win->session->in_mode(EditMode::Capture);
	win->hide_control("undo-redo-box", !win->side_bar->visible or recording);
	win->hide_control("copy-paste-box", !editing);
}

