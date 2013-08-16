/*
 * MidiEditor.cpp
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#include "MidiEditor.h"
#include "../../Data/AudioFile.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/Synth/DummySynthesizer.h"
#include "../../Tsunami.h"
#include "../AudioView.h"
#include <math.h>

MidiEditor::MidiEditor(HuiWindow* _parent, bool _allow_parent, AudioFile *a, Track *t) :
	HuiDialog(_("Midi-Pattern Manager"), 600, 600, _parent, _allow_parent)
{
	audio = a;
	track = t;

	AddControlTable("", 0, 0, 1, 2, "table1");
	SetTarget("table1", 0);
	AddControlTable("!noexpandy", 0, 0, 6, 1, "table2");
	AddDrawingArea("", 0, 1, 0, 0, "area");
	SetTarget("table2", 0);
	AddButton("", 2, 0, 0, 0, "play_pattern");
	SetImage("play_pattern", "hui:media-play");
	AddButton("", 3, 0, 0, 0, "stop_pattern");
	SetImage("stop_pattern", "hui:media-stop");
	AddSpinButton("4\\1", 4, 0, 0, 0, "beat_partition");

	//SetTooltip("insert_sample", _("f&ugt am Cursor der aktuellen Spur ein"));

	EventM("hui:close", this, &MidiEditor::OnClose);
	//EventM("import_from_file", this, &MidiEditor::OnImportFromFile);
	//EventM("insert_sample", this, &MidiEditor::OnInsert);
	//EventM("create_from_selection", this, &MidiEditor::OnCreateFromSelection);
	EventMX("area", "hui:redraw", this, &MidiEditor::OnAreaDraw);
	EventMX("area", "hui:mouse-move", this, &MidiEditor::OnAreaMouseMove);
	EventMX("area", "hui:left-button-down", this, &MidiEditor::OnAreaLeftButtonDown);
	EventMX("area", "hui:left-button-up", this, &MidiEditor::OnAreaLeftButtonUp);
	EventM("play_pattern", this, &MidiEditor::OnPlay);
	EventM("stop_pattern", this, &MidiEditor::OnStop);
	EventM("beat_partition", this, &MidiEditor::OnBeatPartition);

	if (a->selection.num == 0)
		a->selection.num = a->sample_rate * 4;

	beat_partition = 4;
	CreateParts();

	pitch_min = 60;
	pitch_max = 90;
	cur_part = -1;
	cur_sample = -1;
	creating_new_note = false;
	new_note = new MidiNote;
	new_part_start = -1;
	hover_note = -1;

	Subscribe(audio);
	Subscribe(tsunami->output);
}

MidiEditor::~MidiEditor()
{
	Unsubscribe(tsunami->output);
	Unsubscribe(audio);
}

void MidiEditor::CreateParts()
{
	bars.clear();
	parts.clear();
	if (!audio->GetTimeTrack())
		return;
	bars = audio->GetTimeTrack()->bar.GetBars(audio->selection);

	foreach(Bar &b, bars)
		for (int i=0; i<b.num_beats; i++){
			for (int j=0; j<beat_partition; j++){
				int s0 = b.range.offset + (i * beat_partition + j) * b.range.num / beat_partition / b.num_beats;
				int s1 = b.range.offset + (i * beat_partition + j + 1) * b.range.num / beat_partition / b.num_beats;
				parts.add(Range(s0, s1 - s0));
			}
		}

	Redraw("area");
}


int MidiEditor::x2sample(int x)
{
	return audio->selection.offset + x * audio->selection.num / area_width;
	//return (x * cur_pattern->num_beats * cur_pattern->beat_partition / area_width);
}

int MidiEditor::y2pitch(int y)
{
	return pitch_min + ((area_height - y) * (pitch_max - pitch_min) / area_height);
}

float MidiEditor::sample2x(int s)
{
	return area_width * (s - audio->selection.offset) / audio->selection.num;
	//return area_width * (float)b / (cur_pattern->beat_partition * cur_pattern->num_beats);
}

float MidiEditor::pitch2y(int p)
{
	return area_height - area_height * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void MidiEditor::OnAreaMouseMove()
{
	int x = HuiGetEvent()->mx;
	int y = HuiGetEvent()->my;
	cur_sample = x2sample(x);
	cur_pitch = y2pitch(y);
	cur_part = -1;
	foreachi(Range &r, parts, i)
		if (r.is_inside(cur_sample))
			cur_part = i;

	hover_note = -1;

	if (creating_new_note){
		Range a = parts[new_part_start];
		Range b = parts[cur_part];
		if (cur_part >= new_part_start)
			new_note->range = Range(a.start(), b.end() - a.start());
		else
			new_note->range = Range(b.start(), a.end() - b.start());
	}else{
		// hovering over a note?
		foreachi(MidiNote &n, track->midi, i)
			if ((n.pitch == cur_pitch) && (n.range.is_inside(cur_sample)))
				hover_note = i;
	}
	Redraw("area");
}

void MidiEditor::OnAreaLeftButtonDown()
{
	OnAreaMouseMove();

	// clicked on a note?
	if (hover_note >= 0){
		track->midi.erase(hover_note);
		hover_note = -1;
		return;
	}
	creating_new_note = true;
	new_note->range = parts[cur_part];
	new_note->volume = 1;
	new_note->pitch = cur_pitch;
	new_part_start = cur_part;
}

void MidiEditor::OnAreaLeftButtonUp()
{
	if (creating_new_note){
		track->midi.add(*new_note);
	}
	creating_new_note = false;
}

bool is_sharp(int pitch)
{
	int r = pitch % 12;
	// 69 = 9 = a
	return ((r == 10) || (r == 1) || (r == 3) || (r == 6) || (r == 8));
}

void MidiEditor::DrawNote(HuiPainter *c, MidiNote &n, bool hover)
{
	color col = tsunami->view->GetPitchColor(n.pitch);
	if (hover)
		col = ColorInterpolate(col, White, 0.5f);
	c->SetColor(col);
	c->DrawRect(rect(sample2x(n.range.start()), sample2x(n.range.end()), pitch2y(n.pitch + 1), pitch2y(n.pitch)));
}

void MidiEditor::OnAreaDraw()
{
	HuiPainter *c = BeginDraw("");
	c->SetLineWidth(0.8f);
	int w = c->width;
	int h = c->height;
	area_width = w;
	area_height = h;

	c->SetColor(White);
	c->DrawRect(0, 0, w, h);
	/*if (!cur_pattern){
		c->SetColor(Gray);
		c->DrawStr(100, h / 2, _("kein Muster ausgew&ahlt"));
		c->End();
		return;
	}*/

	// pitch grid
	c->SetColor(Gray);
	for (int i=pitch_min; i<pitch_max; i++){
		float y0 = pitch2y(i + 1);
		float y1 = pitch2y(i);
		if (is_sharp(i)){
			c->SetColor(color(0.2f, 0, 0, 0));
			c->DrawRect(0, y0, w, y1 - y0);
		}
	}


	// time grid
	foreach(Bar &b, bars)
		for (int i=0; i<b.num_beats; i++){
			for (int j=0; j<beat_partition; j++){
				if (j == 0)
					c->SetColor(Black);
				else
					c->SetColor(Gray);
				float x = sample2x(b.range.offset + (i * beat_partition + j) * b.range.num / beat_partition / b.num_beats);
				c->DrawLine(x, 0, x, h);
			}
		}

	foreachi(MidiNote &n, track->midi, i)
		if (n.range.overlaps(audio->selection))
			DrawNote(c, n, i == hover_note);

	if (creating_new_note){
		DrawNote(c, *new_note, true);
	}else if ((hover_note < 0) && (cur_part >= 0)){
		color col = tsunami->view->GetPitchColor(cur_pitch);
		col.a = 0.5f;
		c->SetColor(col);
		Range r = parts[cur_part];
		c->DrawRect(rect(sample2x(r.start()), sample2x(r.end()), pitch2y(cur_pitch + 1), pitch2y(cur_pitch)));
	}

	if (tsunami->output->IsPlaying()){
		int pos = tsunami->output->GetPos();
		int s = tsunami->renderer->TranslateOutputPos(pos);
		float x = sample2x(s);
		c->SetColor(Green);
		c->DrawLine(x, 0, x, h);
	}
}

