/*
 * ActionSubTrackMove.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKMOVE_H_
#define ACTIONSUBTRACKMOVE_H_

#include "Action.h"
#include "../Data/AudioFile.h"

class ActionSubTrackMove: public Action
{
public:
	ActionSubTrackMove(AudioFile *a);
	virtual ~ActionSubTrackMove();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	// continuous editing
	void abort(Data *d);
	void abort_and_notify(Data *d);
	void set_param_and_notify(Data *d, int _param);

private:
	struct SubSaveData{
		int track_no, sub_no;
		int pos_old;
	};
	Array<SubSaveData> sub;
	int param;
};

#endif /* ACTIONSUBTRACKMOVE_H_ */
