/*
 * ActionSubTrackInsert.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTSAMPLE_H_
#define ACTIONTRACKINSERTSAMPLE_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct TrackLayer;

class ActionTrackInsertSample : public history::ActionGroup {
public:
	ActionTrackInsertSample(TrackLayer *layer, int index);

	void* compose(history::Data* d) override;

	TrackLayer *layer;
	int index;
};

}

#endif /* ACTIONTRACKINSERTSAMPLE_H_ */
