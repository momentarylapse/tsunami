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

class SessionManager : public Observable<VirtualBase> {
public:
	Session *create_session();
	void delete_session(Session *s);

	void save_session(Session *s, const Path &filename);
	Session *load_session(const Path &filename);
	void delete_saved_session(const Path &filename);

	Path directory();


	shared_array<Session> sessions;
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
