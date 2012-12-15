/*
 * ActionAudioAddTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIOADDTAG_H_
#define ACTIONAUDIOADDTAG_H_

#include "../../Action.h"
#include "../../../Data/AudioFile.h"

class ActionAudioAddTag : public Action
{
public:
	ActionAudioAddTag(const Tag &tag);
	virtual ~ActionAudioAddTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Tag tag;
};

#endif /* ACTIONAUDIOADDTAG_H_ */
