/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "AudioViewTrack.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "SideBar/SideBar.h"
#include "BottomBar/BottomBar.h"
#include "../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../Audio/AudioInput.h"
#include "../Audio/AudioStream.h"
#include "../Audio/AudioRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Synth/SynthesizerRenderer.h"
#include "../Stuff/Log.h"
#include "../lib/math/math.h"

const int AudioView::FONT_SIZE = 10;
const int AudioView::MAX_TRACK_CHANNEL_HEIGHT = 125;
const float AudioView::LINE_WIDTH = 1.0f;
const int AudioView::SUB_FRAME_HEIGHT = 20;
const int AudioView::TIME_SCALE_HEIGHT = 20;
const int AudioView::TRACK_HANDLE_WIDTH = 60;

const float BORDER_FACTOR = 1.0f / 15.0f;

int get_track_index_save(Track *t)
{
	if (t){
		foreachi(Track *tt, tsunami->audio->tracks, i)
			if (t == tt)
				return i;
	}
	return -1;
}


bool AudioView::is_sharp(int pitch)
{
	int r = (pitch - midi_scale + 12) % 12;
	// 69 = 9 = a
	return ((r == 10) or (r == 1) or (r == 3) or (r == 6) or (r == 8));
}

const string AudioView::MESSAGE_CUR_TRACK_CHANGE = "CurTrackChange";
const string AudioView::MESSAGE_CUR_SAMPLE_CHANGE = "CurSampleChange";
const string AudioView::MESSAGE_CUR_LEVEL_CHANGE = "CurLevelChange";
const string AudioView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string AudioView::MESSAGE_VIEW_CHANGE = "ViewChange";

AudioView::SelectionType::SelectionType()
{
	type = SEL_TYPE_NONE;
	track = NULL;
	vtrack = NULL;
	sample = NULL;
	pos = 0;
	sample_offset = 0;
	show_track_controls = NULL;
	pitch = -1;
	note_start = -1;
}

void AudioView::ColorScheme::create(ColorSchemeBasic &basic)
{
	name = basic.name;
	background = basic.background;
	background_track_selected = ColorInterpolate(basic.background, basic.selection*1.5f, 0.17f);
	background_track = ColorInterpolate(background, background_track_selected, 0.5f);
	selection = basic.selection;
	hover = basic.hover;
	selection_internal = basic.selection;
	selection_internal.a = 0.2f;
	selection_boundary = basic.selection;
	selection_boundary_hover = ColorInterpolate(basic.selection, basic.hover, 0.6f);
	preview_marker = color(1, 0, 0.7f, 0);
	capture_marker = color(1, 0.7f, 0, 0);
	text = basic.text;
	text_soft1 = ColorInterpolate(basic.background, basic.text, pow(0.72f, basic.gamma));
	text_soft3 = ColorInterpolate(basic.background, ColorInterpolate(basic.text, basic.selection, 0.7f), pow(0.3f, basic.gamma));
	text_soft2 = ColorInterpolate(text_soft3, text_soft1, 0.4f);
	grid = text_soft3;
	sample = color(1, 0.6f, 0.6f, 0);
	sample_hover = color(1, 0.6f, 0, 0);
	sample_selected = color(1, 0.4f, 0.4f, 0.4f);
}

