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
	void try_restore_matching_session(Session *session);
	void delete_saved_session(const string &name);

	Path session_path(const string &name) const;
	string session_name(const string &name) const;
	Path directory() const;
	bool session_exists(const string& name) const;

	shared_array<Session> active_sessions;

	void load_session_map();
	base::map<Path, string> session_map;

	Array<SessionLabel> enumerate_all_sessions() const;
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
