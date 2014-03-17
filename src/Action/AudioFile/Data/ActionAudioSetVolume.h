/*
 * ActionAudioSetVolume.h
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#ifndef ACTIONAUDIOSETVOLUME_H_
#define ACTIONAUDIOSETVOLUME_H_

#include "../../ActionMergable.h"

class AudioFile;

class ActionAudioSetVolume : public ActionMergable<float>
{
public:
	ActionAudioSetVolume(AudioFile *a, float volume);
	virtual ~ActionAudioSetVolume();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
};

#endif /* ACTIONAUDIOSETVOLUME_H_ */
