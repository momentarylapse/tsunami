/*
 * ActionTagAdd.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGADD_H_
#define ACTIONTAGADD_H_

#include "../../data/Song.h"
#include "../Action.h"

class ActionTagAdd : public Action
{
public:
	ActionTagAdd(const Tag &tag);
	virtual ~ActionTagAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Tag tag;
};

#endif /* ACTIONTAGADD_H_ */
