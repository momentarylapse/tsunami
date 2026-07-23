/*
 * ActionTrackEditMuted.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct Track;

class ActionTrackEditMuted : public history::Action {
public:
	ActionTrackEditMuted(Track *t, bool muted);

	string name() const override { return ":##:mute track"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	bool muted;
	Track *track;
};

}
