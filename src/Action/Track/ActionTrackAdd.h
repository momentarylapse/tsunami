/*
 * ActionTrackAdd.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADD_H_
#define ACTIONTRACKADD_H_

#include "../Action.h"

class ActionTrackAdd : public Action
{
public:
	ActionTrackAdd(int _index, int _type);
	virtual ~ActionTrackAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index, type;
};

#endif /* ACTIONTRACKADD_H_ */
