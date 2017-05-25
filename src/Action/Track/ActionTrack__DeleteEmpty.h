/*
 * ActionTrack__DeleteEmpty.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__DELETEEMPTY_H_
#define ACTIONTRACK__DELETEEMPTY_H_

#include "../Action.h"
#include "../../Data/Song.h"

class ActionTrack__DeleteEmpty: public Action
{
public:
	ActionTrack__DeleteEmpty(Track *track);
	virtual ~ActionTrack__DeleteEmpty();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Track *track;
};

#endif /* ACTIONTRACK__DELETEEMPTY_H_ */
