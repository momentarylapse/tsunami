/*
 * ActionAudioAddTrack.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIOADDTRACK_H_
#define ACTIONAUDIOADDTRACK_H_

#include "Action.h"

class ActionAudioAddTrack : public Action
{
public:
	ActionAudioAddTrack(int _index);
	virtual ~ActionAudioAddTrack();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int index;
};

#endif /* ACTIONAUDIOADDTRACK_H_ */
