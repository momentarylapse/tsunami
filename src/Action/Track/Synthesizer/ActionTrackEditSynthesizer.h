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

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

#endif /* ACTIONTRACKEDITSYNTHESIZER_H_ */
