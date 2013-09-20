/*
 * ActionAudioSetVolume.h
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#ifndef ACTIONAUDIOSETVOLUME_H_
#define ACTIONAUDIOSETVOLUME_H_

#include "../../Action.h"

class ActionAudioSetVolume : public Action
{
public:
	ActionAudioSetVolume(float volume);
	virtual ~ActionAudioSetVolume();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	float volume;
};

#endif /* ACTIONAUDIOSETVOLUME_H_ */
