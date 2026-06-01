/*
 * ActionTrackCreateBuffers.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_

#include <lib/history/ActionGroup.h>
#include "../../../data/Range.h"

namespace tsunami {

class Track;
class TrackLayer;

class ActionTrackCreateBuffers : public history::ActionGroup {
public:
	ActionTrackCreateBuffers(TrackLayer *l, const Range &r);

	void* compose(history::Data* d) override;

	TrackLayer *layer;
	Range r;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKCREATEBUFFERS_H_ */
