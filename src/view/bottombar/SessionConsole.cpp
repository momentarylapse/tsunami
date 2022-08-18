/*
 * SessionConsole.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionConsole.h"
#include "../../lib/os/filesystem.h"
#include "../../stuff/SessionManager.h"
#include "../../stuff/BackupManager.h"
#include "../../storage/Storage.h"
#include "../../data/Song.h"
#include "../../TsunamiWindow.h"
#include "../../Tsunami.h"
#include "../../Session.h"

string title_filename(const Path &filename);


void load_backup(Session *session, const Path &filename) {
	if (!filename)
		return;
	Storage::options_in = "format:f32,channels:2,samplerate:44100";
	if (session->song->is_empty()) {
		session->storage->load(session->song.get(), filename);
		//BackupManager::set_save_state(session);
	} else {
		Session *s = tsunami->session_manager->create_session();
		s->win->show();
		s->storage->load(s->song.get(), filename);
	}
	Storage::options_in = "";

	//BackupManager::del
}

SessionConsole::SessionConsole(Session *s, BottomBar *bar) :
	BottomBar::Console(_("Sessions"), "session-console", s, bar)
{
	from_resource("session-console");
	id_list = "sessions";

	load_data();

	event("save", [this] { on_save(); });
	event(id_list, [this] { on_list_double_click(); });

	tsunami->session_manager->subscribe(this, [this] { load_data(); }, SessionManager::MESSAGE_ANY);
}

SessionConsole::~SessionConsole() {
	tsunami->session_manager->unsubscribe(this);
}

void SessionConsole::find_sessions() {
	session_labels.clear();

	// active
	for (auto s: weak(tsunami->session_manager->sessions))
		session_labels.add({Type::ACTIVE, title_filename(s->song->filename), s});

	// backups
	for (auto &f: BackupManager::files)
		session_labels.add({Type::BACKUP, f.filename.str()});

	// saved sessions
	auto list = os::fs::search(tsunami->session_manager->directory(), "*.session", "f");
	for (auto &e: list)
		session_labels.add({Type::SAVED, e.no_ext().str(), nullptr});
}

void SessionConsole::on_save() {
	os::fs::create_directory(tsunami->session_manager->directory());
	hui::file_dialog_save(win, "", tsunami->session_manager->directory(), {"filter=*.session", "showfilter=*.session"}, [this] (const Path &filename) {
		if (filename)
			tsunami->session_manager->save_session(session, filename);
	});
}

void SessionConsole::on_list_double_click() {
	int n = get_int("");
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (l.type == Type::SAVED)
		tsunami->session_manager->load_session(tsunami->session_manager->directory() << (l.name + ".session"));
	else if (l.type == Type::BACKUP)
		load_backup(session, l.name);
}

void SessionConsole::load_data() {
	find_sessions();
	reset(id_list);
	for (auto &l: session_labels) {
		if (l.type == Type::ACTIVE) {
			if (l.session == session)
				add_string(id_list, format("<b><big>%s</big>\n      <small>this window's session</small></b>", l.name));
			else
				add_string(id_list, format("<big>%s</big>\n      <small>other window's session</small>", l.name));
		} else if (l.type == Type::SAVED) {
			add_string(id_list, format("<span alpha=\"50%%\"><big>%s</big></span>\n      <span alpha=\"50%%\"><small>saved session</small></span>", l.name));
		} else if (l.type == Type::BACKUP) {
			add_string(id_list, format("<span color=\"orange\"><big>%s</big></span>\n      <span color=\"orange\"><small>recording backup</small></span>", l.name));
		}
	}
}

