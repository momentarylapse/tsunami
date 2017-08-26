/*
 * ViewModeDefault.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeDefault.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../SideBar/SideBar.h"
#include "../../TsunamiWindow.h"
#include "../../Device/OutputStream.h"
#include "../../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "math.h"

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = NULL;
}

ViewModeDefault::~ViewModeDefault()
{
}


void ViewModeDefault::onLeftButtonDown()
{
	selectUnderMouse();

	setBarriers(*hover);

	view->applyBarriers(hover->pos);

	// selection:
	//   start after lb down and moving
	if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TIME) or (hover->type == Selection::TYPE_BACKGROUND) or (hover->type == Selection::TYPE_CLEF_POSITION)){
		setCursorPos(hover->pos);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::TYPE_SELECTION_END){
		hover->range = view->sel.range;
		view->selection_mode = view->SELECTION_MODE_TIME;
	}else if (hover->type == Selection::TYPE_SELECTION_START){
		// swap end / start
		hover->type = Selection::TYPE_SELECTION_END;
		hover->range = view->sel.range;
		hover->range.invert();
		view->selection_mode = view->SELECTION_MODE_TIME;
	}else if (hover->type == Selection::TYPE_MUTE){
		hover->track->setMuted(!hover->track->muted);
	}else if (hover->type == Selection::TYPE_SOLO){
		for (Track *t: song->tracks)
			view->sel.set(t, (t == hover->track));
		if (hover->track->muted)
			hover->track->setMuted(false);
	}else if (hover->type == Selection::TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(view->song, view->sel);
	}
}

void ViewModeDefault::onLeftButtonUp()
{
	if (cur_action)
		song->execute(cur_action);
	cur_action = NULL;

	view->selection_mode = view->SELECTION_MODE_NONE;
	view->msp.stop();
}

void ViewModeDefault::onLeftDoubleClick()
{
	if (view->selection_mode != view->SELECTION_MODE_NONE)
		return;

	selectUnderMouse();

	if (hover->type == Selection::TYPE_SAMPLE){
		win->side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	}else if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HANDLE) or ((hover->track) and ((hover->type == Selection::TYPE_SELECTION_START) or (hover->type == Selection::TYPE_SELECTION_END)))){
		win->side_bar->open(SideBar::TRACK_CONSOLE);
	}else if (!hover->track){
		win->side_bar->open(SideBar::SONG_CONSOLE);
	}
	//hover->type = Selection::TYPE_NONE;
}

void ViewModeDefault::onRightButtonDown()
{
	selectUnderMouse();

	// pop up menu...
	view->updateMenu();

	if (hover->type == Selection::TYPE_SAMPLE)
		view->menu_sample->openPopup(view->win, 0, 0);
	else if (hover->type == Selection::TYPE_MARKER)
		view->menu_marker->openPopup(view->win, 0, 0);
	else if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HANDLE) or (hover->type == Selection::TYPE_SELECTION_START) or (hover->type == Selection::TYPE_SELECTION_END)){
		view->menu_track->enable("track_edit_midi", view->cur_track->type == Track::TYPE_MIDI);
		view->menu_track->openPopup(view->win, 0, 0);
	}else if (!hover->track)
		view->menu_song->openPopup(view->win, 0, 0);
}

void ViewModeDefault::onRightButtonUp()
{
}

void ViewModeDefault::onMouseMove()
{
	bool _force_redraw_ = false;

	auto e = hui::GetEvent();

	if (e->lbut){

		// cheap auto scrolling
		if (hover->allow_auto_scroll()){
			if (view->mx < 50)
				cam->move(-10 / cam->scale);
			if (view->mx > view->area.width() - 50)
				cam->move(10 / cam->scale);
		}

		view->selectionUpdatePos(view->hover);
	}else{
		//Selection hover_old = *hover;
		*hover = getHover();
		/*if (hover_changed(*hover, hover_old))
			view->forceRedraw();
		return;*/
	}


	// drag & drop
	if (view->selection_mode == view->SELECTION_MODE_TIME){

		/*Selection mo = getHover();
		if (mo.track)
			view->sel.add(mo.track);*/

		//_force_redraw_ = true;
		/*_force_redraw_ = false;
		int x, w;
		int r = 4;
		if (e->dx < 0){
			x = view->mx - r;
			w = - e->dx + 2*r;
		}else{
			x = view->mx + r;
			w = - e->dx - 2*r;
		}
		win->redrawRect("area", x, view->area.y1, w, view->area.height());*/
	}else if (hover->type == Selection::TYPE_PLAYBACK){
		view->stream->seek(hover->pos);
		_force_redraw_ = true;
	}else if (hover->type == Selection::TYPE_SAMPLE){
		view->applyBarriers(hover->pos);
		int dpos = (float)hover->pos - hover->sample_offset - hover->sample->pos;
		if (cur_action)
			cur_action->set_param_and_notify(view->song, dpos);
		_force_redraw_ = true;
	}

	if (_force_redraw_)
		view->forceRedraw();
}

