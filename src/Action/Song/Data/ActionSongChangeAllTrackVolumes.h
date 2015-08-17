/*
 * ActionSongChangeAllTrackVolumes.h
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_DATA_ACTIONSONGCHANGEALLTRACKVOLUMES_H_
#define SRC_ACTION_SONG_DATA_ACTIONSONGCHANGEALLTRACKVOLUMES_H_

#include "../../ActionMergable.h"

class Song;
class Track;

class ActionSongChangeAllTrackVolumes : public ActionMergable<float>
{
public:
	ActionSongChangeAllTrackVolumes(Song *s, Track *t, float volume);
	virtual ~ActionSongChangeAllTrackVolumes(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

	int track_no;
	Array<float> old_volumes;
};

#endif /* SRC_ACTION_SONG_DATA_ACTIONSONGCHANGEALLTRACKVOLUMES_H_ */
