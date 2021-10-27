/*
 * ActionTrackEditPanning.h
 *
 *  Created on: 13.03.2013
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditPanning : public ActionMergable<float> {
public:
	ActionTrackEditPanning(Track *t, float panning);

	string name() const override { return ":##:track panning"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};
