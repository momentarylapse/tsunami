/*
 * ActionLayerMerge.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayerMerge.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../buffer/ActionTrackEditBuffer.h"
#include "../buffer/ActionTrackCreateBuffers.h"
#include "ActionTrackLayerDelete.h"
#include "ActionTrackLayerAdd.h"

#include "../../../module/audio/TrackRenderer.h"


namespace tsunami {

ActionTrackLayerMerge::ActionTrackLayerMerge(Track *t) {
	track = t;
}


void ActionTrackLayerMerge::build(Data *d) {
	TrackLayer *lnew = new TrackLayer(track);

	Range r = track->range();
	auto tr = ownify(new TrackRenderer(track, nullptr));
	tr->set_pos(r.start());

	AudioBuffer buf;
	//lnew->getBuffers(buf, r);
	add_sub_action(new ActionTrackCreateBuffers(lnew, r), d);
	auto *a = new ActionTrackEditBuffer(lnew, r);
	lnew->read_buffers(buf, r, true);

	tr->render_audio(buf);

	add_sub_action(a, d);

	add_sub_action(new ActionTrackLayerAdd(track, lnew), d);

	for (int i=track->layers.num-2; i>=0; i--)
		add_sub_action(new ActionTrackLayerDelete(track, i), d);
}

}
