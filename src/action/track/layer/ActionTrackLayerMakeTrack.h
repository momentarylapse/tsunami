/*
 * ActionTrackLayerMakeTrack.h
 *
 *  Created on: 14.08.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>

namespace tsunami {

struct TrackLayer;

class ActionTrackLayerMakeTrack : public history::ActionGroup {
public:
	ActionTrackLayerMakeTrack(TrackLayer *layer);

	string name() const override { return ":##:layer -> track"; }

	void* compose(history::Data* d) override;

	TrackLayer *layer;
};

}
