/*
 * ActionTrackEditSample.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITSAMPLE_H_
#define ACTIONTRACKEDITSAMPLE_H_

#include <lib/history/MergableAction.h>

namespace tsunami {

class Track;
class SampleRef;

struct EditSampleRefData {
	float volume;
	bool mute;
};

class ActionTrackEditSample : public history::MergableValueAction<EditSampleRefData> {
public:
	ActionTrackEditSample(shared<SampleRef> ref, float volume, bool mute);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	shared<SampleRef> ref;
};

}

#endif /* ACTIONTRACKEDITSAMPLE_H_ */
