/*
 * ActionTrackEditSample.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITSAMPLE_H_
#define ACTIONTRACKEDITSAMPLE_H_

#include "../../ActionMergable.h"

namespace tsunami {

class Track;
class SampleRef;

struct EditSampleRefData {
	float volume;
	bool mute;
};

class ActionTrackEditSample : public ActionMergable<EditSampleRefData> {
public:
	ActionTrackEditSample(shared<SampleRef> ref, float volume, bool mute);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	shared<SampleRef> ref;
};

}

#endif /* ACTIONTRACKEDITSAMPLE_H_ */
