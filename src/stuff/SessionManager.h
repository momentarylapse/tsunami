/*
 * SessionManager.h
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#ifndef SRC_STUFF_SESSIONMANAGER_H_
#define SRC_STUFF_SESSIONMANAGER_H_

class Session;
class Path;

class SessionManager {
public:
	static Session *create_session();
	static void save_session(Session *s, const Path &filename);
	static Session *load_session(const Path &filename);

	static Path directory();
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
