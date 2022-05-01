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
class Tsunami;

class SessionManager {
public:
	//static Session *create(Tsunami *tsunami);
	static void save(Session *s, const Path &filename);
	static Session *load(const Path &filename);

	static Path directory();
};

#endif /* SRC_STUFF_SESSIONMANAGER_H_ */
