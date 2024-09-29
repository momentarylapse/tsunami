/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#pragma once

#include "../ActionGroup.h"

namespace tsunami {

class Track;

class ActionTrackDelete : public ActionGroup {
public:
	explicit ActionTrackDelete(Track *track);

	string name() const override { return ":##:delete track"; }

	void build(Data *d) override;

	Track *track;
};

}
