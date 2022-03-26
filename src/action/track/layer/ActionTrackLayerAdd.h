/*
 * ActionTrackLayerAdd.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

class Track;
class TrackLayer;

class ActionTrackLayerAdd : public Action {
public:
	ActionTrackLayerAdd(Track *t, shared<TrackLayer> l);

	string name() const override { return ":##:add layer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	shared<TrackLayer> layer;
};
