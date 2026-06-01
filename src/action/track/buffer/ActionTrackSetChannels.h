/*
 * ActionTrackSetChannels.h
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>

namespace tsunami {

class Track;

class ActionTrackSetChannels : public history::ActionGroup {
public:
	ActionTrackSetChannels(Track *t, int channels);
	void* compose(history::Data *d) override;

	string name() const override { return ":##:set channels"; }

private:
	Track *track;
	int channels;
};

}
