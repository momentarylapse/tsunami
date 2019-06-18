/*
 * ActionTrackEditVolume.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITVOLUME_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITVOLUME_H_

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditVolume : public ActionMergable<float> {
public:
	ActionTrackEditVolume(Track *t, float volume);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITVOLUME_H_ */
