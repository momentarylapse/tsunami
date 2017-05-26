/*
 * ActionTrackEditSample.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITSAMPLE_H_
#define ACTIONTRACKEDITSAMPLE_H_

#include "../../ActionMergable.h"


class Track;
class SampleRef;

struct EditSampleRefData
{
	float volume;
	bool mute;
};

class ActionTrackEditSample : public ActionMergable<EditSampleRefData>
{
public:
	ActionTrackEditSample(SampleRef *ref, float volume, bool mute);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	SampleRef *ref;
};

#endif /* ACTIONTRACKEDITSAMPLE_H_ */
