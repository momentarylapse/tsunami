/*
 * ActionTrack__DeleteEmpty.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONTRACK__DELETEEMPTY_H_
#define ACTIONTRACK__DELETEEMPTY_H_

#include "../Action.h"

class Track;

class ActionTrack__DeleteEmpty: public Action
{
public:
	ActionTrack__DeleteEmpty(Track *track);
	~ActionTrack__DeleteEmpty() override;

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	Track *track;
};

#endif /* ACTIONTRACK__DELETEEMPTY_H_ */
