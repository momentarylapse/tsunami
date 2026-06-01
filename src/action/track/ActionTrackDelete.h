/*
 * ActionTrackDelete.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>

namespace tsunami {

class Track;

class ActionTrackDelete : public history::ActionGroup {
public:
	explicit ActionTrackDelete(Track *track);

	string name() const override { return ":##:delete track"; }

	void* compose(history::Data* d) override;

	Track *track;
};

}
