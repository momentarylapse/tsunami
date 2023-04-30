/*
 * SessionManager.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionManager.h"
#include "BackupManager.h"
#include "../lib/base/base.h"
#include "../lib/os/filesystem.h"
#include "../lib/doc/xml.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/SongSelection.h"
#include "../plugins/TsunamiPlugin.h"
#include "../storage/Storage.h"
#include "../view/audioview/AudioView.h"
#include "../view/TsunamiWindow.h"
#include "../Tsunami.h"
#include "../Session.h"

string title_filename(const Path &filename);


bool SessionLabel::is_active() const {
	return flags & Flags::ACTIVE;
}

bool SessionLabel::is_persistent() const {
	return flags & Flags::PERSISTENT;
}

bool SessionLabel::is_backup() const {
	return flags & Flags::BACKUP;
}

Session *SessionManager::spawn_new_session() {
	Session *session = new Session(tsunami->log.get(), tsunami->device_manager.get(), tsunami->plugin_manager.get(), this, tsunami->perf_mon.get());

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	session->set_win(new TsunamiWindow(session));
	session->win->show();

	active_sessions.add(session);
	hui::run_later(0.1f, [this] { notify(); });
	return session;
}

Session *SessionManager::get_empty_session(Session *s) {
	if (s and s->song->is_empty())
		return s;
	return spawn_new_session();
}

void SessionManager::end_session(Session *session) {
	foreachi(Session *s, weak(active_sessions), i)
		if (s == session /*and s->auto_delete*/)
			active_sessions.erase(i);
	hui::run_later(0.1f, [this] { notify(); });

	if (active_sessions.num == 0)
		tsunami->end();
}

void SessionManager::save_session(Session *s, const string &name) {
	xml::Parser parser;

	auto e = xml::Element("session");
	e.add(xml::Element("head").witha("version", "1.0"));

	// file
	e.add(xml::Element("file").witha("path", s->song->filename.absolute().str()));

	// view
	auto ev = xml::Element("view");
	auto es = xml::Element("selection")
			.witha("start", i2s(s->view->sel.range().start()))
			.witha("end", i2s(s->view->sel.range().end()));
	ev.add(es);
	auto ec = xml::Element("viewport")
			.witha("start", i2s(s->view->cam.range().start()))
			.witha("end", i2s(s->view->cam.range().end()));
	ev.add(ec);
	e.add(ev);

	// plugins
	auto epp = xml::Element("plugins");
	for (auto p: weak(s->plugins)) {
		auto ep = xml::Element("plugin")
				.witha("class", p->module_class)
			//	.witha("name", p->module_name)
				.witha("version", i2s(p->version()))
				.witha("config", p->config_to_string());
		epp.add(ep);
	}
	e.add(epp);

	parser.elements.add(e);
	parser.save(session_path(name));

	s->persistent_name = session_name(name);


	notify();
}


Path SessionManager::session_path(const string &name) const {
	// already a full filename?
	if (name.tail(8) == ".session")
		return name;
	return directory() | (name + ".session");
}

string SessionManager::session_name(const string &name) const {
	return session_path(name).basename_no_ext();
}

Session *SessionManager::load_session(const string &name, Session *session_caller) {

	xml::Parser parser;
	parser.load(session_path(name));
	auto &e = parser.elements[0];

	auto *s = get_empty_session(session_caller);
	s->persistent_name = session_name(name);
	s->win->show();

	auto ef = e.find("file");
	s->storage->load(s->song.get(), ef->value("path"));

	auto ev = e.find("view");
	if (ev) {
		auto es = ev->find("selection");
		s->view->sel = SongSelection::from_range(s->song.get(), Range::to(es->value("start")._int(), es->value("end")._int()));
		s->view->update_selection();
		auto ec = ev->find("viewport");
		s->view->cam.set_range(Range::to(ec->value("start")._int(), ec->value("end")._int()));
	}

	auto epp = e.find("plugins");
	if (epp) {
		for (auto &ep: epp->elements) {
			string _class = ep.value("class");
			string config = ep.value("config");
			int version = ep.value("version")._int();

			auto p = s->execute_tsunami_plugin(_class);
			if (p and (config != "")) {
				p->config_from_string(version, config);
				p->notify();
			}
		}
	}

	notify();
	return s;
}

void SessionManager::delete_saved_session(const string &name) {
	os::fs::_delete(session_path(name));
	notify();
}

Path SessionManager::directory() const {
	return tsunami->directory | "sessions";
}

static SessionLabel::Flags operator|(SessionLabel::Flags a, SessionLabel::Flags b) {
	return (SessionLabel::Flags)((int)a | (int)b);
}

Array<SessionLabel> SessionManager::enumerate_all_sessions() const {
	Array<SessionLabel> session_labels;

	auto find_active = [this] (const string &name) -> Session* {
		for (auto s: weak(active_sessions))
				if (s->persistent_name == name)
					return s;
		return nullptr;
	};

	// non-persistent active
	for (auto s: weak(active_sessions))
		if (s->persistent_name == "")
			session_labels.add({SessionLabel::Flags::ACTIVE, title_filename(s->song->filename), s, -1});

	// persistent sessions
	auto list = os::fs::search(tsunami->session_manager->directory(), "*.session", "f");
	for (auto &e: list) {
		auto name = e.no_ext().str();
		if (auto s = find_active(name))
			session_labels.add({SessionLabel::Flags::PERSISTENT | SessionLabel::Flags::ACTIVE, name, s, -1});
		else
			session_labels.add({SessionLabel::Flags::PERSISTENT, name, nullptr, -1});
	}

	// backups
	BackupManager::check_old_files();
	for (auto &f: BackupManager::files)
		session_labels.add({SessionLabel::Flags::BACKUP, f.filename.str(), nullptr, f.uuid});

	return session_labels;
}
