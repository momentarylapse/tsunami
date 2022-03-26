/*
 * ActionTrackSetChannels.h
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#pragma once

#include "../../ActionGroup.h"

class Track;

class ActionTrackSetChannels : public ActionGroup {
public:
	ActionTrackSetChannels(Track *t, int channels);
	void build(Data *d) override;

	string name() const override { return ":##:set channels"; }

private:
	Track *track;
	int channels;
};
