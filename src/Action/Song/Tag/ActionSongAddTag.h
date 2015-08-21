/*
 * ActionSongAddTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGADDTAG_H_
#define ACTIONSONGADDTAG_H_

#include "../../../Data/Song.h"
#include "../../Action.h"

class ActionSongAddTag : public Action
{
public:
	ActionSongAddTag(const Tag &tag);
	virtual ~ActionSongAddTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Tag tag;
};

#endif /* ACTIONSONGADDTAG_H_ */
