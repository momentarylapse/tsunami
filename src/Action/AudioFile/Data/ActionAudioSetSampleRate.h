/*
 * ActionAudioSetSampleRate.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETSAMPLERATE_H_
#define SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETSAMPLERATE_H_

#include "../../ActionMergable.h"

class AudioFile;

class ActionAudioSetSampleRate : public ActionMergable<int>
{
public:
	ActionAudioSetSampleRate(AudioFile *a, int sample_rate);
	virtual ~ActionAudioSetSampleRate(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
};

#endif /* SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETSAMPLERATE_H_ */
