/*
 * SynthesizerDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ConfigurableSelectorDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"

Configurable *ConfigurableSelectorDialog::_return;

ConfigurableSelectorDialog::ConfigurableSelectorDialog(HuiWindow* _parent, int _type, Song *_song, const string &old_name) :
	HuiWindow("configurable-selection-dialog", _parent)
{
	type = _type;
	song = _song;
	if (type == Configurable::TYPE_EFFECT){
		string prefix = HuiAppDirectoryStatic + "Plugins/Buffer/";
		for (auto &pf : tsunami->plugin_manager->plugin_files){
			if (pf.filename.match(prefix + "*")){
				names.add(pf.name);
				string g = pf.filename.substr(prefix.num, -1).explode("/")[0];
				groups.add(g);
				ugroups.add(g);
			}
		}
	}else if (type == Configurable::TYPE_MIDI_EFFECT){
		for (auto &pf : tsunami->plugin_manager->plugin_files){
			if (pf.filename.match(HuiAppDirectoryStatic + "Plugins/Midi/*"))
				names.add(pf.name);
		}
	}else if (type == Configurable::TYPE_SYNTHESIZER){
		names = FindSynthesizers();
	}

	for (string &g : ugroups)
		setString("list", g);

	foreachi(string &name, names, i){
		int n = i;
		if (groups.num > 0){
			int r = ugroups.find(groups[i]);
			addChildString("list", r, name);
		}else
			setString("list", name);
		if (name == old_name)
			setInt("list", n);
	}

	event("hui:close", this, &ConfigurableSelectorDialog::onClose);
	eventX("list", "hui:select", this, &ConfigurableSelectorDialog::onListSelect);
	event("list", this, &ConfigurableSelectorDialog::onSelect);
	event("cancel", this, &ConfigurableSelectorDialog::onCancel);
	event("ok", this, &ConfigurableSelectorDialog::onOk);
	enable("ok", false);

	_return = NULL;
}

ConfigurableSelectorDialog::~ConfigurableSelectorDialog()
{
}

void ConfigurableSelectorDialog::onListSelect()
{
	int n = getInt("list") - ugroups.num;
	enable("ok", n >= 0);
}

void ConfigurableSelectorDialog::onSelect()
{
	int n = getInt("list") - ugroups.num;
	if (n < 0)
		return;
	if (type == Configurable::TYPE_EFFECT)
		_return = CreateEffect(names[n], song);
	else if (type == Configurable::TYPE_MIDI_EFFECT)
		_return = CreateMidiEffect(names[n], song);
	else if (type == Configurable::TYPE_SYNTHESIZER)
		_return = CreateSynthesizer(names[n], song);
	delete(this);
}

void ConfigurableSelectorDialog::onClose()
{
	delete(this);
}

void ConfigurableSelectorDialog::onCancel()
{
	delete(this);
}

void ConfigurableSelectorDialog::onOk()
{
	onSelect();
}

Synthesizer *ChooseSynthesizer(HuiWindow *parent, Song *song, const string &old_name)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent, Configurable::TYPE_SYNTHESIZER, song, old_name);
	dlg->run();
	return (Synthesizer*)ConfigurableSelectorDialog::_return;
}
