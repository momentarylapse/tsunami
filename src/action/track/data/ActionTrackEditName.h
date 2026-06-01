/*
 * ActionTrackEditName.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class Track;

class ActionTrackEditName: public history::MergableValueAction<string> {
public:
	ActionTrackEditName(Track *t, const string &name);

	string name() const override { return ":##:set track name"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
