/*
 * SessionManager.cpp
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#include "SessionManager.h"
#include "BackupManager.h"
#include "../lib/base/base.h"
#include "../lib/base/iter.h"
#include "../lib/base/optional.h"
#include "../lib/os/filesystem.h"
#include "../lib/os/file.h"
#include "../lib/doc/xml.h"
#include "../data/base.h"
#include "../data/Track.h"
#include "../data/Song.h"
#include "../data/SongSelection.h"
#include "../data/midi/MidiData.h"
#include "../plugins/TsunamiPlugin.h"
#include "../storage/Storage.h"
#include "../view/audioview/AudioView.h"
#include "../view/audioview/graph/AudioViewTrack.h"
#include "../view/TsunamiWindow.h"
#include "../module/SignalChain.h"
#include "../Tsunami.h"
#include "../Session.h"
#include "../Playback.h"

string title_filename(const Path &filename);
xml::Element signal_chain_to_xml(SignalChain* chain);
xfer<SignalChain> signal_chain_from_xml(Session *session, xml::Element& root);


bool SessionLabel::is_active() const {
	return flags & Flags::ACTIVE;
}

bool SessionLabel::is_persistent() const {
	return flags & Flags::PERSISTENT;
}

bool SessionLabel::is_backup() const {
	return flags & Flags::BACKUP;
}

SessionManager::SessionManager() {
	//os::Timer t;
	load_session_map_legacy();
	save_session_map();
	//msg_write(f2s(t.get()*1000, 3));
}

void SessionManager::load_session_map_legacy() {
	auto all = enumerate_all_sessions();
	if constexpr (false) {
		// correct
		for (const auto& l: all)
			if (l.is_persistent()) {
				xml::Parser parser;
				try {
					parser.load(session_path(l.name));
					auto &e = parser.elements[0];
					if (auto ef = e.find("file")) {
						if (ef->value("path") != "")
							session_map[ef->value("path")] = l.name;
					}
				} catch(...) {
				}
			}
	} else {
		// quick'n'dirty
		for (const auto& l: all)
			if (l.is_persistent()) {
				try {
					auto s = os::fs::read_text(session_path(l.name));
					int p1 = s.find("<file path=\"");
					if (p1 < 0)
						continue;
					int p2 = s.find("\"", p1 + 13);
					if (p2 < 0)
						continue;
					session_map.set(s.sub_ref(p1 + 12, p2), l.name);
				} catch(...) {
				}
			}
	}
}

void SessionManager::save_session_map() {
	auto f = os::fs::open(directory() | "index", "wt");

	auto all = enumerate_all_sessions();
	for (const auto& l: all)
		if (l.is_persistent()) {
			f->write_str(l.name);
			Path path;
			for (const auto&& [p, n]: session_map)
				if (n == l.name)
					path = p;
			f->write_str(str(path));
			f->write_int(0);
			f->write_int(0);
		}
}

Session *SessionManager::spawn_new_session() {
	Session *session = new Session(tsunami->log.get(), tsunami->device_manager.get(), tsunami->plugin_manager.get(), this, tsunami->perf_mon.get());

	session->song = new Song(session, DEFAULT_SAMPLE_RATE);

	session->playback = new Playback(session);
	session->set_win(new TsunamiWindow(session));
	session->win->show();

	active_sessions.add(session);
	hui::run_later(0.01f, [this] {
		out_changed.notify();
	});
	return session;
}

Session *SessionManager::get_empty_session(Session *s) {
	if (s and s->song->is_empty() and s->persistent_name == "")
		return s;
	return spawn_new_session();
}

void SessionManager::end_session(Session *session) {
	foreachi(Session *s, weak(active_sessions), i)
		if (s == session /*and s->auto_delete*/)
			active_sessions.erase(i);
	hui::run_later(0.01f, [this] {
		out_changed.notify();
	});

	if (active_sessions.num == 0)
		tsunami->end();
}

string midi_mode_str(MidiMode m) {
	if (m == MidiMode::CLASSICAL)
		return "classical";
	if (m == MidiMode::TAB)
		return "tab";
	if (m == MidiMode::LINEAR)
		return "linear";
	if (m == MidiMode::DRUM)
		return "drum";
	return "?";
}

string audio_mode_str(AudioViewMode m) {
	if (m == AudioViewMode::PEAKS)
		return "peaks";
	if (m == AudioViewMode::SPECTRUM)
		return "spectrum";
	return "?";
}

base::optional<MidiMode> parse_midi_mode(const string& s) {
	if (s == "classical")
		return MidiMode::CLASSICAL;
	if (s == "tab")
		return MidiMode::TAB;
	if (s == "linear")
		return MidiMode::LINEAR;
	if (s == "drum")
		return MidiMode::DRUM;
	return base::None;
}

base::optional<AudioViewMode> parse_audio_mode(const string& s) {
	if (s == "peaks")
		return AudioViewMode::PEAKS;
	if (s == "spectrum")
		return AudioViewMode::SPECTRUM;
	return base::None;
}

