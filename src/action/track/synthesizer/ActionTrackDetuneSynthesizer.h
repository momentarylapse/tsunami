/*
 * ActionTrackDetuneSynthesizer.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../data/midi/Temperament.h"

namespace tsunami {

class Track;

class ActionTrackDetuneSynthesizer: public Action {
public:
	ActionTrackDetuneSynthesizer(Track *t, const Temperament &temperament);

	string name() const override { return ":##:detune synthesizer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	Temperament temperament;
};

}
