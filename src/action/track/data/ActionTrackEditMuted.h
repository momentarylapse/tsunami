/*
 * ActionTrackEditMuted.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Track;

class ActionTrackEditMuted : public Action {
public:
	ActionTrackEditMuted(Track *t, bool muted);

	string name() const override { return ":##:mute track"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	bool muted;
	Track *track;
};

}
