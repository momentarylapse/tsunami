/*
 * ActionAudioAddTrack.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIOADDTRACK_H_
#define ACTIONAUDIOADDTRACK_H_

#include "../Action.h"

class ActionAudioAddTrack : public Action
{
public:
	ActionAudioAddTrack(int _index, int _type);
	virtual ~ActionAudioAddTrack();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index, type;
	int old_cur_track;
};

#endif /* ACTIONAUDIOADDTRACK_H_ */
