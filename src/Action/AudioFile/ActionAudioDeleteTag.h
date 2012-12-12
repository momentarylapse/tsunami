/*
 * ActionAudioDeleteTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIODELETETAG_H_
#define ACTIONAUDIODELETETAG_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionAudioDeleteTag : public Action
{
public:
	ActionAudioDeleteTag(int index);
	virtual ~ActionAudioDeleteTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag;
};

#endif /* ACTIONAUDIODELETETAG_H_ */
