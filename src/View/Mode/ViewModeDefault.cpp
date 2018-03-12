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
#include "math.h"
#include "../../Audio/Source/SongRenderer.h"
#include "../../Rhythm/Beat.h"
#include "../../Rhythm/Bar.h"

ViewModeDefault::ViewModeDefault(AudioView *view) :
	ViewMode(view)
{
	cur_action = NULL;
	moving_track = NULL;
}

ViewModeDefault::~ViewModeDefault()
{
}


void ViewModeDefault::onLeftButtonDown()
{
	bool track_hover_sel = view->sel.has(hover->track);
	selectUnderMouse();

	setBarriers(*hover);

	view->applyBarriers(hover->pos);

	// selection:
	//   start after lb down and moving
	if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_CLEF_POSITION)){
		setCursorPos(hover->pos, track_hover_sel);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::TYPE_TIME){
		setCursorPos(hover->pos, true);
		view->msp.start(hover->pos, hover->y0);
	}else if (hover->type == Selection::TYPE_BACKGROUND){
		setCursorPos(hover->pos, false);
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
	}else if (hover->type == Selection::TYPE_TRACK_BUTTON_MUTE){
		hover->track->setMuted(!hover->track->muted);
	}else if (hover->type == Selection::TYPE_TRACK_BUTTON_SOLO){
		auto vt = view->get_track(hover->track);
		vt->setSolo(!vt->solo);
	}else if (hover->type == Selection::TYPE_TRACK_BUTTON_EDIT){
		view->win->side_bar->open(SideBar::TRACK_CONSOLE);
	}else if (hover->type == Selection::TYPE_TRACK_BUTTON_FX){
		view->win->side_bar->open(SideBar::FX_CONSOLE);
	}else if (hover->type == Selection::TYPE_TRACK_BUTTON_CURVE){
		view->win->side_bar->open(SideBar::CURVE_CONSOLE);
	}else if (hover->type == Selection::TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(view->song, view->sel);
	}else if (hover->type == Selection::TYPE_TRACK_HEADER){
		view->msp.start(hover->pos, hover->y0);
	}
}

void ViewModeDefault::onLeftButtonUp()
{
	if (cur_action){
		song->execute(cur_action);
		cur_action = NULL;
	}

	if (moving_track){
		int target = getTrackMoveTarget(false);
		//int orig = get_track_index(moving_track);
		moving_track->move(target);
		moving_track = NULL;
	}

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
	}else if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HEADER) or ((hover->track) and ((hover->type == Selection::TYPE_SELECTION_START) or (hover->type == Selection::TYPE_SELECTION_END)))){
		win->side_bar->open(SideBar::TRACK_CONSOLE);
	}else if (!hover->track){
		win->side_bar->open(SideBar::SONG_CONSOLE);
	}
	//hover->type = Selection::TYPE_NONE;
}

