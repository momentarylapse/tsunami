/*
 * ActionSongAddLevel.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONSONGADDLEVEL_H_
#define ACTIONSONGADDLEVEL_H_

#include "../../Action.h"

class ActionSongAddLevel : public Action
{
public:
	ActionSongAddLevel(const string &name);
	virtual ~ActionSongAddLevel();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	string name;
};

#endif /* ACTIONSONGADDLEVEL_H_ */
