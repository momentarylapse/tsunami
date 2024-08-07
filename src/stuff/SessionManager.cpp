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
#include "../view/mainview/MainView.h"
#include "../view/signaleditor/SignalEditorTab.h"

namespace tsunami {

string title_filename(const Path &filename);
xml::Element signal_chain_to_xml(SignalChain* chain);
xfer<SignalChain> signal_chain_from_xml(Session *session, xml::Element& root);


bool SessionLabel::is_active() const {
	return flags & Flags::Active;
}

bool SessionLabel::is_recent() const {
	return flags & Flags::Recent;
}

bool SessionLabel::is_persistent() const {
	return flags & Flags::Persistent;
}

bool SessionLabel::is_backup() const {
	return flags & Flags::Backup;
}

SessionManager::SessionManager() {
	//os::Timer t;
	load_session_map_legacy();
	//msg_write(f2s(t.get()*1000, 3));
}

Path associated_session_filename(const Path& filename) {
	return filename.with(".tsunami.session");
}

Path associated_song_filename(const Path& filename) {
	const auto s = str(filename);
	if (s.tail(16) == ".tsunami.session")
		return s.sub_ref(0, -16);
	//if (s.tail(8) == ".session")
	//	return s.sub_ref(0, -8);
	return "";
}

bool SessionManager::can_find_associated_session_file(const Path& filename) {
	return os::fs::exists(associated_session_filename(filename));
}

SessionPersistenceData* SessionManager::find_for_filename(const Path &filename) const {
	for (auto &p: known_persistence_data)
		if (p->song_filename == filename or p->song_filename == filename)
			return p;
	return nullptr;
}

SessionPersistenceData* SessionManager::find_for_filename_x(const Path &filename) {
	if (auto p = find_for_filename(filename))
		return p;
	if (filename.extension() == "session") {
		if (os::fs::exists(filename)) {
			auto p = new SessionPersistenceData;
			p->session_filename = filename;
			known_persistence_data.add(p);
			out_changed();
			return p;
		}
	} else if (can_find_associated_session_file(filename)) {
		auto p = new SessionPersistenceData;
		p->song_filename = filename;
		p->session_filename = associated_session_filename(filename);
		known_persistence_data.add(p);
		out_changed();
		return p;
	}
	return nullptr;
}

void SessionManager::load_session_map_legacy() {
	auto list = os::fs::search(directory(), "*.session", "f");
	for (auto &e: list) {
		auto p = new SessionPersistenceData;
		p->session_filename = directory() | e;
		known_persistence_data.add(p);
		if constexpr (false) {
			// correct
			xml::Parser parser;
			try {
				parser.load(directory() | e);
				[[maybe_unused]] auto &e = parser.elements[0];
				if ([[maybe_unused]] auto ef = e.find("file")) {
	//				if (ef->value("path") != "")
	//					session_map[ef->value("path")] = l.name;
				}
			} catch(...) {
			}
		} else {
			// quick'n'dirty
			try {
				auto s = os::fs::read_text(directory() | e);
				int p1 = s.find("<file path=\"");
				if (p1 < 0)
					continue;
				int p2 = s.find("\"", p1 + 13);
				if (p2 < 0)
					continue;
				p->song_filename = s.sub_ref(p1 + 12, p2);
			} catch(...) {
			}
		}
	}
}

Session *SessionManager::spawn_new_session() {
	auto t = Tsunami::instance;
	Session *session = new Session(t->log.get(), t->device_manager.get(), t->plugin_manager.get(), this, t->perf_mon.get());

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
	if (s and s->song->is_empty() and !s->persistence_data)
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
		Tsunami::end();
}

string midi_mode_str(MidiMode m) {
	if (m == MidiMode::Classical)
		return "classical";
	if (m == MidiMode::Tab)
		return "tab";
	if (m == MidiMode::Linear)
		return "linear";
	if (m == MidiMode::Drum)
		return "drum";
	return "?";
}

string audio_mode_str(AudioViewMode m) {
	if (m == AudioViewMode::Peaks)
		return "peaks";
	if (m == AudioViewMode::Spectrum)
		return "spectrum";
	return "?";
}

base::optional<MidiMode> parse_midi_mode(const string& s) {
	if (s == "classical")
		return MidiMode::Classical;
	if (s == "tab")
		return MidiMode::Tab;
	if (s == "linear")
		return MidiMode::Linear;
	if (s == "drum")
		return MidiMode::Drum;
	return base::None;
}

base::optional<AudioViewMode> parse_audio_mode(const string& s) {
	if (s == "peaks")
		return AudioViewMode::Peaks;
	if (s == "spectrum")
		return AudioViewMode::Spectrum;
	return base::None;
}

Path suggest_internal_session_path(Session *s) {
	if (!s->song->filename.is_empty()) {
		// song file
		return associated_session_filename(s->song->filename.absolute());
	} else {
		// no song file
		for (int i=1; i<1000; i++) {
			Path suggest = SessionManager::directory() | format("no-file-%d.session", i);
			if (!os::fs::exists(suggest))
				return suggest;
		}
	}
	return "xxx.session";
}

void SessionManager::save_session(Session *s) {
	if (s->persistence_data)
		save_session(s, s->persistence_data->session_filename);
	else
		save_session(s, suggest_internal_session_path(s));
}

xml::Element audio_view_to_xml(AudioView* view) {
	auto ev = xml::Element("view");
	ev.witha("type", "audioview");
	auto es = xml::Element("selection")
			.witha("start", i2s(view->sel.range().start()))
			.witha("end", i2s(view->sel.range().end()));
	ev.add(es);
	auto ec = xml::Element("viewport")
			.witha("start", i2s(view->cam.range().start()))
			.witha("end", i2s(view->cam.range().end()));
	ev.add(ec);
	auto ets = xml::Element("tracks");
	for (auto&& [i,t]: enumerate(weak(view->song->tracks))) {
		auto et = xml::Element("track").witha("index", i2s(i));
		auto vt = view->get_track(t);
		if (vt->solo)
			et.witha("solo", "true");
		if (t->type == SignalType::Audio)
			et.witha("audio_mode", audio_mode_str(vt->audio_mode));
		else if (t->type == SignalType::Midi)
			et.witha("midi_mode", midi_mode_str(vt->midi_mode_wanted));
		ets.add(et);
	}
	ev.add(ets);
	return ev;
}

xml::Element signal_editor_to_xml(SignalEditorTab* s, int uid) {
	auto ev = xml::Element("view");
	ev.witha("type", "signalchain");
	ev.witha("uid", str(uid));
	return ev;
}

xml::Element main_view_node_to_xml(MainViewNode* v, Session* s) {
	if (v == s->view)
		return audio_view_to_xml(s->view);

	for (auto [uid, c]: enumerate(weak(s->all_signal_chains)))
		if (v->mvn_data() == c)
			return signal_editor_to_xml(reinterpret_cast<SignalEditorTab*>(v), uid);

	return xml::Element("view").witha("type", "unknown");
}

void SessionManager::save_session(Session *s, const Path &filename) {
	os::fs::create_directory(directory());

	if (!s->persistence_data) {
		s->persistence_data = new SessionPersistenceData;
		known_persistence_data.add(s->persistence_data);
		s->persistence_data->session = s;
	}
	s->persistence_data->session_filename = filename;
	if (!s->song->filename.is_empty())
		s->persistence_data->song_filename = s->song->filename.absolute();

	xml::Parser parser;

	auto e = xml::Element("session");
	e.add(xml::Element("head").witha("version", "1.0"));

	// file
	if (s->song->filename)
		e.add(xml::Element("file").witha("path", s->song->filename.absolute().str()));

	// mainview
	auto mview = xml::Element("mainview");
	mview.witha("active", str(weak(s->main_view->views).find(s->main_view->active_view)));
	for (auto v: weak(s->main_view->views))
		mview.add(main_view_node_to_xml(v, s));
	e.add(mview);

	// signal chains
	auto echains = xml::Element("signalchains");
	for (auto [uid, c]: enumerate(weak(s->all_signal_chains)))
		if (c->explicitly_save_for_session)
			echains.add(signal_chain_to_xml(c).witha("uid", i2s(uid)));
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
	parser.save(filename);

	Storage::mark_file_used(filename);
	s->out_changed();
	out_changed();
}


Session *SessionManager::load_session(const Path &filename, Session *session_caller) {
	auto *session = get_empty_session(session_caller);
	if (auto p = find_for_filename_x(filename))
		load_into_session(p, session);
	return session;
}

void audio_view_tracks_from_xml(AudioView* view, xml::Element* ets) {
	for (auto& e: ets->elements) {
		int index = e.value("index", "-1")._int();
		if (index >= 0 and index < view->song->tracks.num) {
			auto t = view->song->tracks[index].get();
			auto vt = view->get_track(t);
			vt->set_solo(e.value("solo", "false")._bool());
			if (auto mm = parse_midi_mode(e.value("midi_mode")))
				vt->set_midi_mode(*mm);
			if (auto am = parse_audio_mode(e.value("audio_mode")))
				vt->set_audio_mode(*am);
		}
	}
}

void audio_view_from_xml(AudioView* view, xml::Element* ev) {
	auto es = ev->find("selection");
	view->sel = SongSelection::from_range(view->song, Range::to(es->value("start")._int(), es->value("end")._int()));
	view->update_selection();
	auto ec = ev->find("viewport");
	view->cam.set_range(Range::to(ec->value("start")._int(), ec->value("end")._int()));

	if (auto ets = ev->find("tracks"))
		audio_view_tracks_from_xml(view, ets);
}

void SessionManager::load_into_session(SessionPersistenceData *p, Session *session) {
	xml::Parser parser;
	parser.load(p->session_filename);
	auto &e = parser.elements[0];

	session->persistence_data = p;
	p->session = session;
	session->win->show();

	Path song_filename;
	if (os::fs::exists(associated_song_filename(p->session_filename))) {
		song_filename = associated_song_filename(p->session_filename);
	} else if (auto ef = e.find("file")) {
		if (ef->value("path") != "")
			song_filename = ef->value("path");
	}
	if (!song_filename.is_empty())
		session->storage->load(session->song.get(), song_filename);

	// deprecated
	if (auto ev = e.find("view"))
		audio_view_from_xml(session->view, ev);
	if (auto ets = e.find("tracks"))
		audio_view_tracks_from_xml(session->view, ets);

	base::map<int, SignalChain*> chain_by_uid;

	// signal chains
	if (auto echains = e.find("signalchains")) {
		for (auto &ec: echains->elements) {
			auto chain = signal_chain_from_xml(session, ec);
			session->add_signal_chain(chain);
			if (ec.value("uid") != "")
				chain_by_uid.set(ec.value("uid")._int(), chain);
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

	// main view
	if (auto emv = e.find("mainview")) {
		for (auto &ep: emv->elements) {
			if (ep.value("type") == "audioview") {
				audio_view_from_xml(session->view, &ep);
			} else if (ep.value("type") == "signalchain") {
				int uid = ep.value("uid")._int();
				if (chain_by_uid.contains(uid))
					session->main_view->_add_view(new SignalEditorTab(chain_by_uid[uid]));
			}
		}
		if (emv->value("active", "") != "") {
			const int n = emv->value("active")._int();
			if (n >= 0 and n < session->main_view->views.num)
				session->main_view->activate_view(weak(session->main_view->views)[n]);
		}
	}

	Storage::mark_file_used(p->session_filename);
	out_changed.notify();
}

// session should be empty
// filename might refer to an audio file OR a session file
bool SessionManager::try_restore_session(Session *session, const Path &filename) {
	auto path = filename.absolute();
	auto p = find_for_filename_x(path);
	if (!p)
		return false;
	load_into_session(p, session);
	session->i(_("session restored: ") + str(p->session_filename));
	return true;
}

void SessionManager::delete_saved_session(const Path& filename) {
	if (auto p = find_for_filename(filename)) {
		if (p->session)
			p->session->persistence_data = nullptr;
		os::fs::_delete(p->session_filename);
		int index = weak(known_persistence_data).find(p);
		known_persistence_data.erase(index);
	}
	out_changed();
}

Path SessionManager::directory() {
	return Tsunami::directory | "sessions";
}

static SessionLabel::Flags operator|(SessionLabel::Flags a, SessionLabel::Flags b) {
	return (SessionLabel::Flags)((int)a | (int)b);
}

Array<SessionLabel> SessionManager::enumerate_active_sessions() const {
	Array<SessionLabel> sessions;
	for (auto s: weak(active_sessions)) {
		if (auto p = s->persistence_data) {
			if (s->song->filename)
				sessions.add({SessionLabel::Flags::Active | SessionLabel::Flags::Persistent, s->song->filename, s, -1});
			else
				sessions.add({SessionLabel::Flags::Active | SessionLabel::Flags::Persistent, p->session_filename, s, -1});
		} else {
			sessions.add({SessionLabel::Flags::Active, s->song->filename, s, -1});
		}
	}
	return sessions;
}

// not active!
Array<SessionLabel> SessionManager::enumerate_recently_used_files() const {
	auto is_active = [this] (const Path& filename) {
		for (auto s: weak(active_sessions)) {
			if (s->song->filename == filename)
				return true;
			if (s->persistence_data and s->persistence_data->session_filename == filename)
				return true;
		}
		return false;
	};
	Array<SessionLabel> sessions;
	for (auto& f: Storage::recently_used_files) {
		if (is_active(f))
			continue;
	//	if (find_for_filename(f))
	//		continue;
		if (can_find_associated_session_file(f)) {
			sessions.add({SessionLabel::Recent | SessionLabel::Persistent, f});
		} else if (f.extension() == "session") {
			auto file = associated_song_filename(f);
			if (file.is_empty())
				file = f;
			sessions.add({SessionLabel::Recent | SessionLabel::Persistent, file});
		} else {
			sessions.add({SessionLabel::Recent, f});
		}
	}
	return sessions;
}

// not active, not recent!
Array<SessionLabel> SessionManager::enumerate_persistent_sessions() const {
	auto is_recent = [this] (const Path& path) {
		return Storage::recently_used_files.find(path) >= 0;
	};
	Array<SessionLabel> sessions;
	for (auto p: known_persistence_data) {
		if (p->session) // active?
			continue;
		if (is_recent(p->session_filename))
			continue;
		sessions.add({SessionLabel::Flags::Persistent, p->session_filename, p->session, -1});
	}
	return sessions;
}

Array<SessionLabel> SessionManager::enumerate_all_sessions() const {
	Array<SessionLabel> sessions;

	sessions.append(enumerate_active_sessions());
	sessions.append(enumerate_recently_used_files());
	sessions.append(enumerate_persistent_sessions());

	// backups
	BackupManager::check_old_files();
	for (auto &f: BackupManager::files)
		sessions.add({SessionLabel::Flags::Backup, f.filename, nullptr, f.uuid});

	return sessions;
}

}
