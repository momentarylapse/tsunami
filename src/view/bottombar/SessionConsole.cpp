/*
 * SessionConsole.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionConsole.h"
#include "../dialog/QuestionDialog.h"
#include "../TsunamiWindow.h"
#include "../../lib/os/filesystem.h"
#include "../../stuff/SessionManager.h"
#include "../../stuff/BackupManager.h"
#include "../../storage/Storage.h"
#include "../../data/Song.h"
#include "../../Tsunami.h"
#include "../../Session.h"


void load_backup(Session *session_caller, const Path &filename) {
	if (!filename)
		return;
	Storage::options_in = "format:f32,channels:2,samplerate:44100";
	Session *session = tsunami->session_manager->get_empty_session(session_caller);
	session->win->show();
	session->storage->load(session->song.get(), filename);
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

	tsunami->session_manager->out_changed >> create_sink([this] { load_data(); });
}

SessionConsole::~SessionConsole() {
	tsunami->session_manager->unsubscribe(this);
}

void SessionConsole::on_load() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (l.is_persistent())
		tsunami->session_manager->load_session(l.name, session);
	else if (l.is_backup())
		load_backup(session, l.name);
}

void SessionConsole::on_save() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto &l = session_labels[n];
	if (!l.is_active())
		return;

	QuestionDialogString::ask(win, _("Session name"), [this, s=l.session] (const string& name) {
		if (tsunami->session_manager->session_exists(name))
			hui::question_box(win, _("Session already exists"), _("Do you want to overwrite?"), [s, name=name] (const string& answer) {
				if (answer == "hui:yes")
					tsunami->session_manager->save_session(s, name);
			});
		else
			tsunami->session_manager->save_session(s, name);
	});
}

void SessionConsole::on_delete() {
	int n = get_int(id_list);
	if (n < 0)
		return;
	auto l = session_labels[n];
	hui::question_box(win, _("Deleting session"), _("Can not be undone. Are you sure?"), [l] (const string& answer) {
		if (answer == "hui:yes") {
			if (l.is_backup()) {
				BackupManager::delete_old(l.uuid);

				// TODO: make BackupManager observable :P
				tsunami->session_manager->out_changed.notify();
			} else if (l.is_persistent()) {
				tsunami->session_manager->delete_saved_session(l.name);
			}
		}
	});
}

void SessionConsole::on_list_double_click() {
	on_load();
}

void SessionConsole::on_right_click() {
	int n = hui::get_event()->row;
	if (n >= 0) {
		auto &l = session_labels[n];
		popup_menu->enable("session-load", l.is_backup() or l.is_persistent());
		popup_menu->enable("session-delete", l.is_backup() or l.is_persistent());
		popup_menu->enable("session-save", l.is_active());
	} else {
		popup_menu->enable("session-load", false);
		popup_menu->enable("session-delete", false);
		popup_menu->enable("session-save", false);
	}
	popup_menu->open_popup(this);
}

void SessionConsole::load_data() {
	session_labels = tsunami->session_manager->enumerate_all_sessions();
	reset(id_list);
	auto description = [this] (const SessionLabel& l) -> string {
		if (l.is_active() and l.is_persistent()) {
			if (l.session == session)
				return _("this window's session, persistent");
			else
				return _("other window's session, persistent");
		} else if (l.is_active()) {
			if (l.session == session)
				return _("this window's session");
			else
				return _("other window's session");
		} else if (l.is_persistent()) {
			return _("persistent session");
		} else if (l.is_backup()) {
			return _("recording backup");
		}
		return "";
	};
	auto markup = [this] (const SessionLabel& l) -> string {
		if (l.is_active()) {
			if (l.session == session)
				return "weight='bold'";
		} else if (l.is_persistent()) {
			return "alpha=\"50%%\"";
		} else if (l.is_backup()) {
			return "color='orange'";
		}
		return "";
	};
	for (auto &l: session_labels) {
		auto d = description(l);
		auto m = markup(l);
		add_string(id_list, format("<span %s>%s\n      <small>%s</small></span>", m, l.name, d));
	}
}

