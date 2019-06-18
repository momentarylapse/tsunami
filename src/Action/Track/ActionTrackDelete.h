/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_ACTIONTRACKDELETET_H_
#define SRC_ACTION_TRACK_ACTIONTRACKDELETET_H_

#include "../ActionGroup.h"

class Track;

class ActionTrackDelete : public ActionGroup {
public:
	ActionTrackDelete(Track *track);

	void build(Data *d) override;

	Track *track;
};

#endif /* SRC_ACTION_TRACK_ACTIONTRACKDELETET_H_ */
