/*
 * ActionTrackLayerMakeTrack.h
 *
 *  Created on: 14.08.2018
 *      Author: michi
 */

#pragma once

#include "../../ActionGroup.h"

class TrackLayer;

class ActionTrackLayerMakeTrack : public ActionGroup {
public:
	ActionTrackLayerMakeTrack(TrackLayer *layer);

	string name() const override { return ":##:layer -> track"; }

	void build(Data *d) override;

	TrackLayer *layer;
};
