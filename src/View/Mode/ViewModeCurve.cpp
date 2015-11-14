/*
 * ViewModeCurve.cpp
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#include "ViewModeCurve.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Audio/AudioStream.h"
#include "../../TsunamiWindow.h"

ViewModeCurve::ViewModeCurve(AudioView* view) :
	ViewModeDefault(view)
{
	curve = NULL;
}

ViewModeCurve::~ViewModeCurve()
{
}

void ViewModeCurve::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();
}

void ViewModeCurve::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();
}

void ViewModeCurve::onMouseMove()
{
	ViewModeDefault::onMouseMove();
}

void ViewModeCurve::onKeyDown(int k)
{
	ViewModeDefault::onKeyDown(k);
}

void ViewModeCurve::drawTrackData(HuiPainter* c, AudioViewTrack* t)
{
	ViewModeDefault::drawTrackData(c, t);

	c->setColor(Black);
	c->drawStr(100, 100, "test");
}

Selection ViewModeCurve::getHover()
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
				s.show_track_controls = t->track;
		}
	}

	// selection boundaries?
	view->selectionUpdatePos(s);
	if (view->mouse_over_time(view->sel_raw.end())){
		s.type = Selection::TYPE_SELECTION_END;
		return s;
	}
	if (view->mouse_over_time(view->sel_raw.start())){
		s.type = Selection::TYPE_SELECTION_START;
		return s;
	}
	if (view->stream->isPlaying()){
		if (view->mouse_over_time(view->stream->getPos())){
			s.type = Selection::TYPE_PLAYBACK;
			return s;
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){

		// markers
		for (int i=0; i<min(s.track->markers.num, view->vtrack[s.index]->marker_areas.num); i++){
			if (view->vtrack[s.index]->marker_areas[i].inside(mx, my)){
				s.type = Selection::TYPE_MARKER;
				s.index = i;
				return s;
			}
		}

		// TODO: prefer selected subs
		foreach(SampleRef *ss, s.track->samples){
			int offset = view->mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

	// midi
	/*if ((s.track) and (s.track->type == Track::TYPE_MIDI) and (s.track == view->cur_track)){
		if (scroll_bar.inside(view->mx, view->my)){
			s.type = Selection::TYPE_SCROLL;
			return s;
		}
		if (midi_mode != MIDI_MODE_SELECT){
			s.pitch = y2pitch(my);
			s.type = Selection::TYPE_MIDI_PITCH;
			Array<MidiNote> notes = s.track->midi.getNotes(cam->range());
			foreachi(MidiNote &n, notes, i)
				if ((n.pitch == s.pitch) and (n.range.is_inside(s.pos))){
					s.index = i;
					s.type = Selection::TYPE_MIDI_NOTE;
					return s;
				}
		}
	}*/

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

	// track handle
	if ((s.track) and (mx < view->area.x1 + view->TRACK_HANDLE_WIDTH)){
		s.type = Selection::TYPE_TRACK_HANDLE;
		return s;
	}

	return s;
}

void ViewModeCurve::setCurve(Curve* c)
{
	curve = c;
	view->forceRedraw();
}