void ViewModeDefault::onMouseWheel()
{
	auto e = hui::GetEvent();
	if (fabs(e->scroll_y) > 0.1f)
		cam->zoom(exp(e->scroll_y * view->ZoomSpeed * 0.3f));
	if (fabs(e->scroll_x) > 0.1f)
		cam->move(e->scroll_x / cam->scale * view->ScrollSpeed * 0.03f);
}

void ViewModeDefault::onKeyDown(int k)
{

// view
	// moving
	float dt = 0.05f;
	if (k == hui::KEY_RIGHT)
		cam->move(view->ScrollSpeed * dt / cam->scale);
	if (k == hui::KEY_LEFT)
		cam->move(- view->ScrollSpeed * dt / cam->scale);
	if (k == hui::KEY_NEXT)
		cam->move(view->ScrollSpeedFast * dt / cam->scale);
	if (k == hui::KEY_PRIOR)
		cam->move(- view->ScrollSpeedFast * dt / cam->scale);

	// zoom
	if (k == hui::KEY_ADD)
		cam->zoom(exp(  view->ZoomSpeed));
	if (k == hui::KEY_SUBTRACT)
		cam->zoom(exp(- view->ZoomSpeed));

	if (k == hui::KEY_SPACE){
		if (view->stream->isPlaying()){
			view->stream->pause();
		}else{
			win->onPlay();
		}
	}
	view->updateMenu();
}

void ViewModeDefault::onKeyUp(int k)
{
}

void ViewModeDefault::updateTrackHeights()
{
	int n_ch = 1;
	for (AudioViewTrack *t: view->vtrack){
		t->height_min = view->TIME_SCALE_HEIGHT * 2;
		if (t->track->type == Track::TYPE_AUDIO)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
		else if (t->track->type == Track::TYPE_MIDI)
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
		else
			t->height_wish =view->TIME_SCALE_HEIGHT * 2;
	}
}



void ViewModeDefault::drawMidi(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, int shift)
{
	int mode = which_midi_mode(t->track);
	if (mode == view->MIDI_MODE_LINEAR)
		t->drawMidiLinear(c, midi, as_reference, shift);
	else if (mode == view->MIDI_MODE_TAB)
		t->drawMidiTab(c, midi, as_reference, shift);
	else // if (mode == view->VIEW_MIDI_CLASSICAL)
		t->drawMidiClassical(c, midi, as_reference, shift);
}

void ViewModeDefault::drawTrackBackground(Painter *c, AudioViewTrack *t)
{
	t->drawBlankBackground(c);

	color cc = t->getBackgroundColor();
	view->drawGridTime(c, t->area, cc, false);
	t->drawGridBars(c, cc, (t->track->type == Track::TYPE_TIME), 0);


	if (t->track->type == Track::TYPE_MIDI){
		int mode = which_midi_mode(t->track);
		if (mode == AudioView::MIDI_MODE_CLASSICAL){
			const Clef& clef = t->track->instrument.get_clef();
			t->drawMidiClefClassical(c, clef, view->midi_scale);
		}else if (mode == AudioView::MIDI_MODE_TAB){
			t->drawMidiClefTab(c);
		}
	}
}

void ViewModeDefault::drawTrackData(Painter *c, AudioViewTrack *t)
{
	// midi
	if (t->track->type == Track::TYPE_MIDI)
		drawMidi(c, t, t->track->midi, false, 0);

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	// samples
	for (SampleRef *s: t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker *m, t->track->markers, i)
		t->drawMarker(c, m, i, (hover->type == Selection::TYPE_MARKER) and (hover->track == t->track) and (hover->index == i));
}

void ViewModeDefault::setBarriers(Selection &s)
{
	s.barrier.clear();
	if (s.type == s.TYPE_NONE)
		return;

	int dpos = 0;
	if (s.type == s.TYPE_SAMPLE)
		dpos = s.sample_offset;

	for (Track *t: song->tracks){
		// add subs
		for (SampleRef *sam: t->samples){
			s.barrier.add(sam->pos + dpos);
		}

		// time bar...
		Array<Beat> beats = song->bars.getBeats(cam->range(), true);
		for (Beat &b: beats)
			s.barrier.add(b.range.offset);
	}

	// selection marker
	if (!view->sel.range.empty()){
		s.barrier.add(view->sel.range.start());
		if (view->msp.dist < 0)
			s.barrier.add(view->sel.range.end());
	}
}

