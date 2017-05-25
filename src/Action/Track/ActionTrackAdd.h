/*
 * ActionTrackAdd.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADD_H_
#define ACTIONTRACKADD_H_

#include "../Action.h"

class Track;

class ActionTrackAdd : public Action
{
public:
	ActionTrackAdd(int _type, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index, type;
};

#endif /* ACTIONTRACKADD_H_ */
