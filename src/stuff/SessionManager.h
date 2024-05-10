/*
 * SessionManager.h
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#ifndef SRC_STUFF_SESSIONMANAGER_H_
#define SRC_STUFF_SESSIONMANAGER_H_

#include "../lib/pattern/Observable.h"
#include "../lib/base/pointer.h"
#include "../lib/base/map.h"

class Session;
class Path;

struct SessionLabel {
	enum Flags {
		ACTIVE = 1,
		PERSISTENT = 2,
		BACKUP = 4
	};

	Flags flags;
	string name;
	Session *session;
	int uuid;
	bool is_active() const;
	bool is_persistent() const;
	bool is_backup() const;
};

class SessionManager : public obs::Node<VirtualBase> {
public:
	SessionManager();
	Session *spawn_new_session();
	Session *get_empty_session(Session *session_caller);
	void end_session(Session *s);

	void save_session(Session *s, const string &name);
	Session *load_session(const string &name, Session *session_caller = nullptr);
	void load_into_session(const string &name, Session *session);
	bool try_restore_session_for_song(Session *session, const Path &song_filename);
	void delete_saved_session(const string &name);

	Path session_path(const string &name) const;
	string session_name(const string &name) const;
	static Path directory();
	bool session_exists(const string& name) const;
	bool is_persistent(Session *s) const;

	shared_array<Session> active_sessions;

	void load_session_map_legacy();
	void load_session_map();
	void save_session_map();
	base::map<Path, string> session_map;

	Array<SessionLabel> enumerate_all_sessions() const;
	Array<SessionLabel> enumerate_persistent_sessions() const;
	Array<SessionLabel> enumerate_active_non_persistent_sessions() const;
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
