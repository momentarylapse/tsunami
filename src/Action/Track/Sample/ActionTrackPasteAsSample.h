/*
 * ActionTrackPasteAsSample.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKPASTEASSAMPLE_H_
#define ACTIONTRACKPASTEASSAMPLE_H_

#include "../../ActionGroup.h"

class AudioFile;
class BufferBox;

class ActionTrackPasteAsSample : public ActionGroup
{
public:
	ActionTrackPasteAsSample(AudioFile *a, int track_no, int pos, BufferBox *buf);
	virtual ~ActionTrackPasteAsSample();
};

#endif /* ACTIONTRACKPASTEASSAMPLE_H_ */
