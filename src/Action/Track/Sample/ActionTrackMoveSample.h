/*
 * ActionTrackMoveSample.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKMOVESAMPLE_H_
#define ACTIONTRACKMOVESAMPLE_H_

#include "../../Action.h"
class Song;
class Track;
class SampleRef;
class SongSelection;

class ActionTrackMoveSample: public Action
{
public:
	ActionTrackMoveSample(Song *a, SongSelection &sel);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	// continuous editing
	virtual void abort(Data *d);
	virtual void abort_and_notify(Data *d);
	virtual void set_param_and_notify(Data *d, int _param);

	virtual bool is_trivial();

private:
	struct SubSaveData{
		Track *track;
		SampleRef *sample;
		int pos_old;
	};
	Array<SubSaveData> sub;
	int param;
};

#endif /* ACTIONTRACKMOVESAMPLE_H_ */
