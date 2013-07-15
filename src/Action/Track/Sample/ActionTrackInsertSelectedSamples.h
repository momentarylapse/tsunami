/*
 * ActionTrackInsertSelectedSamples.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTSELECTEDSAMPLE_H_
#define ACTIONTRACKINSERTSELECTEDSAMPLES_H_

#include "../../ActionGroup.h"
class AudioFile;

class ActionTrackInsertSelectedSamples : public ActionGroup
{
public:
	ActionTrackInsertSelectedSamples(AudioFile *a, int level_no);
	virtual ~ActionTrackInsertSelectedSamples();
};

#endif /* ACTIONTRACKINSERTSELECTEDSAMPLES_H_ */
