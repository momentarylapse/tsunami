/*
 * ActionTrackEditName.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITNAME_H_
#define ACTIONTRACKEDITNAME_H_

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditName: public ActionMergable<string>
{
public:
	ActionTrackEditName(Track *t, const string &name);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

#endif /* ACTIONTRACKEDITNAME_H_ */
