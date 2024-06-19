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
#include "../lib/base/optional.h"
#include "../lib/os/path.h"

namespace tsunami {

class Session;

struct SessionPersistenceData {
	Path song_filename;
	Path session_filename;
	Session* session = nullptr;
};

struct SessionLabel {
	enum Flags {
		Active = 1,
		Persistent = 2,
		Backup = 4,
		Recent = 8
	};

	Flags flags;
	Path filename;
	Session *session;
	int uuid;
	bool is_active() const;
	bool is_recent() const;
	bool is_persistent() const;
	bool is_backup() const;
};

class SessionManager : public obs::Node<VirtualBase> {
public:
	SessionManager();
	Session *spawn_new_session();
	Session *get_empty_session(Session *session_caller);
	void end_session(Session *s);

	void save_session(Session *s);
	void save_session(Session *s, const Path& filename);
	Session *load_session(const Path &filename, Session *session_caller = nullptr);
	void load_into_session(SessionPersistenceData *p, Session *session);
	bool try_restore_session(Session *session, const Path &filename);
	void delete_saved_session(const Path &filename);

	static Path directory();

	static bool can_find_associated_session_file(const Path& filename);

	shared_array<Session> active_sessions;

	// cache of all known *.session files (including all in tsunami's session folder)
	owned_array<SessionPersistenceData> known_persistence_data;

	void load_session_map_legacy();
	SessionPersistenceData* find_for_filename(const Path& filename) const;
	SessionPersistenceData* find_for_filename_x(const Path& filename);

	Array<SessionLabel> enumerate_all_sessions() const;
	Array<SessionLabel> enumerate_active_sessions() const;
	Array<SessionLabel> enumerate_recently_used_files() const;
	Array<SessionLabel> enumerate_persistent_sessions() const;
};

}

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
