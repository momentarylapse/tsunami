/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Device/InputStreamAny.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"

InputStreamAny *export_view_input = NULL;

ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	input = NULL;
	export_view_input = NULL;
	capturing_track = NULL;
}

ViewModeCapture::~ViewModeCapture()
{
	setInput(NULL);
}

Selection ViewModeCapture::getHover()
{
	Selection s;
	int mx = view->mx;
	int my = view->my;

	// track?
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (view->mouseOverTrack(t)){
			s.vtrack = t;
			s.index = i;
			s.track = t->track;
			s.type = Selection::TYPE_TRACK;
			if (view->mx < t->area.x1 + view->TRACK_HANDLE_WIDTH)
				s.type = Selection::TYPE_TRACK_HEADER;
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_SOLO;
			return s;
		}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

	return s;
}

void ViewModeCapture::drawPost(Painter *c)
{
	// capturing preview
	if (input and input->isCapturing()){
		int type = input->getType();
		if (type == Track::TYPE_AUDIO)
			((InputStreamAudio*)input)->buffer.update_peaks();
		if (capturing_track){
			if (type == Track::TYPE_AUDIO)
				view->get_track(capturing_track)->drawBuffer(c, dynamic_cast<InputStreamAudio*>(input)->buffer, view->cam.pos - view->sel.range.offset, view->colors.capture_marker);
			if (type == Track::TYPE_MIDI)
				drawMidi(c, view->get_track(capturing_track), midi_events_to_notes(((InputStreamMidi*)input)->midi), true, view->sel.range.start());
		}
	}

	if (input and input->isCapturing())
		view->drawTimeLine(c, view->sel.range.start() + input->getSampleCount(), Selection::TYPE_PLAYBACK, view->colors.capture_marker, true);
}

void ViewModeCapture::setInput(InputStreamAny *_input)
{
	if (input)
		input->unsubscribe(this);

	input = _input;
	export_view_input = input;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	if (input)
		input->subscribe(this, std::bind(&ViewModeCapture::onInputUpdate, this));
}

void ViewModeCapture::onInputUpdate()
{
	if (input){
		if (input->isCapturing())
			view->cam.makeSampleVisible(view->sel.range.start() + input->getSampleCount());
		view->forceRedraw();
	}
}