Selection ViewModeDefault::getHover()
{
	Selection s;
	int mx = view->mx;
	int my = view->my;
	s.pos = view->cam.screen2sample(mx);
	s.range = Range(s.pos, 0);
	s.y0 = s.y1 = my;
	s.type = s.TYPE_BACKGROUND;

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
	if ((my <= view->TIME_SCALE_HEIGHT) or (view->win->getKey(hui::KEY_SHIFT))){
		if (view->mouse_over_time(view->sel.range.end())){
			s.type = Selection::TYPE_SELECTION_END;
			return s;
		}
		if (view->mouse_over_time(view->sel.range.start())){
			s.type = Selection::TYPE_SELECTION_START;
			return s;
		}
		if (view->stream->isPlaying()){
			if (view->mouse_over_time(view->stream->getPos())){
				s.type = Selection::TYPE_PLAYBACK;
				return s;
			}
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
		for (SampleRef *ss: s.track->samples){
			int offset = view->mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

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

void ViewModeDefault::setCursorPos(int pos)
{
	if (view->stream->isPlaying()){
		if (view->renderer->range().is_inside(pos)){
			view->stream->seek(pos);
			hover->type = Selection::TYPE_PLAYBACK;
			view->forceRedraw();
			return;
		}else{
			view->stream->stop();
			/*view->stream->seek(pos);
			hover->type = Selection::TYPE_PLAYBACK;
			view->forceRedraw();
			return;*/
		}
	}
	//view->msp.start(hover->pos, hover->y0);
	//view->sel.clear();
	view->sel.range = Range(pos, 0);
	view->updateSelection();
}

void ViewModeDefault::selectUnderMouse()
{
	view->sel_temp = view->sel;
	*hover = getHover();
	Track *t = hover->track;
	SampleRef *s = hover->sample;
	bool control = win->getKey(hui::KEY_CONTROL);

	// track
	if (hover->track)
		view->setCurTrack(hover->track);
	if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HANDLE)){
		view->selectTrack(t, control);
		if (!control)
			view->unselectAllSamples();
	}

	// sub
	view->setCurSample(s);
	if (hover->type == Selection::TYPE_SAMPLE){
		view->selectSample(s, control);
	}
}

SongSelection ViewModeDefault::getSelectionForRange(const Range &r)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();
	s.tracks = view->sel.tracks;

	for (Track *t: song->tracks){
		if (!s.has(t))
			continue;

		// subs
		for (SampleRef *sr: t->samples)
			s.set(sr, s.range.overlaps(sr->range()));

		// markers
		for (TrackMarker *m: t->markers)
			s.set(m, s.range.is_inside(m->pos));

		// midi
		for (MidiNote *n: t->midi)
			s.set(n, s.range.is_inside(n->range.center()));
	}
	return s;
}

SongSelection ViewModeDefault::getSelectionForRect(const Range &r, int y0, int y1)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();
	if (y0 > y1){
		int t = y0;
		y0 = y1;
		y1 = t;
	}

	for (auto vt: view->vtrack){
		Track *t = vt->track;
		if ((y1 < vt->area.y1) or (y0 > vt->area.y2))
			continue;
		s.add(t);

		// subs
		for (SampleRef *sr: t->samples)
			s.set(sr, s.range.overlaps(sr->range()));

		// markers
		for (TrackMarker *m: t->markers)
			s.set(m, s.range.is_inside(m->pos));

		// midi
		for (MidiNote *n: t->midi)
			if ((n->y >= y0) and (n->y <= y1))
				//s.set(n, s.range.is_inside(n->range.center()));
				s.set(n, s.range.overlaps(n->range));
	}
	return s;
}

SongSelection ViewModeDefault::getSelectionForTrackRect(const Range &r, int y0, int y1)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();
	if (y0 > y1){
		int t = y0;
		y0 = y1;
		y1 = t;
	}

	for (auto vt: view->vtrack){
		Track *t = vt->track;
		if ((y1 < vt->area.y1) or (y0 > vt->area.y2))
			continue;
		s.add(t);

		// subs
		for (SampleRef *sr: t->samples)
			s.set(sr, s.range.overlaps(sr->range()));

		// markers
		for (TrackMarker *m: t->markers)
			s.set(m, s.range.is_inside(m->pos));

		// midi
		for (MidiNote *n: t->midi)
			//s.set(n, s.range.is_inside(n->range.center()));
			s.set(n, s.range.overlaps(n->range));
	}
	return s;
}

void ViewModeDefault::startSelection()
{
	setBarriers(*hover);
	hover->range.set_start(view->msp.start_pos);
	hover->range.set_end(hover->pos);
	if (hover->type == Selection::TYPE_TIME){
		hover->type = Selection::TYPE_SELECTION_END;
		view->selection_mode = view->SELECTION_MODE_TIME;
	}else{
		hover->y0 = view->msp.start_y;
		hover->y1 = view->my;
		view->selection_mode = view->SELECTION_MODE_TRACK_RECT;
	}
	view->sel = getSelection();
	view->updateSelection();
}

int ViewModeDefault::which_midi_mode(Track *t)
{
	if (view->midi_view_mode == view->MIDI_MODE_LINEAR)
			return view->MIDI_MODE_LINEAR;
	if ((view->midi_view_mode == view->MIDI_MODE_TAB) and (t->instrument.string_pitch.num > 0))
		return view->MIDI_MODE_TAB;
	return view->MIDI_MODE_CLASSICAL;
}

