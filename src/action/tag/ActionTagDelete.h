/*
 * ActionTagDelete.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGDELETE_H_
#define ACTIONTAGDELETE_H_

#include "../../data/Song.h"
#include "../Action.h"

class ActionTagDelete : public Action
{
public:
	ActionTagDelete(int index);
	virtual ~ActionTagDelete();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag;
};

#endif /* ACTIONTAGDELETE_H_ */