void ViewModeDefault::onRightButtonDown()
{
	bool track_hover_sel = view->sel.has(hover->track);

	selectUnderMouse();

	// click outside sel.range -> select new position
	if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TIME) or (hover->type == Selection::TYPE_BACKGROUND)){
		if (!view->sel.range.is_inside(hover->pos)){
			setBarriers(*hover);
			view->applyBarriers(hover->pos);
			setCursorPos(hover->pos, track_hover_sel);
		}
	}

	// pop up menu...
	view->updateMenu();

	if (hover->type == Selection::TYPE_SAMPLE)
		view->menu_sample->openPopup(view->win, 0, 0);
	else if (hover->type == Selection::TYPE_MARKER)
		view->menu_marker->openPopup(view->win, 0, 0);
	else if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HEADER) or (hover->type == Selection::TYPE_SELECTION_START) or (hover->type == Selection::TYPE_SELECTION_END)){
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
		view->renderer->seek(hover->pos);
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
		if (view->isPlaying()){
			view->pause(!view->isPaused());
		}else{
			win->onPlay();
			//view->play(); unsafe for recording...
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



void ViewModeDefault::drawMidi(Painter *c, AudioViewTrack *t, const MidiNoteBuffer &midi, bool as_reference, int shift)
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
	if (song->bars.num > 0)
		t->drawGridBars(c, cc, (t->track->type == Track::TYPE_TIME), 0);
	else
		view->drawGridTime(c, t->area, cc, false);


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

void draw_bar_selection(Painter *c, AudioViewTrack *t, AudioView *view)
{
	float y1 = t->area.y1;
	float y2 = t->area.y2;

	float lw = 3;

	c->setLineWidth(lw*2);
	c->setColor(view->colors.selection_internal);
	c->setFill(false);

	auto bars = view->song->bars.getBars(Range::ALL);
	for (auto &b: bars){
		if (view->sel.bars.is_inside(b.index)){
			float x1 = view->cam.sample2screen(b.range.offset);
			float x2 = view->cam.sample2screen(b.range.end());
			c->drawRect(x1 + lw, y1 + lw, x2-x1 - 2*lw, y2-y1 - 2*lw);
		}
	}
	c->setFill(true);
	c->setLineWidth(1);

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

	if (t->track->type == Track::TYPE_TIME)
		draw_bar_selection(c, t, view);
}

int ViewModeDefault::getTrackMoveTarget(bool visual)
{
	int orig = get_track_index(moving_track);
	foreachi(auto vt, view->vtrack, i){
		int y = (vt->area.y1 + vt->area.y2) / 2;
		if (y > view->my){
			if (visual or (i <= orig))
				return i;
			else
				return i - 1;
		}
	}
	return visual ? song->tracks.num : (song->tracks.num-1);
}

void drawCursorHover(ViewMode *m, Painter *c, const string &msg)
{
	float x = max(m->view->mx - 20.0f, 2.0f);
	float y = m->view->my + 20;
	c->setFont("", -1, true, false);
	m->view->drawBoxedStr(c, x, y, msg, m->view->colors.background, m->view->colors.text_soft1);
	c->setFont("", -1, false, false);
}

void ViewModeDefault::drawPost(Painter *c)
{
	if (moving_track){
		//int orig = get_track_index(moving_track);
		int t = getTrackMoveTarget(true);
		int y = view->vtrack.back()->area.y2;
		if (t < view->vtrack.num)
			y = view->vtrack[t]->area.y1;

		c->setColor(view->colors.selection_boundary);
		c->setLineWidth(2.0f);
		c->drawLine(view->area.x1,  y,  view->area.x2,  y);
		c->setLineWidth(1.0f);

		/*c->setColor(view->colors.selection_internal);
		rect r = view->vtrack[orig]->area;
		r.x2 = view->TRACK_HANDLE_WIDTH;
		c->drawRect(r);*/

		drawCursorHover(this, c, moving_track->getNiceName());
	}

	if (hover->type == Selection::TYPE_SAMPLE)
		drawCursorHover(this, c, _("sample ") + hover->sample->origin->name);
	if (hover->type == Selection::TYPE_TRACK_BUTTON_EDIT)
		drawCursorHover(this, c, _("edit track properties"));
	if (hover->type == Selection::TYPE_TRACK_BUTTON_MUTE)
		drawCursorHover(this, c, _("toggle mute"));
	if (hover->type == Selection::TYPE_TRACK_BUTTON_SOLO)
		drawCursorHover(this, c, _("toggle solo"));
	if (hover->type == Selection::TYPE_TRACK_BUTTON_CURVE)
		drawCursorHover(this, c, _("edit curves"));
	if (hover->type == Selection::TYPE_TRACK_BUTTON_FX)
		drawCursorHover(this, c, _("edit effects"));
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

Selection ViewModeDefault::getHoverBasic()
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
			if ((view->mx < t->area.x1 + view->TRACK_HANDLE_WIDTH) and (view->my < t->area.y1 + view->TRACK_HANDLE_HEIGHT))
				s.type = Selection::TYPE_TRACK_HEADER;
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
		if (view->isPlaying()){
			if (view->mouse_over_time(view->stream->getPos(view->stream->getPos(view->renderer->getPos())))){
				s.type = Selection::TYPE_PLAYBACK;
				return s;
			}
		}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

	// track header buttons?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		int x = 5;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_MUTE;
			return s;
		}
		x += 17;
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_SOLO;
			return s;
		}
		x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_EDIT;
			return s;
		}
		x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_FX;
			return s;
		}
		/*x += 17;
		if ((mx >= t->area.x1 + x) and (mx < t->area.x1 + x+12) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_TRACK_BUTTON_CURVE;
			return s;
		}*/
	}

	return s;
}

