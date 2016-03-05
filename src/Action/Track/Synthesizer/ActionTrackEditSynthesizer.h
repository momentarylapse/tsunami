/*
 * ActionTrackEditSynthesizer.h
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITSYNTHESIZER_H_
#define ACTIONTRACKEDITSYNTHESIZER_H_

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditSynthesizer: public ActionMergable<string>
{
public:
	ActionTrackEditSynthesizer(Track *t, const string &params_old);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
};

#endif /* ACTIONTRACKEDITSYNTHESIZER_H_ */
