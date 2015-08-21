/*
 * ActionSongEditTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONSONGDEDITTAG_H_
#define ACTIONSONGDEDITTAG_H_

#include "../../../Data/Song.h"
#include "../../Action.h"

class ActionSongEditTag : public Action
{
public:
	ActionSongEditTag(int index, const Tag &tag);
	virtual ~ActionSongEditTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag, new_tag;
};

#endif /* ACTIONSONGDEDITTAG_H_ */
