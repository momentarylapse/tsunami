/*
 * ActionAudioChangeAllTrackVolumes.h
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOCHANGEALLTRACKVOLUMES_H_
#define SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOCHANGEALLTRACKVOLUMES_H_

#include "../../ActionMergable.h"

class AudioFile;
class Track;

class ActionAudioChangeAllTrackVolumes : public ActionMergable<float>
{
public:
	ActionAudioChangeAllTrackVolumes(AudioFile *a, Track *t, float volume);
	virtual ~ActionAudioChangeAllTrackVolumes(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

	int track_no;
	Array<float> old_volumes;
};

#endif /* SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOCHANGEALLTRACKVOLUMES_H_ */
