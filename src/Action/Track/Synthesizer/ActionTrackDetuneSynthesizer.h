/*
 * ActionTrackDetuneSynthesizer.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_SYNTHESIZER_ACTIONTRACKDETUNESYNTHESIZER_H_
#define SRC_ACTION_TRACK_SYNTHESIZER_ACTIONTRACKDETUNESYNTHESIZER_H_

#include "../../Action.h"
#include "../../../Module/Synth/Synthesizer.h"
class Track;

class ActionTrackDetuneSynthesizer: public Action
{
public:
	ActionTrackDetuneSynthesizer(Track *t, int pitch, float dpitch, bool all_octaves);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	Synthesizer::Tuning tuning;
};

#endif /* SRC_ACTION_TRACK_SYNTHESIZER_ACTIONTRACKDETUNESYNTHESIZER_H_ */
