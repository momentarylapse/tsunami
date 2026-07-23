/*
 * ActionTrackDetuneSynthesizer.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/midi/Temperament.h"

namespace tsunami {

struct Track;

class ActionTrackDetuneSynthesizer: public history::Action {
public:
	ActionTrackDetuneSynthesizer(Track *t, const Temperament &temperament);

	string name() const override { return ":##:detune synthesizer"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Track *track;
	Temperament temperament;
};

}
