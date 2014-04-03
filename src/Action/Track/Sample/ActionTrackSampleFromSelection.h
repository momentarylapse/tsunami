/*
 * ActionTrackSampleFromSelection.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKSAMPLEFROMSELECTION_H_
#define ACTIONTRACKSAMPLEFROMSELECTION_H_

#include "../../ActionGroup.h"
class AudioFile;
class Track;
class Range;

class ActionTrackSampleFromSelection : public ActionGroup
{
public:
	ActionTrackSampleFromSelection(AudioFile *a, const Range &r, int level_no);
	virtual ~ActionTrackSampleFromSelection();

private:
	void CreateSubsFromTrack(Track *t, const Range &r, int level_no);
};

#endif /* ACTIONTRACKSAMPLEFROMSELECTION_H_ */
