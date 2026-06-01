/*
 * ActionTrackLayerAdd.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Track;
class TrackLayer;

class ActionTrackLayerAdd : public history::Action {
public:
	ActionTrackLayerAdd(Track *t, shared<TrackLayer> l);

	string name() const override { return ":##:add layer"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	Track *track;
	shared<TrackLayer> layer;
};

}
