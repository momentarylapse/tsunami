/*
 * ActionSongRenameLevel.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#ifndef ACTIONSONGRENAMELEVEL_H_
#define ACTIONSONGRENAMELEVEL_H_

#include "../../Action.h"

class ActionSongRenameLevel : public Action
{
public:
	ActionSongRenameLevel(int index, const string &name);
	virtual ~ActionSongRenameLevel();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	int index;
	string name;
};

#endif /* ACTIONSONGRENAMELEVEL_H_ */
