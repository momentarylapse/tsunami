/*
 * MidiPatternManager.cpp
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#include "MidiPatternManager.h"
#include "../../Data/AudioFile.h"
#include "../../Audio/AudioOutput.h"
#include "../../Audio/Synth/DummySynthesizer.h"
#include "../../Tsunami.h"
#include "../AudioView.h"
#include <math.h>

MidiPatternManager::MidiPatternManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent) :
	HuiDialog(_("Midi-Pattern Manager"), 600, 600, _parent, _allow_parent)
{
	AddControlTable("", 0, 0, 1, 3, "table1");
	SetTarget("table1", 0);
	AddListView(_("!format=iTTTt,height=150\\Vorschau\\Name\\Schl&age\\Unterteilungen\\Benutzung"), 0, 0, 0, 0, "pattern_list");
	AddControlTable("!noexpandy", 0, 1, 5, 1, "table2");
	AddDrawingArea("", 0, 2, 0, 0, "area");
	SetTarget("table2", 0);
	AddButton(_("Neu"), 0, 0, 0, 0, "add_pattern");
	AddButton(_("L&oschen"), 1, 0, 0, 0, "delete_pattern");
	//AddButton(_("Einf&ugen"), 2, 0, 0, 0, "insert_sample");
	//AddButton(_("Aus Auswahl"), 3, 0, 0, 0, "create_from_selection");
	AddButton("", 2, 0, 0, 0, "play_pattern");
	SetImage("play_pattern", "hui:media-play");
	AddButton("", 3, 0, 0, 0, "stop_pattern");
	SetImage("stop_pattern", "hui:media-stop");
	AddSpinButton("90\\0\\1000\\0.1", 4, 0, 0, 0, "beats_per_minute");
	//AddSpinButton("4\\1\\100", 5, 0, 0, 0, "num_beats");
	//AddSpinButton("4\\1\\200", 6, 0, 0, 0, "partition");

	//SetTooltip("insert_sample", _("f&ugt am Cursor der aktuellen Spur ein"));

	EventM("hui:close", this, &MidiPatternManager::OnClose);
	//EventM("import_from_file", this, &MidiPatternManager::OnImportFromFile);
	//EventM("insert_sample", this, &MidiPatternManager::OnInsert);
	//EventM("create_from_selection", this, &MidiPatternManager::OnCreateFromSelection);
	EventM("add_pattern", this, &MidiPatternManager::OnAdd);
	EventM("delete_pattern", this, &MidiPatternManager::OnDelete);
	EventMX("pattern_list", "hui:select", this, &MidiPatternManager::OnListSelect);
	EventMX("pattern_list", "hui:change", this, &MidiPatternManager::OnListEdit);
	EventMX("area", "hui:redraw", this, &MidiPatternManager::OnAreaDraw);
	EventMX("area", "hui:mouse-move", this, &MidiPatternManager::OnAreaMouseMove);
	EventMX("area", "hui:left-button-down", this, &MidiPatternManager::OnAreaLeftButtonDown);
	EventMX("area", "hui:left-button-up", this, &MidiPatternManager::OnAreaLeftButtonUp);
	EventM("play_pattern", this, &MidiPatternManager::OnPlay);
	EventM("beats_per_minute", this, &MidiPatternManager::OnBeatsPerMinute);


	pitch_min = 60;
	pitch_max = 90;
	beats_per_minute = 90;
	audio = a;
	creating_new_note = false;
	new_note = new MidiNote;
	new_time_start = -1;
	cur_pattern = NULL;
	FillList();

	Subscribe(audio);
	Subscribe(tsunami->output);
}

MidiPatternManager::~MidiPatternManager()
{
	Unsubscribe(tsunami->output);
	Unsubscribe(audio);
}

void MidiPatternManager::OnListEdit()
{
	int n = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (n < 0)
		return;
	MidiPattern *p = audio->midi_pattern[n];
	if (col == 1)
		p->name = GetCell("pattern_list", n, col);
	else if (col == 2)
		p->num_beats = GetCell("pattern_list", n, col)._int();
	else if (col == 3)
		p->beat_partition = GetCell("pattern_list", n, col)._int();
	Redraw("area");
}

void MidiPatternManager::FillList()
{
	Reset("pattern_list");
	foreach(string &name, icon_names)
		HuiDeleteImage(name);
	icon_names.clear();
	if ((audio->midi_pattern.num > 0) && (!cur_pattern))
		SetCurPattern(audio->midi_pattern[0]);
	foreachi(MidiPattern *p, audio->midi_pattern, i){
		icon_names.add("");//render_bufbox(p->buf, 80, 32));
		SetString("pattern_list", icon_names[i] + "\\" + p->name + "\\" + format("%d\\%d\\%d", p->num_beats, p->beat_partition, p->ref_count));
		if (p == cur_pattern)
			SetInt("pattern_list", i);
	}
	Enable("delete_pattern", false);
	//Enable("insert_pattern", false);
}

void MidiPatternManager::OnListSelect()
{
	int n = GetInt("");
	Enable("delete_pattern", n >= 0);
	//Enable("insert_pattern", n >= 0);
	SetCurPattern((n >= 0) ? audio->midi_pattern[n] : NULL);
}


int MidiPatternManager::x2time(int x)
{
	return (x * cur_pattern->num_beats * cur_pattern->beat_partition / area_width);
}

int MidiPatternManager::y2pitch(int y)
{
	return pitch_min + ((area_height - y) * (pitch_max - pitch_min) / area_height);
}

float MidiPatternManager::time2x(int b)
{
	return area_width * (float)b / (cur_pattern->beat_partition * cur_pattern->num_beats);
}

float MidiPatternManager::pitch2y(int p)
{
	return area_height - area_height * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void MidiPatternManager::OnAreaMouseMove()
{
	int x = HuiGetEvent()->mx;
	int y = HuiGetEvent()->my;
	if (cur_pattern){
		cur_time = x2time(x);
		cur_pitch = y2pitch(y);

		hover_note = -1;

		if (creating_new_note){
			if (cur_time >= new_time_start)
				new_note->range = Range(new_time_start, cur_time - new_time_start + 1);
			else
				new_note->range = Range(cur_time, new_time_start - cur_time + 1);
		}else{
			// hovering over a note?
			foreachi(MidiNote &n, cur_pattern->notes, i)
				if ((n.pitch == cur_pitch) && (n.range.is_inside(cur_time)))
					hover_note = i;
		}
	}
	Redraw("area");
}

void MidiPatternManager::OnAreaLeftButtonDown()
{
	if (!cur_pattern)
		return;
	OnAreaMouseMove();

	// clicked on a note?
	if (hover_note >= 0){
		cur_pattern->notes.erase(hover_note);
		return;
	}
	creating_new_note = true;
	new_note->range = Range(cur_time, 1);
	new_note->volume = 1;
	new_note->pitch = cur_pitch;
	new_time_start = cur_time;
}

void MidiPatternManager::OnAreaLeftButtonUp()
{
	if ((cur_pattern) && (creating_new_note)){
		cur_pattern->notes.add(*new_note);
	}
	creating_new_note = false;
}

bool is_sharp(int pitch)
{
	int r = pitch % 12;
	// 69 = 9 = a
	return ((r == 10) || (r == 1) || (r == 3) || (r == 6) || (r == 8));
}

void MidiPatternManager::DrawNote(HuiPainter *c, MidiNote &n, bool hover)
{
	color col = tsunami->view->GetPitchColor(n.pitch);
	if (hover)
		col = ColorInterpolate(col, White, 0.5f);
	c->SetColor(col);
	c->DrawRect(rect(time2x(n.range.start()), time2x(n.range.end()), pitch2y(n.pitch + 1), pitch2y(n.pitch)));
}

void MidiPatternManager::OnAreaDraw()
{
	HuiPainter *c = BeginDraw("");
	c->SetLineWidth(0.8f);
	int w = c->width;
	int h = c->height;
	area_width = w;
	area_height = h;

	c->SetColor(White);
	c->DrawRect(0, 0, w, h);
	if (!cur_pattern){
		c->SetColor(Gray);
		c->DrawStr(100, h / 2, _("kein Muster ausgew&ahlt"));
		c->End();
		return;
	}

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
	for (int i=0; i<cur_pattern->num_beats * cur_pattern->beat_partition; i++){
		if ((i % cur_pattern->beat_partition) == 0)
			c->SetColor(Black);
		else
			c->SetColor(Gray);
		float x = time2x(i);
		c->DrawLine(x, 0, x, h);
	}

	foreachi(MidiNote &n, cur_pattern->notes, i)
		DrawNote(c, n, i == hover_note);

	if (creating_new_note){
		DrawNote(c, *new_note, true);
	}else if (hover_note < 0){
		color col = tsunami->view->GetPitchColor(cur_pitch);
		col.a = 0.5f;
		c->SetColor(col);
		c->DrawRect(rect(time2x(cur_time), time2x(cur_time + 1), pitch2y(cur_pitch + 1), pitch2y(cur_pitch)));
	}

	if (tsunami->output->IsPlaying()){
		int pos = tsunami->output->GetPos(); // FIXME!!!!!!!!!!
		int samples_per_beat = DEFAULT_SAMPLE_RATE * 60 / beats_per_minute;
		int length = samples_per_beat * cur_pattern->num_beats;
		int offset = pos % length;
		float x = w * offset / length;
		c->SetColor(Green);
		c->DrawLine(x, 0, x, h);
	}
}

void MidiPatternManager::OnAdd()
{
	SetCurPattern(audio->AddMidiPattern(format(_("Pattern %d"), audio->midi_pattern.num + 1), 4, 4));
	SetInt("pattern_list", audio->midi_pattern.num - 1);
}

void MidiPatternManager::OnDelete()
{
	int n = GetInt("pattern_list");
	if (n >= 0)
		audio->DeleteMidiPattern(n);
	SetCurPattern(NULL);
}

void MidiPatternManager::SetCurPattern(MidiPattern *p)
{
	cur_pattern = p;
	if (!p)
		tsunami->output->Stop();
	Redraw("area");
}

void stream_func(BufferBox &b)
{
	for (int i=0;i<b.num;i++){
		b.r[i] = 0;
		b.l[i] = 0;
	}
	DummySynthesizer synth;
	MidiPatternManager *m = tsunami->midi_pattern_manager;
	MidiPattern *p = m->cur_pattern;
	int samples_per_beat = DEFAULT_SAMPLE_RATE * 60 / m->beats_per_minute;
	int samples_per_time = samples_per_beat / p->beat_partition;
	int length = samples_per_beat * p->num_beats;
	int offset = b.offset % length;
	foreach(MidiNote &n, p->notes){
		Range r = Range(n.range.offset * samples_per_time - offset, n.range.num * samples_per_time);
		if (r.end() < 0)
			r.offset += length;
		synth.AddTone(b, r, n.pitch, n.volume);
	}
}

void MidiPatternManager::OnPlay()
{
	if (cur_pattern){
		tsunami->output->PlayGenerated((void*)&stream_func, DEFAULT_SAMPLE_RATE);
		//tsunami->output->SetBufferSize(4096);
	}
}

void MidiPatternManager::OnStop()
{
	tsunami->output->Stop();
}

void MidiPatternManager::OnBeatsPerMinute()
{
	beats_per_minute = GetFloat("");
}

void MidiPatternManager::OnClose()
{
	tsunami->output->Stop();
	Hide();
}

void MidiPatternManager::OnUpdate(Observable* o)
{
	if (o == audio)
		FillList();
	else if (o == tsunami->output)
		Redraw("area");
}
