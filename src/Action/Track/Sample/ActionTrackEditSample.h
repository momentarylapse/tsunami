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

struct EditSampleRefData
{
	float volume;
	bool mute;
	int rep_num;
	int rep_delay;
};

class ActionTrackEditSample : public ActionMergable<EditSampleRefData>
{
public:
	ActionTrackEditSample(Track *t, int index, float volume, bool mute, int rep_num, int rep_delay);
	virtual ~ActionTrackEditSample();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no, index;
};

#endif /* ACTIONTRACKEDITSAMPLE_H_ */
