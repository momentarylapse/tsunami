/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Audio/AudioSucker.h"

InputStreamAudio *export_view_input = NULL;
AudioSucker *export_view_sucker = NULL;

ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	input_audio = NULL;
	input_midi = NULL;
	export_view_input = NULL;
	capturing_track = NULL;
	//console_mode = NULL;
}

ViewModeCapture::~ViewModeCapture()
{
	setInputAudio(NULL);
	setInputMidi(NULL);
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
			s.type = Selection::TYPE_TRACK_BUTTON_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_SOLO;
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
	if (input_audio and input_audio->isCapturing()){
		AudioBuffer &buf = export_view_sucker->buf;
		view->update_peaks(buf);
		if (capturing_track)
			view->get_track(capturing_track)->drawBuffer(c, buf, view->cam.pos - view->sel.range.offset, view->colors.capture_marker);
		view->drawTimeLine(c, view->sel.range.start() + buf.length, Selection::TYPE_PLAYBACK, view->colors.capture_marker, true);
	}


	if (input_midi and input_midi->isCapturing()){
		if (capturing_track)
			drawMidi(c, view->get_track(capturing_track), midi_events_to_notes(input_midi->midi), true, view->sel.range.start());
		view->drawTimeLine(c, view->sel.range.start() + input_midi->getSampleCount(), Selection::TYPE_PLAYBACK, view->colors.capture_marker, true);
	}
}

void ViewModeCapture::setInputAudio(InputStreamAudio *_input)
{
	if (input_audio)
		input_audio->unsubscribe(this);

	input_audio = _input;
	export_view_input = input_audio;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	if (input_audio)
		input_audio->subscribe(this, std::bind(&ViewModeCapture::onInputUpdate, this));
}

void ViewModeCapture::setInputMidi(InputStreamMidi *_input)
{
	if (input_midi)
		input_midi->unsubscribe(this);

	input_midi = _input;
	view->notify(view->MESSAGE_INPUT_CHANGE);

	if (input_midi)
		input_midi->subscribe(this, std::bind(&ViewModeCapture::onInputUpdate, this));
}

void ViewModeCapture::onInputUpdate()
{
	if (input_audio){
		AudioBuffer &buf = export_view_sucker->buf;
		if (input_audio->isCapturing())
			view->cam.makeSampleVisible(view->sel.range.start() + buf.length);
		view->forceRedraw();
	}
	if (input_midi){
		if (input_midi->isCapturing())
			view->cam.makeSampleVisible(view->sel.range.start() + input_midi->getSampleCount());
		view->forceRedraw();
	}
}
