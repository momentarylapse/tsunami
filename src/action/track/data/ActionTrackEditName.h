/*
 * ActionTrackEditName.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"

namespace tsunami {

class Track;

class ActionTrackEditName: public ActionMergable<string> {
public:
	ActionTrackEditName(Track *t, const string &name);

	string name() const override { return ":##:set track name"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
