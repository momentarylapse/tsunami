/*
 * ActionTrackEditVolume.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditVolume : public ActionMergable<float> {
public:
	ActionTrackEditVolume(Track *t, float volume);

	string name() const override { return ":##:set volume"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};