Selection ViewModeDefault::getHover()
{
	Selection s = getHoverBasic();
	int mx = view->mx;
	int my = view->my;

	if (s.track){

		// markers
		for (int i=0; i<min(s.track->markers.num, view->vtrack[s.index]->marker_areas.num); i++){
			if (view->vtrack[s.index]->marker_areas[i].inside(mx, my)){
				s.type = Selection::TYPE_MARKER;
				s.index = i;
				return s;
			}
		}

		// TODO: prefer selected samples
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

	return s;
}

void ViewModeDefault::setCursorPos(int pos, bool keep_track_selection)
{
	if (view->isPlaying()){
		if (view->renderer->range().is_inside(pos)){
			view->renderer->seek(pos);
			hover->type = Selection::TYPE_PLAYBACK;
			view->forceRedraw();
			return;
		}else{
			view->stop();
		}
	}
	//view->msp.start(hover->pos, hover->y0);
	Set<const Track*> tracks = view->sel.tracks;
	view->sel.clear();
	if (keep_track_selection)
		view->sel.tracks = tracks;
	else
		view->sel.tracks = view->cur_track;
		//view->sel.all_tracks(view->song);
	view->setSelection(getSelectionForRange(Range(pos, 0)));
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
	if ((hover->type == Selection::TYPE_TRACK) or (hover->type == Selection::TYPE_TRACK_HEADER)){
		view->selectTrack(t, control);
		/*if (!control)
			view->unselectAllSamples();*/
	}
	if (hover->type == Selection::TYPE_MARKER){
		auto m = t->markers[hover->index];
		if (control){
			view->sel.set(m, !view->sel.has(m));
		}else{
			if (view->sel.has(m)){
			}else{
				view->selectTrack(t, false);
				view->sel.clear_data();
				view->sel.add(m);
			}
		}
	}

	// sample
	view->setCurSample(s);
	if (hover->type == Selection::TYPE_SAMPLE){
		if (control){
			view->sel.set(s, !view->sel.has(s));
		}else{
			if (view->sel.has(s)){
			}else{
				view->selectTrack(t, false);
				view->sel.clear_data();
				view->sel.add(s);
			}
		}
	}
}

SongSelection ViewModeDefault::getSelectionForRange(const Range &r)
{
	SongSelection s;
	s.range = r;
	if (s.range.length < 0)
		s.range.invert();
	s.from_range(song, r, view->sel.tracks);
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
			s.set(m, s.range.overlaps(m->range));

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
			s.set(m, s.range.overlaps(m->range));

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
	if (hover->type == Selection::TYPE_TRACK_HEADER){
		moving_track = hover->track;
	}else if (hover->type == Selection::TYPE_TIME){
		hover->type = Selection::TYPE_SELECTION_END;
		view->selection_mode = view->SELECTION_MODE_TIME;
	}else{
		hover->y0 = view->msp.start_y;
		hover->y1 = view->my;
		view->selection_mode = view->SELECTION_MODE_TRACK_RECT;
	}
	view->setSelection(getSelection(hover->range));
}

int ViewModeDefault::which_midi_mode(Track *t)
{
	if (view->midi_view_mode == view->MIDI_MODE_LINEAR)
			return view->MIDI_MODE_LINEAR;
	if ((view->midi_view_mode == view->MIDI_MODE_TAB) and (t->instrument.string_pitch.num > 0))
		return view->MIDI_MODE_TAB;
	return view->MIDI_MODE_CLASSICAL;
}

