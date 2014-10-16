/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "SideBar/SideBar.h"
#include "BottomBar/BottomBar.h"
#include "../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../Audio/AudioInput.h"
#include "../Audio/AudioOutput.h"
#include "../Audio/AudioRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Stuff/Log.h"
#include "../lib/math/math.h"

const int FONT_SIZE_NO_FILE = 12;
const int FONT_SIZE = 10;
const int MAX_TRACK_CHANNEL_HEIGHT = 125;
const float LINE_WIDTH = 1.0f;

const float BORDER_FACTOR = 1.0f / 15.0f;

int get_track_index_save(Track *t)
{
	if (t){
		foreachi(Track *tt, tsunami->audio->track, i)
			if (t == tt)
				return i;
	}
	return -1;
}


static bool is_sharp(int pitch)
{
	int r = pitch % 12;
	// 69 = 9 = a
	return ((r == 10) || (r == 1) || (r == 3) || (r == 6) || (r == 8));
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
	sample = NULL;
	pos = 0;
	sample_offset = 0;
	show_track_controls = NULL;
	note = -1;
	pitch = -1;
}

AudioView::AudioView(TsunamiWindow *parent, AudioFile *_audio, AudioOutput *_output, AudioInput *_input, AudioRenderer *_renderer) :
	Observer("AudioView"),
	Observable("AudioView"),
	SUB_FRAME_HEIGHT(20),
	TIME_SCALE_HEIGHT(20),
	BarrierDist(5)
{
	win = parent;

	ColorBackground = White;
	ColorBackgroundCurWave = color(1, 0.93f, 0.93f, 1);
	ColorBackgroundCurTrack = color(1, 0.88f, 0.88f, 1);
	ColorGrid = color(1, 0.75f, 0.75f, 0.9f);
	ColorSelectionInternal = color(0.2f, 0.2f, 0.2f, 0.8f);
	ColorSelectionBoundary = color(1, 0.2f, 0.2f, 0.8f);
	ColorSelectionBoundaryMO = color(1, 0.8f, 0.2f, 0.2f);
	ColorPreviewMarker = color(1, 0, 0.7f, 0);
	ColorCaptureMarker = color(1, 0.7f, 0, 0);
	ColorWave = Gray;
	ColorWaveCur = color(1, 0.3f, 0.3f, 0.3f);
	ColorSub = color(1, 0.6f, 0.6f, 0);
	ColorSubMO = color(1, 0.6f, 0, 0);
	ColorSubNotCur = color(1, 0.4f, 0.4f, 0.4f);

	drawing_rect = rect(0, 1024, 0, 768);
	bottom_button_rect = rect(0, 0, 0, 0);

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
	output = _output;
	input = _input;
	renderer = _renderer;

	pitch_min = 60;
	pitch_max = 90;
	beat_partition = 4;
	parent->SetInt("beat_partition", beat_partition);
	midi_mode = MIDI_MODE_SELECT;
	chord_type = 0;
	chord_inversion = 0;

	audio->area = rect(0, 0, 0, 0);
	Subscribe(audio);
	Subscribe(output);
	Subscribe(input);

	// events
	parent->EventMX("area", "hui:draw", this, &AudioView::OnDraw);
	parent->EventMX("area", "hui:mouse-move", this, &AudioView::OnMouseMove);
	parent->EventMX("area", "hui:left-button-down", this, &AudioView::OnLeftButtonDown);
	parent->EventMX("area", "hui:left-double-click", this, &AudioView::OnLeftDoubleClick);
	parent->EventMX("area", "hui:left-button-up", this, &AudioView::OnLeftButtonUp);
	parent->EventMX("area", "hui:middle-button-down", this, &AudioView::OnMiddleButtonDown);
	parent->EventMX("area", "hui:middle-button-up", this, &AudioView::OnMiddleButtonUp);
	parent->EventMX("area", "hui:right-button-down", this, &AudioView::OnRightButtonDown);
	parent->EventMX("area", "hui:right-button-up", this, &AudioView::OnRightButtonUp);
	//parent->EventMX("area", "hui:key-down", this, &AudioView::OnKeyDown);
	parent->EventMX("area", "hui:key-down", this, &AudioView::OnKeyDown);
	parent->EventMX("area", "hui:key-up", this, &AudioView::OnKeyUp);
	parent->EventMX("area", "hui:mouse-wheel", this, &AudioView::OnMouseWheel);

	parent->Activate("area");


	menu_audio = HuiCreateResourceMenu("popup_audio_file_menu");
	menu_track = HuiCreateResourceMenu("popup_track_menu");
	menu_sub= HuiCreateResourceMenu("popup_sample_menu");

	//ForceRedraw();
	UpdateMenu();
}

AudioView::~AudioView()
{
	Unsubscribe(audio);
	Unsubscribe(output);
	Unsubscribe(input);

	HuiConfig.setBool("View.Mono", show_mono);
	HuiConfig.setInt("View.DetailSteps", detail_steps);
	HuiConfig.setInt("View.MouseMinMoveToSelect", mouse_min_move_to_select);
	HuiConfig.setInt("View.ScrollSpeed", ScrollSpeed);
	HuiConfig.setInt("View.ScrollSpeedFast", ScrollSpeedFast);
	HuiConfig.setFloat("View.ZoomSpeed", ZoomSpeed);
	HuiConfig.setBool("View.Antialiasing", antialiasing);
}


