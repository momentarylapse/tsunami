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
	ActionTrackAdd(Track *t, int index);
	~ActionTrackAdd();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	int index;
};

#endif /* ACTIONTRACKADD_H_ */