/*int stream_func(BufferBox &b)
{
	for (int i=0;i<b.num;i++){
		b.r[i] = 0;
		b.l[i] = 0;
	}
	DummySynthesizer synth;
	MidiEditor *m = tsunami->midi_pattern_manager;
	MidiPattern *p = m->cur_pattern;
	int samples_per_beat = DEFAULT_SAMPLE_RATE * 60 / m->beats_per_minute;
	int samples_per_time = samples_per_beat / p->beat_partition;
	int length = samples_per_beat * p->num_beats;
	int offset = b.offset % length;
	foreach(MidiNote &n, p->notes){
		Range r = Range(n.range.offset * samples_per_time - offset, n.range.num * samples_per_time);
		if (r.end() < 0)
			r.offset += length;
		synth.RenderNote(b, r, n.pitch, n.volume);
	}
	return b.num;
}*/

void MidiEditor::OnPlay()
{
	/*if (cur_pattern){
		tsunami->output->PlayGenerated((void*)&stream_func, DEFAULT_SAMPLE_RATE);
		tsunami->output->SetBufferSize(16384);
	}*/
	tsunami->OnPlay();
}

void MidiEditor::OnStop()
{
	tsunami->output->Stop();
}

void MidiEditor::OnBeatPartition()
{
	beat_partition = GetInt("");
	CreateParts();
}

void MidiEditor::OnClose()
{
	delete(this);
}

void MidiEditor::OnUpdate(Observable* o)
{
	if (o == audio){
	}else if (o == tsunami->output){
		Redraw("area");
	}
}
