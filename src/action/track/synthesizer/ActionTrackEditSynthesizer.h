/*
 * ActionTrackEditSynthesizer.h
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

struct Track;

class ActionTrackEditSynthesizer: public history::MergableValueAction<string> {
public:
	ActionTrackEditSynthesizer(Track *t);

	string name() const override { return ":##:edit synthesizer"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
	void redo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

}
