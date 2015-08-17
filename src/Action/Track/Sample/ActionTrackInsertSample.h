/*
 * ActionSubTrackInsert.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTSAMPLE_H_
#define ACTIONTRACKINSERTSAMPLE_H_

#include "../../ActionGroup.h"
class Song;

class ActionTrackInsertSample : public ActionGroup
{
public:
	ActionTrackInsertSample(Song *a, int track_no, int index, int level_no);
};

#endif /* ACTIONTRACKINSERTSAMPLE_H_ */
