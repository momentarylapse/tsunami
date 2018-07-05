/*
 * ActionTrackSetSynthesizer.h
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKSETSYNTHESIZER_H_
#define ACTIONTRACKSETSYNTHESIZER_H_

#include "../../Action.h"

class Track;
class Synthesizer;

class ActionTrackSetSynthesizer : public Action
{
public:
	ActionTrackSetSynthesizer(Track *t, Synthesizer *synth);
	virtual ~ActionTrackSetSynthesizer();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	Synthesizer *synth;
};

#endif /* ACTIONTRACKSETSYNTHESIZER_H_ */
