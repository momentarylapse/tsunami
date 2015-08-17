/*
 * ActionSongSetSampleRate.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_DATA_ACTIONSONGSETSAMPLERATE_H_
#define SRC_ACTION_SONG_DATA_ACTIONSONGSETSAMPLERATE_H_

#include "../../ActionMergable.h"

class Song;

class ActionSongSetSampleRate : public ActionMergable<int>
{
public:
	ActionSongSetSampleRate(Song *s, int sample_rate);
	virtual ~ActionSongSetSampleRate(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
};

#endif /* SRC_ACTION_SONG_DATA_ACTIONSONGSETSAMPLERATE_H_ */
