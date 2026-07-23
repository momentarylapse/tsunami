/*
 * ActionLayerMerge.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct Track;

class ActionTrackLayerMerge : public history::ActionGroup {
public:
	ActionTrackLayerMerge(Track *t);

	string name() const override { return ":##:merge layers"; }

	void* compose(history::Data* d) override;

	Track *track;
};

}
