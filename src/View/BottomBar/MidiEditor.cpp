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
		addControlTable("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		setTarget("grid", 0);
		addControlTable("", 0, 0, 5, 1, "header");
		setTarget("header", 0);
		addButton("!flat", 0, 0, 0, 0, "load_favorite");
		setImage("load_favorite", "hui:open");
		setTooltip("load_favorite", _("Parameter laden"));
		addButton("!flat", 1, 0, 0, 0, "save_favorite");
		setImage("save_favorite", "hui:save");
		setTooltip("save_favorite", _("Parameter speichern"));
		addText("!bold,center,expandx\\" + fx->name, 2, 0, 0, 0, "");
		addCheckBox("", 3, 0, 0, 0, "enabled");
		setTooltip("enabled", _("aktiv?"));
		addButton("!flat", 4, 0, 0, 0, "delete");
		setImage("delete", "hui:delete");
		setTooltip("delete", _("Effekt l&oschen"));
		p = fx->createPanel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid", 0);
			addText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("enabled", this, &SingleMidiFxPanel::onEnabled);
		event("delete", this, &SingleMidiFxPanel::onDelete);
		event("load_favorite", this, &SingleMidiFxPanel::onLoad);
		event("save_favorite", this, &SingleMidiFxPanel::onSave);

		check("enabled", fx->enabled);

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
			track->editMidiEffect(index, old_param);
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
			track->enableMidiEffect(index, isChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->deleteMidiEffect(index);
	}
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			if (track)
				track->editMidiEffect(index, old_param);
		}
		check("enabled", fx->enabled);
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

	fromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	Array<string> chord_types = GetChordTypeNames();
	foreach(string &ct, chord_types)
		addString("chord_type", ct);
	setInt("chord_type", 0);
	enable("chord_type", false);
	addString("chord_inversion", _("Grundform"));
	addString("chord_inversion", _("1. Umkehrung"));
	addString("chord_inversion", _("2. Umkehrung"));
	setInt("chord_inversion", 0);
	enable("chord_inversion", false);

	setTooltip("add", _("neuen Effekt hinzuf&ugen"));

	for (int i=0; i<12; i++)
		addString("scale", rel_pitch_name(i) + " " + GetChordTypeName(CHORD_TYPE_MAJOR) + " / " + rel_pitch_name(pitch_to_rel(i + 9)) + " " + GetChordTypeName(CHORD_TYPE_MINOR));
	setInt("scale", view->midi_scale);

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("pitch_offset", this, &MidiEditor::onPitch);
	event("beat_partition", this, &MidiEditor::onBeatPartition);
	event("scale", this, &MidiEditor::onScale);
	event("midi_mode:select", this, &MidiEditor::onMidiModeSelect);
	event("midi_mode:note", this, &MidiEditor::onMidiModeNote);
	event("midi_mode:chord", this, &MidiEditor::onMidiModeChord);
	event("chord_type", this, &MidiEditor::onChordType);
	event("chord_inversion", this, &MidiEditor::onChordInversion);

	event("add", this, &MidiEditor::onAdd);

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
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);
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
	view->pitch_min = getInt("");
	view->pitch_max = getInt("") + 30;
	view->forceRedraw();
}

void MidiEditor::onScale()
{
	view->midi_scale = getInt("");
	view->forceRedraw();
}

void MidiEditor::onBeatPartition()
{
	view->beat_partition = getInt("");
	view->forceRedraw();
}

void MidiEditor::onMidiModeSelect()
{
	view->midi_mode = view->MIDI_MODE_SELECT;
	enable("chord_type", false);
	enable("chord_inversion", false);
}

void MidiEditor::onMidiModeNote()
{
	view->midi_mode = view->MIDI_MODE_NOTE;
	enable("chord_type", false);
	enable("chord_inversion", false);
}

void MidiEditor::onMidiModeChord()
{
	view->midi_mode = view->MIDI_MODE_CHORD;
	enable("chord_type", true);
	enable("chord_inversion", true);
}

void MidiEditor::onChordType()
{
	view->chord_type = getInt("");
}

void MidiEditor::onChordInversion()
{
	view->chord_inversion = getInt("");
}


void MidiEditor::onAdd()
{
	MidiEffect *effect = tsunami->plugin_manager->ChooseMidiEffect(this);
	if (!effect)
		return;
	if (track)
		track->addMidiEffect(effect);
}

void MidiEditor::clear()
{
	if (track)
		unsubscribe(track);
	foreachi(HuiPanel *p, panels, i){
		delete(p);
		removeControl("separator_" + i2s(i));
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
			embed(panels.back(), id_inner, i*2 + 3, 0);
			addSeparator("!vertical", i*2 + 4, 0, 0, 0, "separator_" + i2s(i));
		}
		hideControl("comment_no_fx", track->midi.fx.num > 0);
	}else{
		hideControl("comment_no_fx", false);
	}
	//Enable("add", track);
}

