/*
 * SessionConsole.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionConsole.h"
#include "../TsunamiWindow.h"
#include "../../lib/os/filesystem.h"
#include "../../stuff/SessionManager.h"
#include "../../stuff/BackupManager.h"
#include "../../storage/Storage.h"
#include "../../data/Song.h"
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

	popup_menu = hui::create_resource_menu("popup-menu-session-manager", this);

	load_data();

	event("session-load", [this] { on_load(); });
	event("session-save", [this] { on_save(); });
	event("session-delete", [this] { on_delete(); });
	event(id_list, [this] { on_list_double_click(); });
	event_x(id_list, "hui:right-button-down", [this] { on_right_click(); });

	tsunami->session_manager->subscribe(this, [this] { load_data(); }, SessionManager::MESSAGE_ANY);
}

SessionConsole::~SessionConsole() {
	tsunami->session_manager->unsubscribe(this);
}

void SessionConsole::find_sessions() {
	session_labels.clear();

	// active
	for (auto s: weak(tsunami->session_manager->sessions))
		session_labels.add({Type::ACTIVE, title_filename(s->song->filename), s, -1});

	// backups
	BackupManager::check_old_files();
	for (auto &f: BackupManager::files)
		session_labels.add({Type::BACKUP, f.filename.str(), nullptr, f.uuid});

	// saved sessions
	auto list = os::fs::search(tsunami->session_manager->directory(), "*.session", "f");
	for (auto &e: list)
		session_labels.add({Type::SAVED, e.no_ext().str(), nullptr, -1});
}

void SessionConsole::on_load() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (l.type == Type::SAVED)
		tsunami->session_manager->load_session(tsunami->session_manager->directory() << (l.name + ".session"));
	else if (l.type == Type::BACKUP)
		load_backup(session, l.name);
}

void SessionConsole::on_save() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (l.type != Type::BACKUP)
		return;
	os::fs::create_directory(tsunami->session_manager->directory());
	hui::file_dialog_save(win, "", tsunami->session_manager->directory(), {"filter=*.session", "showfilter=*.session"}, [this, &l] (const Path &filename) {
		if (filename)
			tsunami->session_manager->save_session(l.session, filename);
	});
}

void SessionConsole::on_delete() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (l.type == Type::BACKUP) {
		BackupManager::delete_old(l.uuid);

		// TODO: make BackupManager observable :P
		tsunami->session_manager->notify();
	} else if (l.type == Type::SAVED) {
		tsunami->session_manager->delete_saved_session(tsunami->session_manager->directory() << (l.name + ".session"));
	}
}

void SessionConsole::on_list_double_click() {
	on_load();
}

void SessionConsole::on_right_click() {
	int n = hui::get_event()->row;
	if (n >= 0) {
		auto &l = session_labels[n];
		popup_menu->enable("session-load", l.type == Type::BACKUP or l.type == Type::SAVED);
		popup_menu->enable("session-delete", l.type == Type::BACKUP or l.type == Type::SAVED);
		popup_menu->enable("session-save", l.type == Type::ACTIVE);
	} else {
		popup_menu->enable("session-load", false);
		popup_menu->enable("session-delete", false);
		popup_menu->enable("session-save", false);
	}
	popup_menu->open_popup(this);
}

void SessionConsole::load_data() {
	find_sessions();
	reset(id_list);
	for (auto &l: session_labels) {
		if (l.type == Type::ACTIVE) {
			if (l.session == session)
				add_string(id_list, format("<b>%s\n      <small>this window's session</small></b>", l.name));
			else
				add_string(id_list, format("%s\n      <small>other window's session</small>", l.name));
		} else if (l.type == Type::SAVED) {
			add_string(id_list, format("<span alpha=\"50%%\">%s</span>\n      <span alpha=\"50%%\"><small>saved session</small></span>", l.name));
		} else if (l.type == Type::BACKUP) {
			add_string(id_list, format("<span color=\"orange\">%s</span>\n      <span color=\"orange\"><small>recording backup</small></span>", l.name));
		}
	}
}

