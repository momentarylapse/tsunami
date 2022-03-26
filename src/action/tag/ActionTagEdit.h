/*
 * ActionTagEdit.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONTAGDEDIT_H_
#define ACTIONTAGDEDIT_H_

#include "../../data/Song.h"
#include "../Action.h"

class ActionTagEdit : public Action
{
public:
	ActionTagEdit(int index, const Tag &tag);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag, new_tag;
};

#endif /* ACTIONTAGEDIT_H_ */
