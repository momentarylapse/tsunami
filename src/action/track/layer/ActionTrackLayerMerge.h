/*
 * ActionLayerMerge.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#pragma once

#include "../../ActionGroup.h"

namespace tsunami {

class Track;

class ActionTrackLayerMerge : public ActionGroup {
public:
	ActionTrackLayerMerge(Track *t);

	string name() const override { return ":##:merge layers"; }

	void build(Data *d) override;

	Track *track;
};

}
