/*
 * ActionTrackEditSynthesizer.h
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"

namespace tsunami {

class Track;

class ActionTrackEditSynthesizer: public ActionMergable<string> {
public:
	ActionTrackEditSynthesizer(Track *t);

	string name() const override { return ":##:edit synthesizer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