void SessionManager::save_session(Session *s, const string &name) {
	os::fs::create_directory(directory());

	s->persistent_name = session_name(name);

	xml::Parser parser;

	auto e = xml::Element("session");
	e.add(xml::Element("head").witha("version", "1.0").witha("name", s->persistent_name));

	// file
	if (s->song->filename)
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
	auto ets = xml::Element("tracks");
	for (auto&& [i,t]: enumerate(weak(s->song->tracks))) {
		auto et = xml::Element("track").witha("index", i2s(i));
		auto vt = s->view->get_track(t);
		if (vt->solo)
			et.witha("solo", "true");
		if (t->type == SignalType::AUDIO)
			et.witha("audio_mode", audio_mode_str(vt->audio_mode));
		else if (t->type == SignalType::MIDI)
			et.witha("midi_mode", midi_mode_str(vt->midi_mode_wanted));
		ets.add(et);
	}
	e.add(ets);
	e.add(ev);

	// signal chains
	auto echains = xml::Element("signalchains");
	for (auto c: weak(s->all_signal_chains))
		if (c->explicitly_save_for_session)
			echains.add(signal_chain_to_xml(c));
	e.add(echains);

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

	save_session_map();

	out_changed.notify();
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

bool SessionManager::session_exists(const string& name) const {
	for (const auto& l: enumerate_all_sessions())
		if (l.is_persistent() and l.name == session_name(name))
			return true;
	return false;
}

bool SessionManager::is_persistent(Session *s) const {
	for (const auto& l: enumerate_all_sessions())
		if (l.session == s and l.is_persistent())
			return true;
	return false;
}

Session *SessionManager::load_session(const string &name, Session *session_caller) {
	auto *session = get_empty_session(session_caller);
	load_into_session(name, session);
	return session;
}

void SessionManager::load_into_session(const string &name, Session *session) {
	xml::Parser parser;
	parser.load(session_path(name));
	auto &e = parser.elements[0];

	session->persistent_name = session_name(name);
	session->win->show();

	if (auto ef = e.find("file")) {
		if (ef->value("path") != "")
			session->storage->load(session->song.get(), ef->value("path"));
	}

	if (auto ev = e.find("view")) {
		auto es = ev->find("selection");
		session->view->sel = SongSelection::from_range(session->song.get(), Range::to(es->value("start")._int(), es->value("end")._int()));
		session->view->update_selection();
		auto ec = ev->find("viewport");
		session->view->cam.set_range(Range::to(ec->value("start")._int(), ec->value("end")._int()));
	}

	if (auto ets = e.find("tracks")) {
		for (auto& e: ets->elements) {
			int index = e.value("index", "-1")._int();
			if (index >= 0 and index < session->song->tracks.num) {
				auto t = session->song->tracks[index].get();
				auto vt = session->view->get_track(t);
				vt->set_solo(e.value("solo", "false")._bool());
				if (auto mm = parse_midi_mode(e.value("midi_mode")))
					vt->set_midi_mode(*mm);
				if (auto am = parse_audio_mode(e.value("audio_mode")))
					vt->set_audio_mode(*am);
			}
		}
	}

	// signal chains
	if (auto echains = e.find("signalchains")) {
		for (auto &ec: echains->elements) {
			auto chain = signal_chain_from_xml(session, ec);
			session->add_signal_chain(chain);
		}
	}

	// plugins
	if (auto epp = e.find("plugins")) {
		for (auto &ep: epp->elements) {
			string _class = ep.value("class");
			string config = ep.value("config");
			int version = ep.value("version")._int();

			auto p = session->execute_tsunami_plugin(_class);
			if (p and (config != "")) {
				p->config_from_string(version, config);
				p->out_changed.notify();
			}
		}
	}

	out_changed.notify();
}

bool SessionManager::try_restore_session_for_song(Session *session, const Path &song_filename) {
	auto path = song_filename.absolute();
	if (session_map.find(path) < 0)
		return false;
	load_into_session(session_map[path], session);
	session->i(_("session automatically restored: ") + session_map[path]);
	return true;
}

void SessionManager::delete_saved_session(const string &name) {
	os::fs::_delete(session_path(name));
	for (auto s: weak(active_sessions))
		if (s->persistent_name == name)
			s->persistent_name = "";
	save_session_map();
	out_changed.notify();
}

Path SessionManager::directory() {
	return Tsunami::directory | "sessions";
}

static SessionLabel::Flags operator|(SessionLabel::Flags a, SessionLabel::Flags b) {
	return (SessionLabel::Flags)((int)a | (int)b);
}

Array<SessionLabel> SessionManager::enumerate_persistent_sessions() const {
	auto find_active = [this] (const string &name) -> Session* {
		for (auto s: weak(active_sessions))
			if (s->persistent_name == name)
				return s;
		return nullptr;
	};

	Array<SessionLabel> sessions;
	auto list = os::fs::search(directory(), "*.session", "f");
	for (auto &e: list) {
		auto name = e.no_ext().str();
		if (auto s = find_active(name))
			sessions.add({SessionLabel::Flags::PERSISTENT | SessionLabel::Flags::ACTIVE, name, s, -1});
		else
			sessions.add({SessionLabel::Flags::PERSISTENT, name, nullptr, -1});
	}
	return sessions;
}

Array<SessionLabel> SessionManager::enumerate_active_non_persistent_sessions() const {
	Array<SessionLabel> sessions;
	for (auto s: weak(active_sessions))
		if (s->persistent_name == "")
			sessions.add({SessionLabel::Flags::ACTIVE, title_filename(s->song->filename), s, -1});
	return sessions;
}

Array<SessionLabel> SessionManager::enumerate_all_sessions() const {
	Array<SessionLabel> session_labels;

	session_labels.append(enumerate_active_non_persistent_sessions());
	session_labels.append(enumerate_persistent_sessions());

	// backups
	BackupManager::check_old_files();
	for (auto &f: BackupManager::files)
		session_labels.add({SessionLabel::Flags::BACKUP, f.filename.str(), nullptr, f.uuid});

	return session_labels;
}
