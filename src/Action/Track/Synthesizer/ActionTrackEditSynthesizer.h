/*
 * ActionTrackEditSynthesizer.h
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITSYNTHESIZER_H_
#define ACTIONTRACKEDITSYNTHESIZER_H_

#include "../../Action.h"
class Track;

class ActionTrackEditSynthesizer: public Action
{
public:
	ActionTrackEditSynthesizer(Track *t, const string &params_old);
	virtual ~ActionTrackEditSynthesizer();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	string params;
	int track_no;
	bool first_execution;
};

#endif /* ACTIONTRACKEDITSYNTHESIZER_H_ */