AudioView::AudioView(TsunamiWindow *parent, AudioFile *_audio, AudioOutput *_output, AudioInput *_input) :
	Observer("AudioView"),
	Observable("AudioView"),
	BarrierDist(5)
{
	win = parent;
	thm.dirty = true;
	thm.t = 0;
	thm.render_area = rect(0, 0, 0, 0);
	thm.animating = false;
	thm.midi_track = NULL;

	ColorSchemeBasic bright;
	bright.background = White;
	bright.text = color(1, 0.3f, 0.3f, 0.3f);
	bright.selection = color(1, 0.2f, 0.2f, 0.7f);
	bright.hover = White;
	bright.gamma = 1.0f;
	bright.name = "bright";
	basic_schemes.add(bright);

	ColorSchemeBasic dark;
	dark.background = color(1, 0.15f, 0.15f, 0.15f);
	dark.text = color(1, 0.95f, 0.95f, 0.95f);
	dark.selection = color(1, 0.3f, 0.3f, 0.8f);
	dark.hover = White;
	dark.gamma = 0.3f;
	dark.name = "dark";
	basic_schemes.add(dark);

	setColorScheme(HuiConfig.getStr("View.ColorScheme", "bright"));

	drawing_rect = rect(0, 1024, 0, 768);
	enabled = true;

	show_mono = HuiConfig.getBool("View.Mono", true);
	detail_steps = HuiConfig.getInt("View.DetailSteps", 1);
	mouse_min_move_to_select = HuiConfig.getInt("View.MouseMinMoveToSelect", 5);
	preview_sleep_time = HuiConfig.getInt("PreviewSleepTime", 10);
	ScrollSpeed = HuiConfig.getInt("View.ScrollSpeed", 300);
	ScrollSpeedFast = HuiConfig.getInt("View.ScrollSpeedFast", 3000);
	ZoomSpeed = HuiConfig.getFloat("View.ZoomSpeed", 0.1f);
	peak_mode = HuiConfig.getInt("View.PeakMode", BufferBox::PEAK_MODE_SQUAREMEAN);
	antialiasing = HuiConfig.getBool("View.Antialiasing", false);

	image_unmuted.load(HuiAppDirectoryStatic + "Data/volume.tga");
	image_muted.load(HuiAppDirectoryStatic + "Data/mute.tga");
	image_solo.load(HuiAppDirectoryStatic + "Data/solo.tga");
	image_track_audio.load(HuiAppDirectoryStatic + "Data/track-audio.tga");
	image_track_time.load(HuiAppDirectoryStatic + "Data/track-time.tga");
	image_track_midi.load(HuiAppDirectoryStatic + "Data/track-midi.tga");

	view_zoom = 0.001;
	view_pos = 0;

	mouse_possibly_selecting = -1;
	cur_action = NULL;

	cur_track = NULL;
	cur_sample = NULL;
	cur_level = 0;

	audio = _audio;
	input = _input;

	pitch_min = 60;
	pitch_max = 90;
	beat_partition = 4;
	parent->setInt("beat_partition", beat_partition);
	midi_scale = 0;
	midi_mode = MIDI_MODE_NOTE;
	chord_type = 0;
	chord_inversion = 0;

	renderer = new AudioRenderer;
	stream = new AudioStream(renderer);

	midi_preview_renderer = new SynthesizerRenderer(NULL);
	midi_preview_renderer->setAutoStop(true);
	midi_preview_stream = new AudioStream(midi_preview_renderer);
	midi_preview_stream->setBufferSize(2048);

	area = rect(0, 0, 0, 0);
	mx = my = 0;
	subscribe(audio);
	subscribe(stream);
	subscribe(input);

	// events
	parent->eventX("area", "hui:draw", this, &AudioView::onDraw);
	parent->eventX("area", "hui:mouse-move", this, &AudioView::onMouseMove);
	parent->eventX("area", "hui:left-button-down", this, &AudioView::onLeftButtonDown);
	parent->eventX("area", "hui:left-double-click", this, &AudioView::onLeftDoubleClick);
	parent->eventX("area", "hui:left-button-up", this, &AudioView::onLeftButtonUp);
	parent->eventX("area", "hui:middle-button-down", this, &AudioView::onMiddleButtonDown);
	parent->eventX("area", "hui:middle-button-up", this, &AudioView::onMiddleButtonUp);
	parent->eventX("area", "hui:right-button-down", this, &AudioView::onRightButtonDown);
	parent->eventX("area", "hui:right-button-up", this, &AudioView::onRightButtonUp);
	//parent->eventX("area", "hui:key-down", this, &AudioView::OnKeyDown);
	parent->eventX("area", "hui:key-down", this, &AudioView::onKeyDown);
	parent->eventX("area", "hui:key-up", this, &AudioView::onKeyUp);
	parent->eventX("area", "hui:mouse-wheel", this, &AudioView::onMouseWheel);

	parent->activate("area");


	menu_audio = HuiCreateResourceMenu("popup_audio_file_menu");
	menu_track = HuiCreateResourceMenu("popup_track_menu");
	menu_sub = HuiCreateResourceMenu("popup_sample_menu");

	//ForceRedraw();
	updateMenu();
}

AudioView::~AudioView()
{
	unsubscribe(audio);
	unsubscribe(stream);
	unsubscribe(input);

	delete(stream);
	delete(renderer);
	delete(midi_preview_stream);
	delete(midi_preview_renderer);

	HuiConfig.setBool("View.Mono", show_mono);
	HuiConfig.setInt("View.DetailSteps", detail_steps);
	HuiConfig.setInt("View.MouseMinMoveToSelect", mouse_min_move_to_select);
	HuiConfig.setInt("View.ScrollSpeed", ScrollSpeed);
	HuiConfig.setInt("View.ScrollSpeedFast", ScrollSpeedFast);
	HuiConfig.setFloat("View.ZoomSpeed", ZoomSpeed);
	HuiConfig.setBool("View.Antialiasing", antialiasing);
}

void AudioView::setColorScheme(const string &name)
{
	HuiConfig.setStr("View.ColorScheme", name);
	colors.create(basic_schemes[0]);
	foreach(ColorSchemeBasic &b, basic_schemes)
		if (b.name == name)
			colors.create(b);
	forceRedraw();
}

void AudioView::setMouse()
{
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my;
}

bool AudioView::mouseOverTrack(AudioViewTrack *t)
{
	return t->area.inside(mx, my);
}

