/*
 * ActionTrackDeleteSample.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETESAMPLE_H_
#define ACTIONTRACKDELETESAMPLE_H_

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct SampleRef;

class ActionTrackDeleteSample : public history::ActionGroup {
public:
	ActionTrackDeleteSample(shared<SampleRef> ref);

	void* compose(history::Data* d) override;

private:
	shared<SampleRef> ref;
};

}

#endif /* ACTIONTRACKDELETESAMPLE_H_ */
