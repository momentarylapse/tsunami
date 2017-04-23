/*
 * ActionTrackInsertSelectedSamples.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTSELECTEDSAMPLE_H_
#define ACTIONTRACKINSERTSELECTEDSAMPLES_H_

#include "../../ActionGroup.h"
class Song;
class SongSelection;

class ActionTrackInsertSelectedSamples : public ActionGroup
{
public:
	ActionTrackInsertSelectedSamples(const SongSelection &sel, int layer_no);

	virtual void build(Data *d);

	const SongSelection &sel;
	int layer_no;
};

#endif /* ACTIONTRACKINSERTSELECTEDSAMPLES_H_ */
