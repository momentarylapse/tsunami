/*
 * ActionTrackEditPanning.h
 *
 *  Created on: 13.03.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class Track;

class ActionTrackEditPanning : public history::MergableValueAction<float> {
public:
	ActionTrackEditPanning(Track *t, float panning);

	string name() const override { return ":##:track panning"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
