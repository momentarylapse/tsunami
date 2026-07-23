/*
 * ActionTrackLayerDelete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct Track;

class ActionTrackLayerDelete : public history::ActionGroup {
public:
	ActionTrackLayerDelete(Track *t, int index);

	string name() const override { return ":##:delete layer"; }

	void* compose(history::Data* d) override;

	Track *track;
	int index;
};

}
