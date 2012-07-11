/*
 * ActionSubTrackDelete.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKDELETE_H_
#define ACTIONSUBTRACKDELETE_H_

#include "../Action.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackDelete : public Action
{
public:
	ActionSubTrackDelete(int _track_no, int _index);
	virtual ~ActionSubTrackDelete();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no, index;
	Track sub;
};

#endif /* ACTIONSUBTRACKDELETE_H_ */
