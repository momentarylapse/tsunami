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
	virtual ~ActionTrackEditName();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
};

#endif /* ACTIONTRACKEDITNAME_H_ */