int AudioView::mouseOverSample(SampleRef *s)
{
	if ((mx >= s->area.x1) and (mx < s->area.x2)){
		int offset = screen2sample(mx) - s->pos;
		if ((my >= s->area.y1) and (my < s->area.y1 + SUB_FRAME_HEIGHT))
			return offset;
		if ((my >= s->area.y2 - SUB_FRAME_HEIGHT) and (my < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::selectionUpdatePos(SelectionType &s)
{
	s.pos = screen2sample(mx);
}

void AudioView::updateSelection()
{
	msg_db_f("UpdateSelection", 1);
	sel_range = sel_raw;
	if (sel_range.num < 0)
		sel_range.invert();


	renderer->setRange(getPlaybackSelection());
	if (stream->isPlaying()){
		if (renderer->range().is_inside(stream->getPos()))
			renderer->setRange(getPlaybackSelection());
		else
			stream->stop();
	}

	audio->updateSelection(sel_range);
	notify(MESSAGE_SELECTION_CHANGE);
}

bool mouse_over_time(AudioView *v, int pos)
{
	int ssx = v->sample2screen(pos);
	return ((v->mx >= ssx - 5) and (v->mx <= ssx + 5));
}

AudioView::SelectionType AudioView::getMouseOver()
{
	SelectionType s;

	// track?
	foreachi(AudioViewTrack *t, vtrack, i){
		if (mouseOverTrack(t)){
			s.vtrack = t;
			s.track = t->track;
			s.type = SEL_TYPE_TRACK;
			if (mx < t->area.x1 + TRACK_HANDLE_WIDTH)
				s.show_track_controls = t->track;
		}
	}

	// selection boundaries?
	selectionUpdatePos(s);
	if (mouse_over_time(this, sel_raw.end())){
		s.type = SEL_TYPE_SELECTION_END;
		return s;
	}
	if (mouse_over_time(this, sel_raw.start())){
		s.type = SEL_TYPE_SELECTION_START;
		return s;
	}
	if (stream->isPlaying()){
		if (mouse_over_time(this, stream->getPos())){
			s.type = SEL_TYPE_PLAYBACK;
			return s;
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = SEL_TYPE_MUTE;
			return s;
		}
		if ((audio->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = SEL_TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){
		// TODO: prefer selected subs
		foreach(SampleRef *ss, s.track->samples){
			int offset = mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = SEL_TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

	// midi
	if ((s.track) and (s.track == cur_track) and (editingMidi()) and (midi_mode != MIDI_MODE_SELECT)){
		s.pitch = y2pitch(my);
		s.type = SEL_TYPE_MIDI_PITCH;
		Array<MidiNote> notes = s.track->midi.getNotes(viewRange());
		foreach(MidiNote &n, notes)
			if ((n.pitch == s.pitch) and (n.range.is_inside(s.pos))){
				s.note_start = n.range.offset;
				s.type = SEL_TYPE_MIDI_NOTE;
				return s;
			}
	}

	// time scale
	if (my < TIME_SCALE_HEIGHT){
		s.type = SEL_TYPE_TIME;
		return s;
	}

	// track handle
	if ((s.track) and (mx < area.x1 + TRACK_HANDLE_WIDTH)){
		s.type = SEL_TYPE_TRACK_HANDLE;
		return s;
	}

	return s;
}

Range AudioView::viewRange()
{
	return Range(view_pos, area.width() / view_zoom);
}


Range AudioView::getPlaybackSelection()
{
	if (sel_range.empty()){
		int num = audio->getRange().end() - sel_range.start();
		if (num <= 0)
			num = audio->sample_rate; // 1 second
		return Range(sel_range.start(), num);
	}
	return sel_range;
}

void AudioView::selectUnderMouse()
{
	msg_db_f("SelectUnderMouse", 2);
	hover = getMouseOver();
	selection = hover;
	Track *t = selection.track;
	SampleRef *s = selection.sample;
	bool control = win->getKey(KEY_CONTROL);

	// track
	if (selection.track)
		setCurTrack(selection.track);
	if ((selection.type == SEL_TYPE_TRACK) or (selection.type == SEL_TYPE_TRACK_HANDLE)){
		selectTrack(t, control);
		if (!control)
			audio->unselectAllSamples();
	}

	// sub
	setCurSample(s);
	if (selection.type == SEL_TYPE_SAMPLE){
		selectSample(s, control);
	}
}

void AudioView::setBarriers(SelectionType *s)
{
	msg_db_f("SetBarriers", 2);
	s->barrier.clear();
	if (s->type == SEL_TYPE_NONE)
		return;

	int dpos = 0;
	if (s->type == SEL_TYPE_SAMPLE)
		dpos = s->sample_offset;

	foreach(Track *t, audio->tracks){
		// add subs
		foreach(SampleRef *sam, t->samples){
			s->barrier.add(sam->pos + dpos);
		}

		// time bar...
		int x0 = 0;
		foreach(BarPattern &b, t->bars){
			// FIXME
			for (int i=0;i<b.num_beats;i++)
				s->barrier.add(x0 + (int)((float)b.length * i / (float)b.num_beats) + dpos);
			x0 += b.length;
		}
	}

	// selection marker
	if (!sel_range.empty()){
		s->barrier.add(sel_raw.start());
		if (mouse_possibly_selecting < 0)
			s->barrier.add(sel_raw.end());
	}
}

void AudioView::applyBarriers(int &pos)
{
	msg_db_f("ApplyBarriers", 2);
	foreach(int b, selection.barrier){
		int dpos = sample2screen(b) - sample2screen(pos);
		if (abs(dpos) <= BarrierDist){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			pos = b;
		}
	}
}

bool hover_changed(AudioView::SelectionType &hover, AudioView::SelectionType &hover_old)
{
	return (hover.type != hover_old.type)
			or (hover.sample != hover_old.sample)
			or (hover.show_track_controls != hover_old.show_track_controls)
			or (hover.note_start != hover_old.note_start)
			or (hover.pitch != hover_old.pitch);
}

void AudioView::onMouseMove()
{
	msg_db_f("OnMouseMove", 2);
	setMouse();
	bool _force_redraw_ = false;

	if (HuiGetEvent()->lbut){
		selectionUpdatePos(selection);
	}else{
		SelectionType hover_old = hover;
		hover = getMouseOver();
		_force_redraw_ |= hover_changed(hover, hover_old);
	}


	// drag & drop
	if (selection.type == SEL_TYPE_SELECTION_END){
		SelectionType mo = getMouseOver();
		if (mo.track)
			mo.track->is_selected = true;

		applyBarriers(selection.pos);
		sel_raw.set_end(selection.pos);
		updateSelection();
		//_force_redraw_ = true;
		_force_redraw_ = false;
		int x, w;
		int r = 4;
		if (HuiGetEvent()->dx < 0){
			x = mx - r;
			w = - HuiGetEvent()->dx + 2*r;
		}else{
			x = mx + r;
			w = - HuiGetEvent()->dx - 2*r;
		}
		win->redrawRect("area", x, area.y1, w, area.height());
	}else if (selection.type == SEL_TYPE_PLAYBACK){
		renderer->seek(selection.pos);
		stream->play();
		_force_redraw_ = true;
	}else if (selection.type == SEL_TYPE_SAMPLE){
		applyBarriers(selection.pos);
		int dpos = (float)selection.pos - selection.sample_offset - selection.sample->pos;
		if (cur_action)
			cur_action->set_param_and_notify(audio, dpos);
		_force_redraw_ = true;
	}

	// selection:
	if (!HuiGetEvent()->lbut){
		mouse_possibly_selecting = -1;
	}
	if (mouse_possibly_selecting >= 0)
		mouse_possibly_selecting += abs(HuiGetEvent()->dx);
	if (mouse_possibly_selecting > mouse_min_move_to_select){
		sel_raw.offset = mouse_possibly_selecting_start;
		sel_raw.num = selection.pos - mouse_possibly_selecting_start;
		setBarriers(&selection);
		updateSelection();
		selection.type = SEL_TYPE_SELECTION_END;
		hover.type = SEL_TYPE_SELECTION_END;
		_force_redraw_ = true;
		mouse_possibly_selecting = -1;
	}

	if ((HuiGetEvent()->lbut) and (selection.type == SEL_TYPE_MIDI_PITCH))
		_force_redraw_ = true;

	if (_force_redraw_)
		forceRedraw();
}


void AudioView::setCursorPos(int pos)
{
	if (stream->isPlaying()){
		if (renderer->range().is_inside(pos)){
			renderer->seek(pos);
			stream->play();
			selection.type = SEL_TYPE_PLAYBACK;
			forceRedraw();
			return;
		}else
			stream->stop();
	}
	mouse_possibly_selecting = 0;
	sel_raw = Range(pos, 0);
	updateSelection();
}

void deleteMidiNote(Track *t, int pitch, int start)
{
	Array<int> events;
	foreachi(MidiEvent &e, t->midi, i)
		if (e.pitch == pitch){
			if (e.pos >= start){
				events.add(i);
				if (e.volume <= 0)
					break;
			}
		}

	t->root->action_manager->beginActionGroup();
	foreachb(int i, events)
		t->deleteMidiEvent(i);
	t->root->action_manager->endActionGroup();
}

void AudioView::onLeftButtonDown()
{
	msg_db_f("OnLBD", 2);
	setMouse();
	selectUnderMouse();
	updateMenu();

	mouse_possibly_selecting_start = selection.pos;

	// selection:
	//   start after lb down and moving
	if ((selection.type == SEL_TYPE_TRACK) or (selection.type == SEL_TYPE_TIME)){
		setCursorPos(selection.pos);
	}else if (selection.type == SEL_TYPE_SELECTION_START){
		// swap end / start
		selection.type = SEL_TYPE_SELECTION_END;
		hover.type = SEL_TYPE_SELECTION_END;
		sel_raw.invert();
	}else if (selection.type == SEL_TYPE_MUTE){
		selection.track->setMuted(!selection.track->muted);
	}else if (selection.type == SEL_TYPE_SOLO){
		foreach(Track *t, audio->tracks)
			t->is_selected = (t == selection.track);
		if (selection.track->muted)
			selection.track->setMuted(false);
	}else if (selection.type == SEL_TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(audio);
	}else if (selection.type == SEL_TYPE_MIDI_NOTE){
		deleteMidiNote(cur_track, selection.pitch, selection.note_start);
	}else if (selection.type == SEL_TYPE_MIDI_PITCH){
		midi_preview_renderer->resetMidiData();
		midi_preview_renderer->setSynthesizer(cur_track->synth);

		Array<int> pitch = GetChordNotes((midi_mode == MIDI_MODE_CHORD) ? chord_type : -1, chord_inversion, selection.pitch);
		foreach(int p, pitch)
			midi_preview_renderer->add(MidiEvent(0, p, 1));
		midi_preview_stream->play();
	}

	setBarriers(&selection);

	forceRedraw();
	updateMenu();
}


Array<MidiNote> AudioView::getCreationNotes()
{
	int start = min(mouse_possibly_selecting_start, selection.pos);
	int end = max(mouse_possibly_selecting_start, selection.pos);
	Track *t = audio->getTimeTrack();
	if (t){
		Array<Beat> beats = t->bars.getBeats(Range(-0x4000000, 0x8000000));//audio->getRange());
		foreach(Beat &b, beats){
			if (b.range.is_inside(start)){
				int dl = b.range.num / beat_partition;
				start = b.range.offset + dl * ((start - b.range.offset) / dl);
			}
			if (b.range.is_inside(end)){
				int dl = b.range.num / beat_partition;
				end = b.range.offset + dl * ((end - b.range.offset) / dl + 1);
				break;
			}
		}
	}
	Range r = Range(start, end - start);
	Array<MidiNote> notes;
	Array<int> pitch = GetChordNotes((midi_mode == MIDI_MODE_CHORD) ? chord_type : -1, chord_inversion, selection.pitch);
	foreach(int p, pitch)
		notes.add(MidiNote(r, p, 1));
	return notes;
}


void AudioView::onLeftButtonUp()
{
	msg_db_f("OnLBU", 2);
	if (selection.type == SEL_TYPE_SAMPLE){
		if (cur_action)
			audio->execute(cur_action);
	}else if (selection.type == SEL_TYPE_MIDI_PITCH){
		cur_track->addMidiEvents(midi_notes_to_events(getCreationNotes()));

		midi_preview_renderer->endAllNotes();
	}
	cur_action = NULL;

	// TODO !!!!!!!!
	selection.type = SEL_TYPE_NONE;
	forceRedraw();
	updateMenu();
}



void AudioView::onMiddleButtonDown()
{
	selectUnderMouse();

	selectNone();
	updateMenu();
}



void AudioView::onMiddleButtonUp()
{
}



void AudioView::onRightButtonDown()
{
	selectUnderMouse();

	// pop up menu...
	updateMenu();

	if (selection.type == SEL_TYPE_SAMPLE)
		menu_sub->openPopup(win, 0, 0);
	else if ((selection.type == SEL_TYPE_TRACK) or (selection.type == SEL_TYPE_TRACK_HANDLE)){
		menu_track->enable("track_edit_midi", cur_track->type == Track::TYPE_MIDI);
		menu_track->openPopup(win, 0, 0);
	}else if (!selection.track)
		menu_audio->openPopup(win, 0, 0);
}



void AudioView::onRightButtonUp()
{
}



void AudioView::onLeftDoubleClick()
{
	selectUnderMouse();

	if (mouse_possibly_selecting < mouse_min_move_to_select){
		if (selection.type == SEL_TYPE_SAMPLE){
			win->side_bar->open(SideBar::SAMPLEREF_DIALOG);
		}else if ((selection.type == SEL_TYPE_TRACK) or (selection.type == SEL_TYPE_TRACK_HANDLE) or ((selection.track) and ((selection.type == SEL_TYPE_SELECTION_START) or (selection.type == SEL_TYPE_SELECTION_END)))){
			win->side_bar->open(SideBar::TRACK_CONSOLE);
		}else if (!selection.track){
			win->side_bar->open(SideBar::AUDIOFILE_CONSOLE);
		}
		selection.type = SEL_TYPE_NONE;
	}
	updateMenu();
}



void AudioView::onCommand(const string & id)
{
}



void AudioView::onKeyDown()
{
	int k = HuiGetEvent()->key_code;

// view
	// moving
	float dt = 0.05f;
	if (k == KEY_RIGHT)
		move(ScrollSpeed * dt / view_zoom);
	if (k == KEY_LEFT)
		move(- ScrollSpeed * dt / view_zoom);
	if (k == KEY_NEXT)
		move(ScrollSpeedFast * dt / view_zoom);
	if (k == KEY_PRIOR)
		move(- ScrollSpeedFast * dt / view_zoom);
	// zoom
	if (k == KEY_ADD)
		zoom(exp(  ZoomSpeed));
	if (k == KEY_SUBTRACT)
		zoom(exp(- ZoomSpeed));

	if (k == KEY_SPACE){
		if (stream->isPlaying()){
			stream->pause();
		}else{
			win->onPlay();
		}
	}
	updateMenu();
}



void AudioView::onKeyUp()
{
}



void AudioView::onMouseWheel()
{
	zoom(exp(ZoomSpeed * HuiGetEvent()->dz));
}


void AudioView::forceRedraw()
{
	force_redraw = true;
	win->redraw("area");
}



void AudioView::updateBufferZoom()
{
	prefered_buffer_level = 0;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (view_zoom < 0.8f)
		for (int i=24-1;i>=0;i--){
			double _f = pow(2, (double)i);
			if (_f > 1.0 / view_zoom){
				prefered_buffer_level = i;
				buffer_zoom_factor = _f;
			}
		}
}

void DrawStrBg(HuiPainter *c, float x, float y, const string &str, const color &fg, const color &bg)
{
	color bg2 = bg;
	bg2.a = 0.6f;
	c->setColor(bg2);
	c->drawRect(x, y, c->getStrWidth(str), AudioView::FONT_SIZE * 1.5f);
	c->setColor(fg);
	c->drawStr(x, y, str);
}

int AudioView::y2pitch(int y)
{
	int ti = cur_track->get_index();
	AudioViewTrack *t = vtrack[ti];
	return pitch_min + ((t->area.y2 - y) * (pitch_max - pitch_min) / t->area.height());
}

float AudioView::pitch2y(int p)
{
	int ti = cur_track->get_index();
	AudioViewTrack *t = vtrack[ti];
	return t->area.y2 - t->area.height() * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void AudioView::drawGridTime(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	double dl = AudioViewTrack::MIN_GRID_DIST / view_zoom; // >= 10 pixel
	double dt = dl / audio->sample_rate;
	double ldt = log10(dt);
	double factor = 1;
	if (ldt > 1.5)
		factor = 1.0/0.6/0.60000001;
	else if (ldt > 0)
		factor = 1.0/0.600000001;
	ldt += log10(factor);
	double exp_s = ceil(ldt);
	double exp_s_mod = exp_s - ldt;
	dt = pow(10, exp_s) / factor;
	dl = dt * audio->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dl);
	int nx1 = ceil(screen2sample(r.x2) / dl);
	color c1 = ColorInterpolate(bg, colors.grid, exp_s_mod);
	color c2 = colors.grid;
	for (int n=nx0; n<nx1; n++){
		c->setColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dl);
		c->drawLine(xx, r.y1, xx, r.y2);
	}
	if (show_time){
		if (stream->isPlaying()){
			color cc = colors.preview_marker;
			cc.a = 0.25f;
			c->setColor(cc);
			float x0 = sample2screen(renderer->range().start());
			float x1 = sample2screen(renderer->range().end());
			c->drawRect(x0, r.y1, x1 - x0, r.y1 + TIME_SCALE_HEIGHT);
		}
		c->setColor(colors.grid);
		for (int n=nx0; n<nx1; n++){
			if ((sample2screen(dl) - sample2screen(0)) > 25){
				if (n % 5 == 0)
					c->drawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy((double)n * dl, dt * 5));
			}else{
				if ((n % 10) == 0)
					c->drawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy((double)n * dl, dt * 10));
			}
		}
	}
}

bool AudioView::editingMidi()
{
	if (!cur_track)
		return false;
	if (cur_track->type != Track::TYPE_MIDI)
		return false;
	return win->side_bar->isActive(SideBar::TRACK_MIDI_EDITOR);
}

void AudioView::drawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	Track *t = audio->getTimeTrack();
	if (!t)
		return;
	bool editing_midi = editingMidi();
	int s0 = screen2sample(r.x1 - 1);
	int s1 = screen2sample(r.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash, no_dash;
	dash.add(6);
	dash.add(4);
	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar> bars = t->bars.getBars(Range(s0, s1 - s0));
	foreach(Bar &b, bars){
		int xx = sample2screen(b.range.offset);

		float dx_bar = dsample2screen(b.range.num);
		float dx_beat = dx_bar / b.num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b.index % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			c->setColor(ColorInterpolate(bg, colors.text_soft1, f1));
			c->setLineDash(no_dash, r.y1);
			c->drawLine(xx, r.y1, xx, r.y2);
		}

		if (f2 >= 0.1f){
			color c1 = ColorInterpolate(bg, colors.text_soft1, f2);
			float beat_length = (float)b.range.num / (float)b.num_beats;
			c->setLineDash(dash, r.y1);
			for (int i=0; i<b.num_beats; i++){
				float beat_offset = b.range.offset + (float)i * beat_length;
				if (editing_midi){
					color c2 = ColorInterpolate(bg, c1, 0.6f);
					c->setColor(c2);
					for (int j=1; j<beat_partition; j++){
						int x = sample2screen(beat_offset + beat_length * j / beat_partition);
						c->drawLine(x, r.y1, x, r.y2);
					}
				}
				if (i == 0)
					continue;
				c->setColor(c1);
				int x = sample2screen(beat_offset);
				c->drawLine(x, r.y1, x, r.y2);
			}
		}

		if ((show_time) and (f1 > 0.9f)){
			c->setColor(colors.text_soft1);
			c->drawStr(xx + 2, r.y1, i2s(b.index + 1));
		}
	}
	c->setLineDash(no_dash, 0);
	c->setLineWidth(LINE_WIDTH);
}

void AudioView::checkConsistency()
{
	// check cur_track consistency
	int n = get_track_index_save(cur_track);
	if ((cur_track) and (n < 0))
		if (audio->tracks.num > 0)
			setCurTrack(audio->tracks[0]);

	// check cur_level consistency
	if ((cur_level < 0) or (cur_level >= audio->level_names.num)){
		cur_level = 0;
		forceRedraw();
	}
}

void AudioView::onUpdate(Observable *o, const string &message)
{
	checkConsistency();

	if (o == audio){
		if (message == audio->MESSAGE_NEW){
			updateTracks();
			sel_range = sel_raw = Range(0, 0);
			setCurTrack(NULL);
			if (audio->tracks.num > 0)
				setCurTrack(audio->tracks[0]);
			optimizeView();
		}else{
			if ((message == audio->MESSAGE_ADD_TRACK) or (message == audio->MESSAGE_DELETE_TRACK))
				updateTracks();
			forceRedraw();
			updateMenu();
		}

		if (message == audio->MESSAGE_CHANGE)
			audio->updatePeaks(peak_mode);
	}else if (o == stream){
		if (stream->isPlaying())
			makeSampleVisible(stream->getPos());
		forceRedraw();
	}else if (o == input){
		if (input->isCapturing())
			makeSampleVisible(sel_range.start() + input->getSampleCount());
		forceRedraw();
	}else{
		forceRedraw();
	}
}

void AudioView::updateTracks()
{
	Array<AudioViewTrack*> vtrack2;
	vtrack2.resize(audio->tracks.num);
	foreachi(Track *t, audio->tracks, ti){
		bool found = false;
		foreachi(AudioViewTrack *v, vtrack, vi)
			if (v){
				if (v->track == t){
					vtrack2[ti] = v;
					vtrack[vi] = NULL;
					found = true;
					break;
				}
			}
		if (!found)
			vtrack2[ti] = new AudioViewTrack(this, t);
	}
	foreach(AudioViewTrack *v, vtrack)
		if (v)
			delete(v);
	vtrack = vtrack2;
	thm.dirty = true;
	foreachi(AudioViewTrack *v, vtrack, i){
		if (i > 0){
			if (v->area.y1 < vtrack[i-1]->area.y2){
				v->area.y1 = vtrack[i-1]->area.y2;
				v->area.y2 = vtrack[i-1]->area.y2;
			}
		}
	}

	checkConsistency();
}

bool AudioView::TrackHeightManager::check(AudioFile *a)
{
	return true;
}

rect rect_inter(const rect &a, const rect &b, float t)
{
	return rect((1-t) * a.x1 + t * b.x1,
			(1-t) * a.x2 + t * b.x2,
			(1-t) * a.y1 + t * b.y1,
			(1-t) * a.y2 + t * b.y2);
}

void AudioView::TrackHeightManager::update(AudioView *v, AudioFile *a, const rect &r)
{
	Track *new_midi_track = (v->editingMidi() ? v->cur_track : NULL);
	if ((dirty) or (render_area != r) or (midi_track != new_midi_track)){
		plan(v, a, r);
		t = 0;
		animating = true;
		dirty = false;

		foreach(AudioViewTrack *v, v->vtrack)
			v->area_last = v->area;
	}
	midi_track = new_midi_track;
	foreach(AudioViewTrack *v, v->vtrack){
		v->area.x1 = v->area_target.x1 = v->area_last.x1 = r.x1;
		v->area.x2 = v->area_target.x2 = v->area_last.x2 = r.x2;
	}


	if (render_area != r){
		render_area = r;
		foreach(AudioViewTrack *v, v->vtrack)
			v->area = v->area_target;
		animating = false;
		t = 0;
	}

	if (!animating)
		return;

	t += 0.07f;
	if (t >= 1){
		t = 1;
		animating = false;
	}
	foreach(AudioViewTrack *v, v->vtrack)
		v->area = rect_inter(v->area_last, v->area_target, (t < 0.5f) ? 2*t*t : -2*t*t+4*t-1);

	if (animating)
		HuiRunLaterM(0.03f, v, &AudioView::forceRedraw);
}

void AudioView::TrackHeightManager::plan(AudioView *v, AudioFile *a, const rect &r)
{
	if (v->editingMidi()){
		float y0 = v->TIME_SCALE_HEIGHT;
		foreachi(AudioViewTrack *t, v->vtrack, i){
			float h = v->TIME_SCALE_HEIGHT;
			if (t->track == v->cur_track)
				h = r.height() - a->tracks.num * v->TIME_SCALE_HEIGHT;
			t->area_target = rect(r.x1, r.x2, y0, y0 + h);
			y0 += h;
		}
		return;
	}
	int n_ch = v->show_mono ? 1 : 2;

	int h_wish = v->TIME_SCALE_HEIGHT;
	int h_fix = v->TIME_SCALE_HEIGHT;
	int n_var = 0;
	foreach(Track *t, a->tracks){
		if (t->type == t->TYPE_AUDIO){
			h_wish += MAX_TRACK_CHANNEL_HEIGHT * n_ch;
			n_var += n_ch;
		}else if (t->type == t->TYPE_MIDI){
			h_wish += MAX_TRACK_CHANNEL_HEIGHT;
			n_var ++;
		}else{
			h_wish += v->TIME_SCALE_HEIGHT * 2;
			h_fix += v->TIME_SCALE_HEIGHT * 2;
		}
	}

	int y0 = r.y1 + v->TIME_SCALE_HEIGHT;
	int opt_channel_height = MAX_TRACK_CHANNEL_HEIGHT;
	if (h_wish > r.height())
		opt_channel_height = (r.height() - h_fix) / n_var;
	foreachi(AudioViewTrack *t, v->vtrack, i){
		float h = v->TIME_SCALE_HEIGHT*2;
		if (t->track->type == Track::TYPE_AUDIO)
			h = opt_channel_height * n_ch;
		else if (t->track->type == Track::TYPE_MIDI)
			h = opt_channel_height;
		t->area_target = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

void AudioView::drawTimeLine(HuiPainter *c, int pos, int type, color &col, bool show_time)
{
	int p = sample2screen(pos);
	if ((p >= area.x1) and (p <= area.x2)){
		c->setColor((type == hover.type) ? colors.selection_boundary_hover : col);
		c->drawLine(p, area.y1, p, area.y2);
		if (show_time)
			c->drawStr(p, (area.y1 + area.y2) / 2, audio->get_time_str_long(pos));
	}
}

void AudioView::drawBackground(HuiPainter *c, const rect &r)
{
	int yy = 0;
	if (vtrack.num > 0)
		yy = vtrack.back()->area.y2;

	// time scale
	c->setColor(colors.background_track);
	c->drawRect(r.x1, r.y1, r.width(), TIME_SCALE_HEIGHT);
	drawGridTime(c, rect(r.x1, r.x2, r.y1, r.y1 + TIME_SCALE_HEIGHT), colors.background_track, true);

	// tracks
	foreachi(AudioViewTrack *t, vtrack, i){
		color cc = (t->track->is_selected) ? colors.background_track_selected : colors.background_track;
		c->setColor(cc);
		c->drawRect(t->area);

		if (t->track->type == Track::TYPE_TIME){
			drawGridBars(c, t->area, cc, true);
		}else{
			drawGridTime(c, t->area, cc, false);
			drawGridBars(c, t->area, cc, false);
		}

		if ((t->track == cur_track) and (editingMidi())){
			// pitch grid
			c->setColor(color(0.25f, 0, 0, 0));
			for (int i=pitch_min; i<pitch_max; i++){
				float y0 = pitch2y(i + 1);
				float y1 = pitch2y(i);
				if (is_sharp(i)){
					c->setColor(color(0.2f, 0, 0, 0));
					c->drawRect(r.x1, y0, r.width(), y1 - y0);
				}
			}
		}
	}

	// free space below tracks
	if (yy < r.y2){
		c->setColor(colors.background);
		rect rr = rect(r.x1, r.x2, yy, r.y2);
		c->drawRect(rr);
		drawGridTime(c, rr, colors.background, false);
	}

	// lines between tracks
	c->setColor(colors.grid);
	foreachi(AudioViewTrack *t, vtrack, i)
		c->drawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->drawLine(0, yy, r.width(), yy);

	//DrawGrid(c, r, ColorBackgroundCurWave, true);
}

void AudioView::drawSelection(HuiPainter *c, const rect &r)
{
	int sx1 = sample2screen(sel_range.start());
	int sx2 = sample2screen(sel_range.end());
	int sxx1 = clampi(sx1, r.x1, r.x2);
	int sxx2 = clampi(sx2, r.x1, r.x2);
	c->setColor(colors.selection_internal);
	foreachi(AudioViewTrack *t, vtrack, i)
		if (t->track->is_selected)
			c->drawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
	drawTimeLine(c, sel_raw.start(), SEL_TYPE_SELECTION_START, colors.selection_boundary);
	drawTimeLine(c, sel_raw.end(), SEL_TYPE_SELECTION_END, colors.selection_boundary);
}

void AudioView::drawAudioFile(HuiPainter *c, const rect &r)
{
	area = r;

	thm.update(this, audio, r);
	updateBufferZoom();

	// background
	drawBackground(c, r);

	// tracks
	foreachi(AudioViewTrack *t, vtrack, i)
		t->drawTrack(c, t->area, i);


	// selection
	drawSelection(c, r);

	// playing position
	if (stream->isPlaying()){
		if (!input->isCapturing())
			drawTimeLine(c, stream->getPos(), SEL_TYPE_PLAYBACK, colors.preview_marker, true);
	}

	// capturing position
	if (input->isCapturing())
		drawTimeLine(c, sel_range.start() + input->getSampleCount(), SEL_TYPE_PLAYBACK, colors.capture_marker, true);
}

int frame=0;

void AudioView::onDraw()
{
	msg_db_f("OnDraw", 1);
	force_redraw = false;

	HuiPainter *c = win->beginDraw("area");
	drawing_rect = rect(0, c->width, 0, c->height);
	c->setFontSize(FONT_SIZE);
	c->setLineWidth(LINE_WIDTH);
	c->setAntialiasing(antialiasing);
	//c->setColor(ColorWaveCur);

	if (enabled)
		drawAudioFile(c, rect(0, c->width, 0, c->height));

	//c->DrawStr(100, 100, i2s(frame++));

	c->end();
}

void AudioView::optimizeView()
{
	msg_db_f("OptimizeView", 1);
	if (area.x2 <= 0)
		area.x2 = drawing_rect.x2;

	Range r = audio->getRange();

	if (r.num == 0)
		r.num = 10 * audio->sample_rate;
	int border = r.num * BORDER_FACTOR;
	r.offset -= border;
	r.num += border * 2;
	view_zoom = area.width() / (double)r.length();
	view_pos = (double)r.start();
	notify(MESSAGE_VIEW_CHANGE);
	forceRedraw();
}

void AudioView::updateMenu()
{
	// view
	win->check("view_mono", show_mono);
	win->check("view_stereo", !show_mono);
	win->check("view_peaks_max", peak_mode == BufferBox::PEAK_MODE_MAXIMUM);
	win->check("view_peaks_mean", peak_mode == BufferBox::PEAK_MODE_SQUAREMEAN);
	win->enable("view_samples", false);
}

void AudioView::setPeaksMode(int mode)
{
	peak_mode = mode;
	audio->invalidateAllPeaks();
	audio->updatePeaks(peak_mode);
	forceRedraw();
	updateMenu();
}

void AudioView::setShowMono(bool mono)
{
	show_mono = mono;
	forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	updateMenu();
}

void AudioView::zoomIn()
{
	zoom(2.0f);
}

void AudioView::zoomOut()
{
	zoom(0.5f);
}
void AudioView::makeSampleVisible(int sample)
{
	double x = sample2screen(sample);
	if ((x > area.x2) or (x < area.x1)){
		view_pos = sample - area.width() / view_zoom * BORDER_FACTOR;
		notify(MESSAGE_VIEW_CHANGE);
		forceRedraw();
	}
}

void AudioView::selectAll()
{
	sel_raw = audio->getRange();
	updateSelection();
}

void AudioView::selectNone()
{
	// select all/none
	sel_raw.clear();
	updateSelection();
	audio->unselectAllSamples();
	setCurSample(NULL);
}



void AudioView::selectSample(SampleRef *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		s->is_selected = !s->is_selected;
	}else{
		if (!s->is_selected)
			s->owner->unselectAllSamples();

		// select this sub
		s->is_selected = true;
	}
}

void AudioView::selectTrack(Track *t, bool diff)
{
	if (!t)
		return;
	if (diff){
		bool is_only_selected = true;
		foreach(Track *tt, t->root->tracks)
			if ((tt->is_selected) and (tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected or is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(Track *tt, t->root->tracks)
				tt->is_selected = false;
		}

		// select this track
		t->is_selected = true;
	}
	updateSelection();
}

void AudioView::setCurSample(SampleRef *s)
{
	if (cur_sample == s)
		return;
	cur_sample = s;
	forceRedraw();
	notify(MESSAGE_CUR_SAMPLE_CHANGE);
}


void AudioView::setCurTrack(Track *t)
{
	if (cur_track == t)
		return;
	cur_track = t;
	forceRedraw();
	notify(MESSAGE_CUR_TRACK_CHANGE);
}

void AudioView::setCurLevel(int l)
{
	if (cur_level == l)
		return;
	if ((l < 0) or (l >= audio->level_names.num))
		return;
	cur_level = l;
	forceRedraw();
	notify(MESSAGE_CUR_LEVEL_CHANGE);
}



double AudioView::screen2sample(double _x)
{
	return (_x - area.x1) / view_zoom + view_pos;
}

double AudioView::sample2screen(double s)
{
	return area.x1 + (s - view_pos) * view_zoom;
}

double AudioView::dsample2screen(double ds)
{
	return ds * view_zoom;
}

void AudioView::zoom(float f)
{
	// max zoom: 20 pixel per sample
	double zoom_new = clampf(view_zoom * f, 0.000001, 20.0);

	view_pos += (mx - area.x1) * (1.0/view_zoom - 1.0/zoom_new);
	view_zoom = zoom_new;
	notify(MESSAGE_VIEW_CHANGE);
	forceRedraw();
}

void AudioView::move(float dpos)
{
	view_pos += dpos;
	notify(MESSAGE_VIEW_CHANGE);
	forceRedraw();
}

void AudioView::enable(bool _enabled)
{
	if (enabled and !_enabled)
		unsubscribe(audio);
	else if (!enabled and _enabled)
		subscribe(audio);
	enabled = _enabled;
}



