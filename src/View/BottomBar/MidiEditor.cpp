/*
 * MidiEditor.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiEditor.h"
#include "../../Data/Track.h"
#include "../../Data/MidiData.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"


class SingleMidiFxPanel : public HuiPanel, public Observer
{
public:
	SingleMidiFxPanel(AudioFile *a, Track *t, MidiEffect *_fx, int _index) :
		Observer("SingleMidiFxPanel")
	{
		audio = a;
		track = t;
		fx = _fx;
		index = _index;
		AddControlTable("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddControlTable("", 0, 0, 5, 1, "header");
		SetTarget("header", 0);
		AddButton("!flat", 0, 0, 0, 0, "load_favorite");
		SetImage("load_favorite", "hui:open");
		SetTooltip("load_favorite", _("Parameter laden"));
		AddButton("!flat", 1, 0, 0, 0, "save_favorite");
		SetImage("save_favorite", "hui:save");
		SetTooltip("save_favorite", _("Parameter speichern"));
		AddText("!bold,center,expandx\\" + fx->name, 2, 0, 0, 0, "");
		AddCheckBox("", 3, 0, 0, 0, "enabled");
		SetTooltip("enabled", _("aktiv?"));
		AddButton("!flat", 4, 0, 0, 0, "delete");
		SetImage("delete", "hui:delete");
		SetTooltip("delete", _("Effekt l&oschen"));
		p = fx->createPanel();
		if (p){
			Embed(p, "grid", 0, 1);
			p->update();
		}else{
			SetTarget("grid", 0);
			AddText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			HideControl("load_favorite", true);
			HideControl("save_favorite", true);
		}

		EventM("enabled", this, &SingleMidiFxPanel::onEnabled);
		EventM("delete", this, &SingleMidiFxPanel::onDelete);
		EventM("load_favorite", this, &SingleMidiFxPanel::onLoad);
		EventM("save_favorite", this, &SingleMidiFxPanel::onSave);

		Check("enabled", fx->enabled);

		old_param = fx->configToString();
		subscribe(fx, fx->MESSAGE_CHANGE);
		subscribe(fx, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleMidiFxPanel()
	{
		unsubscribe(fx);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, fx, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(fx, name);
		if (track)
			track->EditMidiEffect(index, old_param);
		old_param = fx->configToString();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, fx, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(fx, name);
	}
	void onEnabled()
	{
		if (track)
			track->EnableMidiEffect(index, IsChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->DeleteMidiEffect(index);
	}
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			if (track)
				track->EditMidiEffect(index, old_param);
		}
		Check("enabled", fx->enabled);
		p->update();
		old_param = fx->configToString();
	}
	AudioFile *audio;
	Track *track;
	MidiEffect *fx;
	ConfigPanel *p;
	string old_param;
	int index;
};

MidiEditor::MidiEditor(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Midi")),
	Observer("MidiEditor")
{
	view = _view;
	audio = _audio;

	FromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	Array<string> chord_types = GetChordTypeNames();
	foreach(string &ct, chord_types)
		AddString("chord_type", ct);
	SetInt("chord_type", 0);
	Enable("chord_type", false);
	AddString("chord_inversion", _("Grundform"));
	AddString("chord_inversion", _("1. Umkehrung"));
	AddString("chord_inversion", _("2. Umkehrung"));
	SetInt("chord_inversion", 0);
	Enable("chord_inversion", false);

	SetTooltip("add", _("neuen Effekt hinzuf&ugen"));

	for (int i=0; i<12; i++)
		AddString("scale", rel_pitch_name(i) + " " + GetChordTypeName(CHORD_TYPE_MAJOR) + " / " + rel_pitch_name(pitch_to_rel(i + 9)) + " " + GetChordTypeName(CHORD_TYPE_MINOR));
	SetInt("scale", view->midi_scale);

	track = NULL;
	//Enable("add", false);
	Enable("track_name", false);

	EventM("pitch_offset", this, &MidiEditor::onPitch);
	EventM("beat_partition", this, &MidiEditor::onBeatPartition);
	EventM("scale", this, &MidiEditor::onScale);
	EventM("midi_mode:select", this, &MidiEditor::onMidiModeSelect);
	EventM("midi_mode:note", this, &MidiEditor::onMidiModeNote);
	EventM("midi_mode:chord", this, &MidiEditor::onMidiModeChord);
	EventM("chord_type", this, &MidiEditor::onChordType);
	EventM("chord_inversion", this, &MidiEditor::onChordInversion);

	EventM("add", (HuiPanel*)this, &MidiEditor::onAdd);

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	update();
}

MidiEditor::~MidiEditor()
{
	clear();
	unsubscribe(view);
	unsubscribe(audio);
}

void MidiEditor::update()
{
	bool allow = false;
	if (view->cur_track)
		allow = (view->cur_track->type == Track::TYPE_MIDI);
	HideControl("me_grid_yes", !allow);
	HideControl("me_grid_no", allow);
	HideControl(id_inner, !allow);
}

void MidiEditor::onUpdate(Observable* o, const string &message)
{
	update();
	if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
		setTrack(view->cur_track);
	else
		setTrack(track);
}


void MidiEditor::onPitch()
{
	view->pitch_min = GetInt("");
	view->pitch_max = GetInt("") + 30;
	view->ForceRedraw();
}

void MidiEditor::onScale()
{
	view->midi_scale = GetInt("");
	view->ForceRedraw();
}

void MidiEditor::onBeatPartition()
{
	view->beat_partition = GetInt("");
	view->ForceRedraw();
}

void MidiEditor::onMidiModeSelect()
{
	view->midi_mode = view->MIDI_MODE_SELECT;
	Enable("chord_type", false);
	Enable("chord_inversion", false);
}

void MidiEditor::onMidiModeNote()
{
	view->midi_mode = view->MIDI_MODE_NOTE;
	Enable("chord_type", false);
	Enable("chord_inversion", false);
}

void MidiEditor::onMidiModeChord()
{
	view->midi_mode = view->MIDI_MODE_CHORD;
	Enable("chord_type", true);
	Enable("chord_inversion", true);
}

void MidiEditor::onChordType()
{
	view->chord_type = GetInt("");
}

void MidiEditor::onChordInversion()
{
	view->chord_inversion = GetInt("");
}


void MidiEditor::onAdd()
{
	MidiEffect *effect = tsunami->plugin_manager->ChooseMidiEffect(this);
	if (!effect)
		return;
	if (track)
		track->AddMidiEffect(effect);
}

void MidiEditor::clear()
{
	if (track)
		unsubscribe(track);
	foreachi(HuiPanel *p, panels, i){
		delete(p);
		RemoveControl("separator_" + i2s(i));
	}
	panels.clear();
	track = NULL;
	//Enable("add", false);
}

void MidiEditor::setTrack(Track *t)
{
	clear();
	track = t;
	if (track){
		subscribe(track, track->MESSAGE_DELETE);
		subscribe(track, track->MESSAGE_ADD_MIDI_EFFECT);
		subscribe(track, track->MESSAGE_DELETE_MIDI_EFFECT);
	}


	if (track){
		foreachi(MidiEffect *e, track->midi.fx, i){
			panels.add(new SingleMidiFxPanel(audio, track, e, i));
			Embed(panels.back(), id_inner, i*2 + 3, 0);
			AddSeparator("!vertical", i*2 + 4, 0, 0, 0, "separator_" + i2s(i));
		}
		HideControl("comment_no_fx", track->midi.fx.num > 0);
	}else{
		HideControl("comment_no_fx", false);
	}
	//Enable("add", track);
}

