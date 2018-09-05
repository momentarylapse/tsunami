/*
 * ActionLayerMerge.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayerMerge.h"
#include "../../../Data/Track.h"
#include "../Buffer/ActionTrackEditBuffer.h"
#include "../Buffer/ActionTrackCreateBuffers.h"
#include "ActionTrackLayerDelete.h"
#include "ActionTrackLayerAdd.h"
#include "ActionTrackFadeDelete.h"

#include "../../../Module/Audio/TrackRenderer.h"


ActionTrackLayerMerge::ActionTrackLayerMerge(Track *t)
{
	track = t;
}


void ActionTrackLayerMerge::build(Data *d)
{
	TrackLayer *lnew = new TrackLayer(track);
	addSubAction(new ActionTrackLayerAdd(track, lnew), d);

	Range r = track->range();
	TrackRenderer *tr = new TrackRenderer(track, nullptr);
	tr->seek(r.start());

	AudioBuffer buf;
	//lnew->getBuffers(buf, r);
	addSubAction(new ActionTrackCreateBuffers(lnew, r), d);
	lnew->readBuffers(buf, r, true);

	tr->render_audio(buf);

	for (int i=track->layers.num-2; i>=0; i--)
		addSubAction(new ActionTrackLayerDelete(track, i), d);

	for (int i=track->fades.num-1; i>=0; i--)
		addSubAction(new ActionTrackFadeDelete(track, i), d);

	delete tr;

}
