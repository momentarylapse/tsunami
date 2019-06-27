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

class ActionTrackDetuneSynthesizer: public Action {
public:
	ActionTrackDetuneSynthesizer(Track *t, const float tuning[MAX_PITCH]);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	float tuning[MAX_PITCH];
};

#endif /* SRC_ACTION_TRACK_SYNTHESIZER_ACTIONTRACKDETUNESYNTHESIZER_H_ */
