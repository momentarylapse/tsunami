/*
 * ActionTrackLayerDelete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#pragma once

#include "../../ActionGroup.h"

class Track;

class ActionTrackLayerDelete : public ActionGroup {
public:
	ActionTrackLayerDelete(Track *t, int index);

	string name() const override { return ":##:delete layer"; }

	void build(Data *d) override;

	Track *track;
	int index;
};
