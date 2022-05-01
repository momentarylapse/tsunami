/*
 * SessionConsole.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionConsole.h"
#include "../../lib/file/file_op.h"
#include "../../stuff/SessionManager.h"

SessionConsole::SessionConsole(Session *s, BottomBar *bar) :
	BottomBar::Console(_("Session"), "session-console", s, bar)
{
	from_resource("session-console");
	id_list = "sessions";

	load_data();

	event("save", [this] { on_save(); });
	event(id_list, [this] { on_list_double_click(); });
}

SessionConsole::~SessionConsole() {
}

void SessionConsole::on_save() {
	dir_create(SessionManager::directory());
	hui::file_dialog_save(win, "", SessionManager::directory(), {"filter=*.session", "showfilter=*.session"}, [this] (const Path &filename) {
		if (filename)
			SessionManager::save_session(session, filename);
	});
}

void SessionConsole::on_list_double_click() {
	int n = get_int("");
	auto list = dir_search(SessionManager::directory(), "*.session", "f");
	if (n >= 0 and n < list.num)
		SessionManager::load_session(SessionManager::directory() << list[n]);
}

void SessionConsole::load_data() {
	auto list = dir_search(SessionManager::directory(), "*.session", "f");
	reset(id_list);
	for (auto &e: list)
		add_string(id_list, e.no_ext().str());

}

