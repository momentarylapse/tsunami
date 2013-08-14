/*
 * MidiPatternManager.cpp
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#include "MidiPatternManager.h"
#include "../../Data/AudioFile.h"
#include "../../Tsunami.h"
#include <math.h>

MidiPatternManager::MidiPatternManager(AudioFile *a, HuiWindow* _parent, bool _allow_parent) :
	HuiDialog(_("Midi-Pattern Manager"), 600, 400, _parent, _allow_parent)
{
	AddControlTable("", 0, 0, 1, 3, "table1");
	SetTarget("table1", 0);
	AddListView(_("!format=iTttt\\Vorschau\\Name\\Schl&age\\Unterteilungen\\Benutzung"), 0, 0, 0, 0, "pattern_list");
	AddControlTable("!noexpandy", 0, 1, 4, 1, "table2");
	AddDrawingArea("", 0, 2, 0, 0, "area");
	SetTarget("table2", 0);
	AddButton(_("Neu"), 0, 0, 0, 0, "add_pattern");
	AddButton(_("L&oschen"), 1, 0, 0, 0, "delete_pattern");
	//AddButton(_("Einf&ugen"), 2, 0, 0, 0, "insert_sample");
	//AddButton(_("Aus Auswahl"), 3, 0, 0, 0, "create_from_selection");
	AddButton(_("Abspielen"), 2, 0, 0, 0, "play_pattern");

	//SetTooltip("insert_sample", _("f&ugt am Cursor der aktuellen Spur ein"));

	EventM("hui:close", this, &MidiPatternManager::OnClose);
	//EventM("import_from_file", this, &MidiPatternManager::OnImportFromFile);
	//EventM("insert_sample", this, &MidiPatternManager::OnInsert);
	//EventM("create_from_selection", this, &MidiPatternManager::OnCreateFromSelection);
	EventM("add_pattern", this, &MidiPatternManager::OnAdd);
	EventM("delete_pattern", this, &MidiPatternManager::OnDelete);
	EventMX("pattern_list", "hui:select", this, &MidiPatternManager::OnListSelect);
	EventMX("area", "hui:redraw", this, &MidiPatternManager::OnAreaDraw);
	EventMX("area", "hui:mouse-move", this, &MidiPatternManager::OnAreaMouseMove);
	EventMX("area", "hui:left-button-down", this, &MidiPatternManager::OnAreaClick);


	pitch_min = 60;
	pitch_max = 90;
	audio = a;
	cur_pattern = NULL;
	FillList();

	Subscribe(audio);
}

MidiPatternManager::~MidiPatternManager()
{
	Unsubscribe(audio);
}

void MidiPatternManager::FillList()
{
	Reset("pattern_list");
	foreach(string &name, icon_names)
		HuiDeleteImage(name);
	icon_names.clear();
	foreachi(MidiPattern *p, audio->midi_pattern, i){
		icon_names.add("");//render_bufbox(p->buf, 80, 32));
		SetString("pattern_list", icon_names[i] + "\\" + p->name + "\\" + format("%d\\%d\\%d", p->num_beats, p->beat_partition, p->ref_count));
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


int MidiPatternManager::x2beat(int x)
{
	return (x * cur_pattern->num_beats * cur_pattern->beat_partition / area_width);
}

int MidiPatternManager::y2pitch(int y)
{
	return pitch_min + (y * (pitch_max - pitch_min) / area_height);
}

float MidiPatternManager::beat2x(int b)
{
	return area_width * (float)b / (cur_pattern->beat_partition * cur_pattern->num_beats);
}

float MidiPatternManager::pitch2y(int p)
{
	return area_height * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void MidiPatternManager::OnAreaMouseMove()
{
	int x = HuiGetEvent()->mx;
	int y = HuiGetEvent()->my;
	if (cur_pattern){
		cur_time = x2beat(x);
		cur_pitch = y2pitch(y);
	}
	Redraw("area");
}

void MidiPatternManager::OnAreaClick()
{
}

bool is_sharp(int pitch)
{
	int r = pitch % 12;
	// 69 = 9 = a
	return ((r == 10) || (r == 1) || (r == 3) || (r == 6) || (r == 8));
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
		float y0 = pitch2y(i);
		float y1 = pitch2y(i + 1);
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
		float x = beat2x(i);
		c->DrawLine(x, 0, x, h);
	}

	c->SetColor(color(0.5f, 0, 0, 1));
	c->DrawRect(rect(beat2x(cur_time), beat2x(cur_time + 1), pitch2y(cur_pitch), pitch2y(cur_pitch + 1)));
}

void MidiPatternManager::OnAdd()
{
	SetCurPattern(audio->AddMidiPattern("pattern", 4, 4));
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
	Redraw("area");
}

void MidiPatternManager::OnClose()
{
	Hide();
}

void MidiPatternManager::OnUpdate(Observable* o)
{
	FillList();
}
