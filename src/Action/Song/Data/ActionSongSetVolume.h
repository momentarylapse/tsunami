/*
 * ActionSongSetVolume.h
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#ifndef ACTIONSONGSETVOLUME_H_
#define ACTIONSONGSETVOLUME_H_

#include "../../ActionMergable.h"

class Song;

class ActionSongSetVolume : public ActionMergable<float>
{
public:
	ActionSongSetVolume(Song *s, float volume);
	virtual ~ActionSongSetVolume(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
};

#endif /* ACTIONSONGSETVOLUME_H_ */
