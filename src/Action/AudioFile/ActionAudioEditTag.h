/*
 * ActionAudioEditTag.h
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIODEDITTAG_H_
#define ACTIONAUDIODEDITTAG_H_

#include "../ActionGroup.h"
#include "../../Data/AudioFile.h"

class ActionAudioEditTag : public Action
{
public:
	ActionAudioEditTag(int index, const Tag &tag);
	virtual ~ActionAudioEditTag();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Tag old_tag, new_tag;
};

#endif /* ACTIONAUDIODEDITTAG_H_ */
