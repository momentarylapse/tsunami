/*
 * ActionSubTrackMove.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#ifndef ACTIONSUBTRACKMOVE_H_
#define ACTIONSUBTRACKMOVE_H_

#include "../Action.h"
#include "../../Data/AudioFile.h"

class ActionSubTrackMove: public Action
{
public:
	ActionSubTrackMove(AudioFile *a);
	virtual ~ActionSubTrackMove();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	// continuous editing
	virtual void abort(Data *d);
	virtual void abort_and_notify(Data *d);
	virtual void set_param_and_notify(Data *d, int _param);

private:
	struct SubSaveData{
		int track_no, sub_no;
		int pos_old;
	};
	Array<SubSaveData> sub;
	int param;
};

#endif /* ACTIONSUBTRACKMOVE_H_ */
