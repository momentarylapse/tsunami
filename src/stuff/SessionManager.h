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
#include "../lib/os/path.h"

class Session;

struct SessionPersistenceData {
	Path song_filename;
	Path session_filename;
	Session* session = nullptr;
};

struct SessionLabel {
	enum Flags {
		ACTIVE = 1,
		PERSISTENT = 2,
		BACKUP = 4
	};

	Flags flags;
	Path session_filename;
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

	void save_session(Session *s);
	void save_session(Session *s, const Path& filename);
	Session *load_session(const Path &filename, Session *session_caller = nullptr);
	void load_into_session(SessionPersistenceData *p, Session *session);
	bool try_restore_session_for_song(Session *session, const Path &song_filename);
	void delete_saved_session(const Path &filename);

	static Path directory();

	shared_array<Session> active_sessions;

	// all *.session files in tsunami's session folder
	owned_array<SessionPersistenceData> session_persistence_data_internal;

	void load_session_map_legacy();
	void load_session_map();
	void save_session_map();
	//base::map<Path, string> session_map;
	SessionPersistenceData* find_for_session(Session* session);
	SessionPersistenceData* find_for_song_filename(const Path& filename);
	SessionPersistenceData* find_for_session_filename(const Path& filename);

	Array<SessionLabel> enumerate_all_sessions() const;
	Array<SessionLabel> enumerate_persistent_sessions() const;
	Array<SessionLabel> enumerate_active_non_persistent_sessions() const;
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
