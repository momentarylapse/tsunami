/*
 * HeaderBar.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "HeaderBar.h"
#include "TsunamiWindow.h"
#include "ColorScheme.h"
#include "sidebar/SideBar.h"
#include "../Session.h"
#include "../EditModes.h"
#include "../Tsunami.h"
#include "../stuff/SessionManager.h"
#include "../lib/hui/Controls/ControlMenuButton.h"
#include "../lib/hui/Window.h"
#include "../lib/base/iter.h"
#include "mainview/MainView.h"
#include "mainview/MainViewNode.h"


#include "../lib/os/msg.h"

namespace tsunami {

string nice_filename(const Path& f);

HeaderBar::HeaderBar(TsunamiWindow* _win) {
	win = _win;

	win->_add_headerbar();

	// file load/save
	win->set_target(":header:");
	win->add_grid("!box,linked", 0, 0, "file-box");
		win->set_target("file-box");
		win->add_button("!ignorefocus", 0, 0, "new");
		win->set_image("new", "hui:new");
	//	win->add_button("!ignorefocus\\Open", 1, 0, "open");
		win->add_menu_button("!ignorefocus\\Open", 2, 0, "open-menu");
		win->add_button("!ignorefocus", 1, 0, "save");
		win->set_image("save", "hui:save");
		win->add_menu_button("!ignorefocus,width=10", 2, 0, "save-menu");
		win->set_options("save-menu", "menu=header-save-menu");

	// unde/redo
	win->set_target(":header:");
	win->add_grid("!box,linked", 1, 0, "undo-redo-box");
		win->set_target("undo-redo-box");
		win->add_button("!ignorefocus", 0, 0, "undo");
		win->set_image("undo", "hui:undo");
		win->add_button("!ignorefocus", 1, 0, "redo");
		win->set_image("redo", "hui:redo");

	// copy/paste
	win->set_target(":header:");
	win->add_grid("!box,linked", 2, 0, "copy-paste-box");
		win->set_target("copy-paste-box");
		win->add_button("!ignorefocus", 0, 0, "copy");
		win->set_image("copy", "hui:copy");
		win->add_button("!ignorefocus", 0, 0, "paste");
		win->set_image("paste", "hui:paste");
		win->add_menu_button("!ignorefocus,width=10", 1, 0, "edit-menu");
		win->set_options("edit-menu", "menu=header-paste-menu");

	win->set_target(":header:");
	win->add_label("!style=success,bold\\\u2713Session", 3, 0, "session-indicator");
	win->set_tooltip("session-indicator", "Persistent session - plugins and view state will be automatically saved and associated with the audio file");
	win->hide_control("session-indicator", true);

	//----- right side

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

	for (int i=0; i<100; i++) {
		win->event(format("open-recent-%d", i), [this,i] {
			auto files = win->session->session_manager->enumerate_all_sessions();
			if (i < files.num)
				win->load_song_with_session(files[i].filename);
		});
	}
}

void HeaderBar::update() {
	bool editing = win->main_view->active_view->is_editing();
	bool recording = win->main_view->active_view->is_recording();
	win->hide_control("undo-redo-box", !win->side_bar->visible or recording);
	win->hide_control("copy-paste-box", !editing);

	win->hide_control("session-indicator", !win->session->persistence_data);
	win->set_string("session-indicator", "\u2713 (session)");

	auto label = [] (const SessionLabel& l) {
		string xx;
		if (l.is_persistent())
			xx = format("<b><span fgcolor=\"%s\">\u25cf</span></b> ",
			            theme.pitch_text[loop(str(l.filename).hash() + 5, 0, 11)].hex());
		string a, b;
		if (l.is_persistent()) {
			a = "<b>";
			b = "</b>";
		}
		return format(
				"%s%s%s%s<span alpha=\"50%%\">.%s</span>\n <span size=\"x-small\" alpha=\"50%%\">%s</span>",
				xx,
				a,
				l.filename.basename_no_ext(),
				b,
				l.filename.extension(),
				nice_filename(l.filename.dirname()));
	};

	if (auto c = reinterpret_cast<hui::ControlMenuButton*>(win->_get_control_("open-menu"))) {
		menu_load = new hui::Menu(win);
		menu_load->add("Open...", "open");
		menu_load->add_separator();
		auto files = win->session->session_manager->enumerate_all_sessions();
		menu_load->add("<small><i>Recently used sessions:</i></small>", "recent-sessions");
		menu_load->enable("recent-sessions", false);
		for (auto&& [i,l]: enumerate(files)) {
			if (l.is_recent() and !l.is_active() and l.is_persistent())
				menu_load->add(label(l), format("open-recent-%d", i));
		}
		menu_load->add("<small><i>Recently used files:</i></small>", "recent-files");
		menu_load->enable("recent-files", false);
		for (auto&& [i,l]: enumerate(files)) {
			if (l.is_recent() and !l.is_active() and !l.is_persistent())
				menu_load->add(label(l), format("open-recent-%d", i));
		}
		/*if (files.num == 0) {
			menu_load->add(" -none-", "no-recent-files");
			menu_load->enable("no-recent-files", false);
		}*/
		c->set_menu(menu_load.get());
	}
}

}

