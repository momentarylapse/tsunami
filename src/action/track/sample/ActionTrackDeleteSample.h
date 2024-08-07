/*
 * ActionTrackDeleteSample.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETESAMPLE_H_
#define ACTIONTRACKDELETESAMPLE_H_

#include "../../ActionGroup.h"

namespace tsunami {

class SampleRef;

class ActionTrackDeleteSample : public ActionGroup {
public:
	ActionTrackDeleteSample(shared<SampleRef> ref);

	void build(Data *d) override;

private:
	shared<SampleRef> ref;
};

}

#endif /* ACTIONTRACKDELETESAMPLE_H_ */
