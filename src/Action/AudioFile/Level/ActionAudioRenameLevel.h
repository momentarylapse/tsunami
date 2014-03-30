/*
 * ActionAudioRenameLevel.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#ifndef ACTIONAUDIORENAMELEVEL_H_
#define ACTIONAUDIORENAMELEVEL_H_

#include "../../Action.h"

class ActionAudioRenameLevel : public Action
{
public:
	ActionAudioRenameLevel(int index, const string &name);
	virtual ~ActionAudioRenameLevel();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	int index;
	string name;
};

#endif /* ACTIONAUDIORENAMELEVEL_H_ */
