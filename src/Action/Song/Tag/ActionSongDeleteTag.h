/*
 * ActionSongDeleteTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGDELETETAG_H_
#define ACTIONSONGDELETETAG_H_

#include "../../../Data/Song.h"
#include "../../Action.h"

class ActionSongDeleteTag : public Action
{
public:
	ActionSongDeleteTag(int index);
	virtual ~ActionSongDeleteTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag;
};

#endif /* ACTIONSONGDELETETAG_H_ */
