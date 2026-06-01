/*
 * ActionTrackEditVolume.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class Track;

class ActionTrackEditVolume : public history::MergableValueAction<float> {
public:
	ActionTrackEditVolume(Track *t, float volume);

	string name() const override { return ":##:set volume"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