void AudioView::SetMouse()
{
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my;
}

bool AudioView::MouseOverTrack(Track *t)
{
	return t->area.inside(mx, my);
}

int AudioView::MouseOverSample(SampleRef *s)
{
	if ((mx >= s->area.x1) && (mx < s->area.x2)){
		int offset = screen2sample(mx) - s->pos;
		if ((my >= s->area.y1) && (my < s->area.y1 + SUB_FRAME_HEIGHT))
			return offset;
		if ((my >= s->area.y2 - SUB_FRAME_HEIGHT) && (my < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::SelectionUpdatePos(SelectionType &s)
{
	s.pos = screen2sample(mx);
}

void AudioView::UpdateSelection()
{
	msg_db_f("UpdateSelection", 1);
	sel_range = sel_raw;
	if (sel_range.num < 0)
		sel_range.invert();


	renderer->range = GetPlaybackSelection();
	if (output->IsPlaying()){
		if (renderer->range.is_inside(output->GetPos()))
			renderer->range = GetPlaybackSelection();
		else
			output->Stop();
	}

	audio->UpdateSelection(sel_range);
	Notify(MESSAGE_SELECTION_CHANGE);
}

bool mouse_over_time(AudioView *v, int pos)
{
	int ssx = v->sample2screen(pos);
	return ((v->mx >= ssx - 5) && (v->mx <= ssx + 5));
}

AudioView::SelectionType AudioView::GetMouseOver()
{
	SelectionType s;

	// track?
	foreach(Track *t, audio->track){
		if (MouseOverTrack(t)){
			s.track = t;
			s.type = SEL_TYPE_TRACK;
			if (mx < t->area.x1 + 100)
				s.show_track_controls = t;
		}
	}

	if (!win->bottom_bar->visible)
		if (bottom_button_rect.inside(mx, my)){
			s.type = SEL_TYPE_BOTTOM_BUTTON;
			return s;
		}

	// selection boundaries?
	SelectionUpdatePos(s);
	if (mouse_over_time(this, sel_raw.end())){
		s.type = SEL_TYPE_SELECTION_END;
		return s;
	}
	if (mouse_over_time(this, sel_raw.start())){
		s.type = SEL_TYPE_SELECTION_START;
		return s;
	}
	if ((output->IsPlaying()) && (output->GetSource() == renderer)){
		if (mouse_over_time(this, output->GetPos())){
			s.type = SEL_TYPE_PLAYBACK;
			return s;
		}
	}

	// mute button?
	if (s.track){
		if ((mx >= s.track->area.x1 + 5) && (mx < s.track->area.x1 + 17) && (my >= s.track->area.y1 + 22) && (my < s.track->area.y1 + 34)){
			s.type = SEL_TYPE_MUTE;
			return s;
		}
		if ((audio->track.num > 1) && (mx >= s.track->area.x1 + 22) && (mx < s.track->area.x1 + 34) && (my >= s.track->area.y1 + 22) && (my < s.track->area.y1 + 34)){
			s.type = SEL_TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){
		// TODO: prefer selected subs
		foreach(SampleRef *ss, s.track->sample){
			int offset = MouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = SEL_TYPE_SAMPLE;
				s.sample_offset = offset;
			}
		}
	}

	if ((s.track) && (s.track == cur_track) && (EditingMidi()) && (midi_mode != MIDI_MODE_SELECT)){
		s.pitch = y2pitch(my);
		s.type = SEL_TYPE_MIDI_PITCH;
		foreachi(MidiNote &n, s.track->midi, i)
			if ((n.pitch == s.pitch) && (n.range.is_inside(s.pos))){
				s.note = i;
				s.type = SEL_TYPE_MIDI_NOTE;
			}
	}

	return s;
}


Range AudioView::GetPlaybackSelection()
{
	if (sel_range.empty()){
		int num = audio->GetRange().end() - sel_range.start();
		if (num <= 0)
			num = audio->sample_rate; // 1 second
		return Range(sel_range.start(), num);
	}
	return sel_range;
}

void AudioView::SelectUnderMouse()
{
	msg_db_f("SelectUnderMouse", 2);
	hover = GetMouseOver();
	selection = hover;
	Track *t = selection.track;
	SampleRef *s = selection.sample;
	bool control = win->GetKey(KEY_CONTROL);

	// track
	if (selection.track)
		SetCurTrack(selection.track);
	if (selection.type == SEL_TYPE_TRACK){
		SelectTrack(t, control);
		if (!control)
			audio->UnselectAllSamples();
	}

	// sub
	SetCurSample(s);
	if (selection.type == SEL_TYPE_SAMPLE){
		SelectSample(s, control);
	}
}

void AudioView::SetBarriers(SelectionType *s)
{
	msg_db_f("SetBarriers", 2);
	s->barrier.clear();
	if (s->type == SEL_TYPE_NONE)
		return;

	int dpos = 0;
	if (s->type == SEL_TYPE_SAMPLE)
		dpos = s->sample_offset;

	foreach(Track *t, audio->track){
		// add subs
		foreach(SampleRef *sam, t->sample){
			s->barrier.add(sam->pos + dpos);
		}

		// time bar...
		int x0 = 0;
		foreach(BarPattern &b, t->bar){
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

void AudioView::ApplyBarriers(int &pos)
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
			|| (hover.sample != hover_old.sample)
			|| (hover.show_track_controls != hover_old.show_track_controls)
			|| (hover.note != hover_old.note)
			|| (hover.pitch != hover_old.pitch);
}

void AudioView::OnMouseMove()
{
	msg_db_f("OnMouseMove", 2);
	SetMouse();
	bool _force_redraw_ = false;

	if (HuiGetEvent()->lbut){
		SelectionUpdatePos(selection);
	}else{
		SelectionType hover_old = hover;
		hover = GetMouseOver();
		_force_redraw_ |= hover_changed(hover, hover_old);
	}


	// drag & drop
	if (selection.type == SEL_TYPE_SELECTION_END){
		SelectionType mo = GetMouseOver();
		if (mo.track)
			mo.track->is_selected = true;

		ApplyBarriers(selection.pos);
		sel_raw.set_end(selection.pos);
		UpdateSelection();
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
		win->RedrawRect("area", x, audio->area.y1, w, audio->area.height());
	}else if (selection.type == SEL_TYPE_PLAYBACK){
		renderer->Seek(selection.pos);
		output->Play(renderer);
		_force_redraw_ = true;
	}else if (selection.type == SEL_TYPE_SAMPLE){
		ApplyBarriers(selection.pos);
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
		SetBarriers(&selection);
		UpdateSelection();
		selection.type = SEL_TYPE_SELECTION_END;
		hover.type = SEL_TYPE_SELECTION_END;
		_force_redraw_ = true;
		mouse_possibly_selecting = -1;
	}

	if ((HuiGetEvent()->lbut) && (selection.type == SEL_TYPE_MIDI_PITCH))
		_force_redraw_ = true;

	if (_force_redraw_)
		ForceRedraw();
}


void AudioView::SetCursorPos(int pos)
{
	if (output->IsPlaying()){
		if (renderer->range.is_inside(pos)){
			renderer->Seek(pos);
			output->Play(renderer);
			selection.type = SEL_TYPE_PLAYBACK;
			ForceRedraw();
			return;
		}else
			output->Stop();
	}
	mouse_possibly_selecting = 0;
	sel_raw = Range(pos, 0);
	UpdateSelection();
}

void AudioView::OnLeftButtonDown()
{
	msg_db_f("OnLBD", 2);
	SetMouse();
	SelectUnderMouse();
	UpdateMenu();

	if (!audio->used)
		return;

	mouse_possibly_selecting_start = selection.pos;

	// selection:
	//   start after lb down and moving
	if ((selection.type == SEL_TYPE_TRACK) || (selection.type == SEL_TYPE_TIME)){
		SetCursorPos(selection.pos);
	}else if (selection.type == SEL_TYPE_SELECTION_START){
		// switch end / start
		selection.type = SEL_TYPE_SELECTION_END;
		hover.type = SEL_TYPE_SELECTION_END;
		sel_raw.invert();
	}else if (selection.type == SEL_TYPE_MUTE){
		selection.track->SetMuted(!selection.track->muted);
	}else if (selection.type == SEL_TYPE_SOLO){
		foreach(Track *t, audio->track)
			t->is_selected = (t == selection.track);
		if (selection.track->muted)
			selection.track->SetMuted(false);
	}else if (selection.type == SEL_TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(audio);
	}else if (selection.type == SEL_TYPE_MIDI_NOTE){
		cur_track->DeleteMidiNote(selection.note);
	}else if (selection.type == SEL_TYPE_MIDI_PITCH){

	}else if (selection.type == SEL_TYPE_BOTTOM_BUTTON){
		win->bottom_bar->Show();
	}

	SetBarriers(&selection);

	ForceRedraw();
	UpdateMenu();
}


Array<MidiNote> AudioView::GetSelectedNotes()
{
	int start = min(mouse_possibly_selecting_start, selection.pos);
	int end = max(mouse_possibly_selecting_start, selection.pos);
	Track *t = audio->GetTimeTrack();
	if (t){
		Array<Beat> beats = t->bar.GetBeats(audio->GetRange());
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


void AudioView::OnLeftButtonUp()
{
	msg_db_f("OnLBU", 2);
	if (selection.type == SEL_TYPE_SAMPLE){
		if (cur_action)
			audio->Execute(cur_action);
	}else if (selection.type == SEL_TYPE_MIDI_PITCH){
		cur_track->AddMidiNotes(GetSelectedNotes());
	}
	cur_action = NULL;

	// TODO !!!!!!!!
	selection.type = SEL_TYPE_NONE;
	ForceRedraw();
	UpdateMenu();
}



void AudioView::OnMiddleButtonDown()
{
	SelectUnderMouse();

	SelectNone();
	UpdateMenu();
}



void AudioView::OnMiddleButtonUp()
{
}



void AudioView::OnRightButtonDown()
{
	SelectUnderMouse();

	// pop up menu...
	UpdateMenu();

	if (selection.type == SEL_TYPE_SAMPLE)
		menu_sub->OpenPopup(win, 0, 0);
	else if (selection.type == SEL_TYPE_TRACK){
		menu_track->Enable("track_edit_midi", cur_track->type == Track::TYPE_MIDI);
		menu_track->OpenPopup(win, 0, 0);
	}else if (!selection.track)
		menu_audio->OpenPopup(win, 0, 0);
}



void AudioView::OnRightButtonUp()
{
}



void AudioView::OnLeftDoubleClick()
{
	SelectUnderMouse();

	if (mouse_possibly_selecting < mouse_min_move_to_select)
		if (audio->used){
			if (selection.type == SEL_TYPE_SAMPLE){
				win->side_bar->Open(SideBar::SUB_DIALOG);
			}else if ((selection.type == SEL_TYPE_TRACK) || ((selection.track) && ((selection.type == SEL_TYPE_SELECTION_START) || (selection.type == SEL_TYPE_SELECTION_END)))){
				win->side_bar->Open(SideBar::TRACK_DIALOG);
			}else if (!selection.track){
				SetCurTrack(NULL);
				win->side_bar->Open(SideBar::AUDIO_FILE_DIALOG);
			}
			selection.type = SEL_TYPE_NONE;
		}
	UpdateMenu();
}



void AudioView::OnCommand(const string & id)
{
}



void AudioView::OnKeyDown()
{
	int k = HuiGetEvent()->key_code;

// view
	// moving
	float dt = 0.05f;
	if (k == KEY_RIGHT)
		Move(ScrollSpeed * dt / view_zoom);
	if (k == KEY_LEFT)
		Move(- ScrollSpeed * dt / view_zoom);
	if (k == KEY_NEXT)
		Move(ScrollSpeedFast * dt / view_zoom);
	if (k == KEY_PRIOR)
		Move(- ScrollSpeedFast * dt / view_zoom);
	// zoom
	if (k == KEY_ADD)
		Zoom(exp(  ZoomSpeed));
	if (k == KEY_SUBTRACT)
		Zoom(exp(- ZoomSpeed));

	if (k == KEY_SPACE){
		if (output->IsPlaying()){
			output->Pause();
		}else{
			win->OnPlay();
		}
	}
	UpdateMenu();
}



void AudioView::OnKeyUp()
{
}



void AudioView::OnMouseWheel()
{
	Zoom(exp(ZoomSpeed * HuiGetEvent()->dz));
}


void AudioView::ForceRedraw()
{
	force_redraw = true;
	win->Redraw("area");
}


#define MIN_GRID_DIST	10.0f

static Array<complex> tt;

inline void draw_line_buffer(HuiPainter *c, int width, double view_pos, double zoom, float hf, float x, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double) x          / zoom + view_pos - offset    , 0);
	int i1 = min((double)(x + width) / zoom + view_pos - offset + 2, buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);

	for (int i=i0; i<i1; i++){

		double p = x + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 + buf[i] * hf;
		if (zoom > 5)
			c->drawCircle(p, tt[nl].y, 2);
		nl ++;
	}
	c->drawLines(tt);
}

inline void draw_peak_buffer(HuiPainter *c, int width, int di, double view_pos_rel, double zoom, float f, float hf, float x, float y0, const string &buf, int offset)
{
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize(width/di + 10);
	for (int i=0; i<width+di; i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) && (ip < buf.num))
		if (((int)(p) < offset + buf.num*f) && (p >= offset)){
			tt[nl].x = (float)x+i;
			float dy = ((float)(buf[ip])/255.0f) * hf;
			tt[nl].y  = y0 - dy;
			nl ++;
		}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++){
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->drawPolygon(tt);
}

void AudioView::UpdateBufferZoom()
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

void AudioView::DrawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col)
{
	msg_db_f("DrawBuffer", 1);

	// zero heights of both channels
	float y0r = r.y1 + r.height() / 4;
	float y0l = r.y1 + r.height() * 3 / 4;

	float hf = r.height() / 4;

	if (show_mono){
		y0r = r.y1 + r.height() / 2;
		hf *= 2;
	}


	int di = detail_steps;
	c->setColor(col);
	int l = min(prefered_buffer_level - 1, b.peak.num / 2);
	if (l >= 1){//f < MIN_MAX_FACTOR){
		draw_peak_buffer(c, r.width(), di, view_pos_rel, view_zoom, buffer_zoom_factor, hf, r.x1, y0r, b.peak[l*2-2], b.offset);
		if (!show_mono)
			draw_peak_buffer(c, r.width(), di, view_pos_rel, view_zoom, buffer_zoom_factor, hf, r.x1, y0l, b.peak[l*2-1], b.offset);
	}else{
		draw_line_buffer(c, r.width(), view_pos_rel, view_zoom, hf, r.x1, y0r, b.r, b.offset);
		if (!show_mono)
			draw_line_buffer(c, r.width(), view_pos_rel, view_zoom, hf, r.x1, y0l, b.l, b.offset);
	}
}

void AudioView::DrawTrackBuffers(HuiPainter *c, const rect &r, Track *t, double view_pos_rel, const color &col)
{
	msg_db_f("DrawTrackBuffers", 1);

	// non-current levels
	foreachi(TrackLevel &lev, t->level, level_no){
		if (level_no == cur_level)
			continue;
		foreach(BufferBox &b, lev.buffer)
			DrawBuffer(c, r, b, view_pos_rel, ColorWave);
	}

	// current
	foreach(BufferBox &b, t->level[cur_level].buffer)
		DrawBuffer(c, r, b, view_pos_rel, col);
}

void AudioView::DrawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay)
{
	// frame
	int asx = clampi(sample2screen(s->pos + delay), r.x1, r.x2);
	int aex = clampi(sample2screen(s->pos + s->buf->num + delay), r.x1, r.x2);

	if (delay == 0)
		s->area = rect(asx, aex, r.y1, r.y2);


	color col2 = col;
	col2.a *= 0.2f;
	c->setColor(col2);
	c->drawRect(asx, r.y1,                    aex - asx, SUB_FRAME_HEIGHT);
	c->drawRect(asx, r.y2 - SUB_FRAME_HEIGHT, aex - asx, SUB_FRAME_HEIGHT);

	c->setColor(col);
	c->drawLine(asx, r.y1, asx, r.y2);
	c->drawLine(aex, r.y1, aex, r.y2);
	c->drawLine(asx, r.y1, aex, r.y1);
	c->drawLine(asx, r.y2, aex, r.y2);
}

void AudioView::DrawSample(HuiPainter *c, const rect &r, SampleRef *s)
{
	color col = ColorSub;
	//bool is_cur = ((s == cur_sub) && (t->IsSelected));
	if (!s->is_selected)
		col = ColorSubNotCur;
	if (hover.sample == s)
		col = ColorSubMO;
	//col.a = 0.2f;

	DrawSampleFrame(c, r, s, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		DrawSampleFrame(c, r, s, col2, (i + 1) * s->rep_delay);

	// buffer
	DrawBuffer(c, r, *s->buf, view_pos - (double)s->pos, col);

	int asx = clampi(sample2screen(s->pos), r.x1, r.x2);
	if (s->is_selected)//((is_cur) || (a->sub_mouse_over == s))
		c->drawStr(asx, r.y2 - SUB_FRAME_HEIGHT, s->origin->name);
}

void DrawStrBg(HuiPainter *c, float x, float y, const string &str, const color &fg, const color &bg)
{
	color bg2 = bg;
	bg2.a = 0.6f;
	c->setColor(bg2);
	c->drawRect(x, y, c->getStrWidth(str), FONT_SIZE * 1.5f);
	c->setColor(fg);
	c->drawStr(x, y, str);
}

color AudioView::GetPitchColor(int pitch)
{
	return SetColorHSB(1, (float)(pitch % 12) / 12.0f, 0.6f, 1);
}

int AudioView::y2pitch(int y)
{
	return pitch_min + ((cur_track->area.y2 - y) * (pitch_max - pitch_min) / cur_track->area.height());
}

float AudioView::pitch2y(int p)
{
	return cur_track->area.y2 - cur_track->area.height() * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void AudioView::DrawMidi(HuiPainter *c, const rect &r, MidiData &midi, color col)
{
	c->setLineWidth(3.0f);
	foreach(MidiNote &n, midi){
		c->setColor(GetPitchColor(n.pitch));
		float x1 = sample2screen(n.range.offset);
		float x2 = sample2screen(n.range.end());
		float h = r.y2 - clampf((float)n.pitch / 80.0f - 0.3f, 0, 1) * r.height();
		//c->drawRect(rect(x1, x2, r.y1, r.y2));
		c->drawLine(x1, h, x2, h);
	}
	c->setLineWidth(LINE_WIDTH);
}

void draw_note(HuiPainter *c, const MidiNote &n, color &col, AudioView *v)
{
	float x1 = v->sample2screen(n.range.offset);
	float x2 = v->sample2screen(n.range.end());
	float xm = x1 * 0.8f + x2 * 0.2f;
	float y1 = v->pitch2y(n.pitch + 1);
	float y2 = v->pitch2y(n.pitch);
	c->setColor(col);
	c->drawRect(rect(x1, xm, y1, y2));
	color col2 = ColorInterpolate(col, Black, 0.1f);
	c->setColor(col2);
	c->drawRect(rect(xm, x2, y1, y2));
}

void AudioView::DrawMidiEditable(HuiPainter *c, const rect &r, MidiData &midi, Track *t, color col)
{
	foreachi(MidiNote &n, midi, i){
		if ((n.pitch < pitch_min) || (n.pitch >= pitch_max))
			continue;
		color col = GetPitchColor(n.pitch);
		if ((hover.type == SEL_TYPE_MIDI_NOTE) && (hover.note == i))
			col.a = 0.5f;
		draw_note(c, n, col, this);
	}
	if ((HuiGetEvent()->lbut) && (selection.type == SEL_TYPE_MIDI_PITCH)){
		Array<MidiNote> notes = GetSelectedNotes();
		foreach(MidiNote &n, notes){
			color col = GetPitchColor(n.pitch);
			col.a = 0.5f;
			draw_note(c, n, col, this);
		}
	}

	color cc = ColorWaveCur;
	cc.a = 0.4f;
	Array<SampleRef*> *p = NULL;
	if ((t->synth) && (t->synth->name == "Sample")){
		PluginData *c = t->synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	for (int i=pitch_min; i<pitch_max; i++){
		c->setColor(cc);
		if (((hover.type == SEL_TYPE_MIDI_PITCH) || (hover.type == SEL_TYPE_MIDI_NOTE)) && (i == hover.pitch))
			c->setColor(ColorWaveCur);

		string name = pitch_name(i);
		if (p)
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		c->drawStr(20, r.y1 + r.height() * (pitch_max - i - 1) / (pitch_max - pitch_min), name);
	}
}

void AudioView::DrawTrack(HuiPainter *c, const rect &r, Track *t, color col, int track_no)
{
	msg_db_f("DrawTrack", 1);

	if ((cur_track == t) && (EditingMidi()))
		DrawMidiEditable(c, r, t->midi, t, col);
	else
		DrawMidi(c, r, t->midi, col);

	DrawTrackBuffers(c, r, t, view_pos, col);

	foreach(SampleRef *s, t->sample)
		DrawSample(c, r, s);

	//c->setColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
//	c->setColor(ColorWaveCur);
	c->setFont("", -1, (t == cur_track), false);
	DrawStrBg(c, r.x1 + 23, r.y1 + 3, t->GetNiceName(), ColorWaveCur, ColorBackgroundCurWave);
	c->setFont("", -1, false, false);

	if (t->type == t->TYPE_TIME)
		c->drawImage(r.x1 + 5, r.y1 + 5, image_track_time);
	else if (t->type == t->TYPE_MIDI)
		c->drawImage(r.x1 + 5, r.y1 + 5, image_track_midi);
	else
		c->drawImage(r.x1 + 5, r.y1 + 5, image_track_audio);

	if ((t->muted) || (hover.show_track_controls == t))
		c->drawImage(r.x1 + 5, r.y1 + 22, t->muted ? image_muted : image_unmuted);
	if ((audio->track.num > 1) && (hover.show_track_controls == t))
		c->drawImage(r.x1 + 22, r.y1 + 22, image_solo);
}

void AudioView::DrawGridTime(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	double dl = MIN_GRID_DIST / view_zoom; // >= 10 pixel
	double dt = dl / audio->sample_rate;
	double exp_s = ceil(log10(dt));
	double exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
	dl = dt * audio->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dl);
	int nx1 = ceil(screen2sample(r.x2) / dl);
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->setColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dl);
		c->drawLine(xx, r.y1, xx, r.y2);
	}
	if (show_time){
		if ((output->IsPlaying()) && (output->GetSource() == renderer)){
			color cc = ColorPreviewMarker;
			cc.a = 0.25f;
			c->setColor(cc);
			float x0 = sample2screen(renderer->range.start());
			float x1 = sample2screen(renderer->range.end());
			c->drawRect(x0, r.y1, x1 - x0, r.y1 + TIME_SCALE_HEIGHT);
		}
		c->setColor(ColorGrid);
		for (int n=nx0;n<nx1;n++){
			if ((sample2screen(dl) - sample2screen(0)) > 30){
				if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
					c->drawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy(n * dl, dt * 3));
			}else{
				if ((n % 10) == 0)
					c->drawStr(sample2screen(n * dl) + 2, r.y1, audio->get_time_str_fuzzy(n * dl, dt * 10));
			}
		}
	}
}

bool AudioView::EditingMidi()
{
	if (!cur_track)
		return false;
	if (cur_track->type != Track::TYPE_MIDI)
		return false;
	return win->bottom_bar->IsActive(BottomBar::MIDI_EDITOR);
}

void AudioView::DrawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	Track *t = audio->GetTimeTrack();
	if (!t)
		return;
	bool editing_midi = EditingMidi();
	int s0 = screen2sample(r.x1 - 1);
	int s1 = screen2sample(r.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash, no_dash;
	dash.add(6);
	dash.add(4);
	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar> bars = t->bar.GetBars(Range(s0, s1 - s0));
	foreach(Bar &b, bars){
		int xx = sample2screen(b.range.offset);

		float dx_bar = dsample2screen(b.range.num);
		float dx_beat = dx_bar / b.num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b.index % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			c->setColor(ColorInterpolate(bg, ColorWave, f1));
			c->setLineDash(no_dash, r.y1);
			c->drawLine(xx, r.y1, xx, r.y2);
		}

		if (f2 >= 0.1f){
			color c1 = ColorInterpolate(bg, ColorWave, f2);
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

		if ((show_time) && (f1 > 0.9f)){
			c->setColor(ColorWave);
			c->drawStr(xx + 2, r.y1, i2s(b.index + 1));
		}
	}
	c->setLineDash(no_dash, 0);
	c->setLineWidth(LINE_WIDTH);
}

void AudioView::CheckConsistency()
{
	// check cur_track consistency
	int n = get_track_index_save(cur_track);
	if ((cur_track) && (n < 0))
		if (audio->track.num > 0)
			SetCurTrack(audio->track[0]);

	// check cur_level consistency
	if ((cur_level < 0) || (cur_level >= audio->level_name.num)){
		cur_level = 0;
		ForceRedraw();
	}
}

void AudioView::OnUpdate(Observable *o, const string &message)
{
	CheckConsistency();

	if (o == audio){
		if (message == audio->MESSAGE_NEW){
			sel_range = sel_raw = Range(0, 0);
			SetCurTrack(NULL);
			if (audio->track.num > 0)
				SetCurTrack(audio->track[0]);
			OptimizeView();
		}else{
			ForceRedraw();
			UpdateMenu();
		}

		if (message == audio->MESSAGE_CHANGE)
			audio->UpdatePeaks(peak_mode);
	}else if (o == output){
		if ((output->IsPlaying()) && (output->GetSource() == renderer))
			MakeSampleVisible(output->GetPos());
		ForceRedraw();
	}else if (o == input){
		if (input->IsCapturing())
			MakeSampleVisible(sel_range.start() + input->GetSampleCount());
		ForceRedraw();
	}else{
		ForceRedraw();
	}
}

void plan_track_sizes(const rect &r, AudioFile *a, AudioView *v)
{
	if (v->EditingMidi()){
		float y0 = v->TIME_SCALE_HEIGHT;
		foreachi(Track *t, a->track, i){
			float h = v->TIME_SCALE_HEIGHT;
			if (t == v->cur_track)
				h = r.height() - a->track.num * v->TIME_SCALE_HEIGHT;
			t->area = rect(r.x1, r.x2, y0, y0 + h);
			y0 += h;
		}
		return;
	}
	int n_ch = v->show_mono ? 1 : 2;

	int h_wish = v->TIME_SCALE_HEIGHT;
	int h_fix = v->TIME_SCALE_HEIGHT;
	int n_var = 0;
	foreach(Track *t, a->track){
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
	foreachi(Track *t, a->track, i){
		float h = v->TIME_SCALE_HEIGHT*2;
		if (t->type == t->TYPE_AUDIO)
			h = opt_channel_height * n_ch;
		else if (t->type == t->TYPE_MIDI)
			h = opt_channel_height;
		t->area = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

void AudioView::DrawTimeLine(HuiPainter *c, int pos, int type, color &col, bool show_time)
{
	int p = sample2screen(pos);
	if ((p >= audio->area.x1) && (p <= audio->area.x2)){
		c->setColor((type == hover.type) ? ColorSelectionBoundaryMO : col);
		c->drawLine(p, audio->area.y1, p, audio->area.y2);
		if (show_time)
			c->drawStr(p, (audio->area.y1 + audio->area.y2) / 2, audio->get_time_str_long(pos));
	}
}

void AudioView::DrawEmptyAudioFile(HuiPainter *c, const rect &r)
{
	color col = ColorBackgroundCurWave;
	c->setColor(col);
	c->drawRect(r.x1, r.y2, r.width(), r.height());
	c->setColor(ColorWaveCur);
	c->setFontSize(FONT_SIZE_NO_FILE);
	c->drawStr(r.x1 + r.width() / 2 - 50, r.y1 + r.height() / 2 - 10, _("keine Datei"));
}

void AudioView::DrawBackground(HuiPainter *c, const rect &r)
{
	int yy = 0;
	if (audio->track.num > 0)
		yy = audio->track.back()->area.y2;

	// time scale
	c->setColor(ColorBackgroundCurWave);
	c->drawRect(r.x1, r.y1, r.width(), TIME_SCALE_HEIGHT);
	DrawGridTime(c, rect(r.x1, r.x2, r.y1, r.y1 + TIME_SCALE_HEIGHT), ColorBackgroundCurWave, true);

	// tracks
	foreach(Track *t, audio->track){
		color cc = (t->is_selected) ? ColorBackgroundCurTrack : ColorBackgroundCurWave;
		c->setColor(cc);
		c->drawRect(t->area);

		if (t->type == t->TYPE_TIME){
			DrawGridBars(c, t->area, cc, true);
		}else{
			DrawGridTime(c, t->area, cc, false);
			DrawGridBars(c, t->area, cc, false);
		}

		if ((t == cur_track) && (EditingMidi())){
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
		c->setColor(ColorBackground);
		rect rr = rect(r.x1, r.x2, yy, r.y2);
		c->drawRect(rr);
		DrawGridTime(c, rr, ColorBackground, false);
	}

	// lines between tracks
	c->setColor(ColorGrid);
	foreach(Track *t, audio->track)
		c->drawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->drawLine(0, yy, r.width(), yy);

	//DrawGrid(c, r, ColorBackgroundCurWave, true);
}

void AudioView::DrawSelection(HuiPainter *c, const rect &r)
{
	int sx1 = sample2screen(sel_range.start());
	int sx2 = sample2screen(sel_range.end());
	int sxx1 = clampi(sx1, r.x1, r.x2);
	int sxx2 = clampi(sx2, r.x1, r.x2);
	c->setColor(ColorSelectionInternal);
	foreach(Track *t, audio->track)
		if (t->is_selected)
			c->drawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
	DrawTimeLine(c, sel_raw.start(), SEL_TYPE_SELECTION_START, ColorSelectionBoundary);
	DrawTimeLine(c, sel_raw.end(), SEL_TYPE_SELECTION_END, ColorSelectionBoundary);
}

void AudioView::DrawAudioFile(HuiPainter *c, const rect &r)
{
	audio->area = r;

	// empty file
	if (!audio->used){
		DrawEmptyAudioFile(c, r);
		return;
	}

	plan_track_sizes(r, audio, this);
	UpdateBufferZoom();

	// background
	DrawBackground(c, r);


	c->setColor(ColorWaveCur);

	// tracks
	foreachi(Track *tt, audio->track, i)
		DrawTrack(c, tt->area, tt, ColorWaveCur, i);


	// selection
	DrawSelection(c, r);

	// playing position
	if ((output->IsPlaying()) && (output->GetSource() == renderer)){
		if (!input->IsCapturing())
			DrawTimeLine(c, output->GetPos(), SEL_TYPE_PLAYBACK, ColorPreviewMarker, true);
	}

	// capturing position
	if (input->IsCapturing())
		DrawTimeLine(c, sel_range.start() + input->GetSampleCount(), SEL_TYPE_PLAYBACK, ColorCaptureMarker, true);
}

int frame=0;

void AudioView::OnDraw()
{
	msg_db_f("OnDraw", 1);
	force_redraw = false;

	HuiPainter *c = win->BeginDraw("area");
	drawing_rect = rect(0, c->width, 0, c->height);
	c->setFontSize(FONT_SIZE);
	c->setLineWidth(LINE_WIDTH);
	c->setAntialiasing(antialiasing);
	//c->setColor(ColorWaveCur);

	DrawAudioFile(c, rect(0, c->width, 0, c->height));

	//c->DrawStr(100, 100, i2s(frame++));

	if (!win->bottom_bar->visible){
		c->setColor((hover.type == SEL_TYPE_BOTTOM_BUTTON) ? Black : ColorWave);
		bottom_button_rect = rect(5, 5 + c->getStrWidth(_("Leiste anzeigen")), c->height - 18, c->height);
		c->drawStr(5, c->height - 18, _("Leiste anzeigen"));
	}else
		bottom_button_rect = rect(0, 0, 0, 0);

	c->end();
}

void AudioView::OptimizeView()
{
	msg_db_f("OptimizeView", 1);
	if (audio->area.x2 <= 0)
		audio->area.x2 = drawing_rect.x2;

	Range r = audio->GetRange();

	if (r.num == 0)
		r.num = 10 * audio->sample_rate;
	int border = r.num * BORDER_FACTOR;
	r.offset -= border;
	r.num += border * 2;
	view_zoom = audio->area.width() / (double)r.length();
	view_pos = (double)r.start();
	Notify(MESSAGE_VIEW_CHANGE);
	ForceRedraw();
}

void AudioView::UpdateMenu()
{
	// view
	win->Check("view_mono", show_mono);
	win->Check("view_stereo", !show_mono);
	win->Check("view_peaks_max", peak_mode == BufferBox::PEAK_MODE_MAXIMUM);
	win->Check("view_peaks_mean", peak_mode == BufferBox::PEAK_MODE_SQUAREMEAN);
	win->Enable("zoom_in", audio->used);
	win->Enable("zoom_out", audio->used);
	win->Enable("view_optimal", audio->used);
	win->Enable("view_samples", false);//win->cur_audio->used);
}

void AudioView::SetPeaksMode(int mode)
{
	peak_mode = mode;
	if (audio->used){
		audio->InvalidateAllPeaks();
		audio->UpdatePeaks(peak_mode);
	}
	ForceRedraw();
	UpdateMenu();
}

void AudioView::SetShowMono(bool mono)
{
	show_mono = mono;
	ForceRedraw();
	Notify(MESSAGE_SETTINGS_CHANGE);
	UpdateMenu();
}

void AudioView::ZoomIn()
{
	Zoom(2.0f);
}

void AudioView::ZoomOut()
{
	Zoom(0.5f);
}
void AudioView::MakeSampleVisible(int sample)
{
	double x = sample2screen(sample);
	if ((x > audio->area.x2) || (x < audio->area.x1)){
		view_pos = sample - audio->area.width() / view_zoom * BORDER_FACTOR;
		Notify(MESSAGE_VIEW_CHANGE);
		ForceRedraw();
	}
}

void AudioView::SelectAll()
{
	sel_raw = audio->GetRange();
	UpdateSelection();
}

void AudioView::SelectNone()
{
	// select all/none
	sel_raw.clear();
	UpdateSelection();
	audio->UnselectAllSamples();
	SetCurSample(NULL);
}



void AudioView::SelectSample(SampleRef *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		s->is_selected = !s->is_selected;
	}else{
		if (!s->is_selected)
			s->owner->UnselectAllSamples();

		// select this sub
		s->is_selected = true;
	}
}

void AudioView::SelectTrack(Track *t, bool diff)
{
	if (!t)
		return;
	if (diff){
		bool is_only_selected = true;
		foreach(Track *tt, t->root->track)
			if ((tt->is_selected) && (tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected || is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(Track *tt, t->root->track)
				tt->is_selected = false;
		}

		// select this track
		t->is_selected = true;
	}
	UpdateSelection();
}

void AudioView::SetCurSample(SampleRef *s)
{
	if (cur_sample == s)
		return;
	cur_sample = s;
	ForceRedraw();
	Notify(MESSAGE_CUR_SAMPLE_CHANGE);
}


void AudioView::SetCurTrack(Track *t)
{
	if (cur_track == t)
		return;
	cur_track = t;
	ForceRedraw();
	Notify(MESSAGE_CUR_TRACK_CHANGE);
}

void AudioView::SetCurLevel(int l)
{
	if (cur_level == l)
		return;
	if ((l < 0) || (l >= audio->level_name.num))
		return;
	cur_level = l;
	ForceRedraw();
	Notify(MESSAGE_CUR_LEVEL_CHANGE);
}



double AudioView::screen2sample(double _x)
{
	return (_x - audio->area.x1) / view_zoom + view_pos;
}

double AudioView::sample2screen(double s)
{
	return audio->area.x1 + (s - view_pos) * view_zoom;
}

double AudioView::dsample2screen(double ds)
{
	return ds * view_zoom;
}

void AudioView::Zoom(float f)
{
	// max zoom: 20 pixel per sample
	double zoom_new = clampf(view_zoom * f, 0.000001, 20.0);

	view_pos += (mx - audio->area.x1) * (1.0/view_zoom - 1.0/zoom_new);
	view_zoom = zoom_new;
	Notify(MESSAGE_VIEW_CHANGE);
	ForceRedraw();
}

void AudioView::Move(float dpos)
{
	view_pos += dpos;
	Notify(MESSAGE_VIEW_CHANGE);
	ForceRedraw();
}




