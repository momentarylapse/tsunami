/*
 * ActionAudio__DeleteTrack.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef ACTIONAUDIO__DELETETRACK_H_
#define ACTIONAUDIO__DELETETRACK_H_

#include "../Action.h"
#include "../../Data/AudioFile.h"

class ActionAudio__DeleteTrack : public Action
{
public:
	ActionAudio__DeleteTrack(int _index);
	virtual ~ActionAudio__DeleteTrack();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int index;
	Track *track;
};

#endif /* ACTIONAUDIO__DELETETRACK_H_ */
