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

class SessionManager : public Observable<VirtualBase> {
public:
	Session *spawn_new_session();
	Session *get_empty_session(Session *session_caller);
	void end_session(Session *s);

	void save_session(Session *s, const string &name);
	Session *load_session(const string &name, Session *session_caller = nullptr);
	void delete_saved_session(const string &name);

	Path session_path(const string &name) const;
	string session_name(const string &name) const;
	Path directory() const;

	shared_array<Session> active_sessions;

	Array<SessionLabel> enumerate_all_sessions() const;
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
