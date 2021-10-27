/*
 * ActionSongChangeAllTrackVolumes.h
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#include "../../ActionMergable.h"

class Song;
class Track;

class ActionSongChangeAllTrackVolumes : public ActionMergable<float> {
public:
	ActionSongChangeAllTrackVolumes(Song *s, Track *t, float volume);

	string name() const override { return ":##:set volume"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

	int track_no;
	Array<float> old_volumes;
};

