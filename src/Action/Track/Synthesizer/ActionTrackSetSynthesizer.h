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

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	Synthesizer *synth;
};

#endif /* ACTIONTRACKSETSYNTHESIZER_H_ */
