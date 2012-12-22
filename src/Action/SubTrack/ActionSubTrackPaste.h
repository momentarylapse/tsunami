/*
 * ActionSubTrackPaste.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKPASTE_H_
#define ACTIONSUBTRACKPASTE_H_

#include "../Action.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackPaste: public Action
{
public:
	ActionSubTrackPaste(int track_no, int pos, BufferBox *buf);
	virtual ~ActionSubTrackPaste();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Track *sub;
};

#endif /* ACTIONSUBTRACKPASTE_H_ */
