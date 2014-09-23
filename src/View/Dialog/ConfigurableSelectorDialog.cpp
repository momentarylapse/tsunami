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

ConfigurableSelectorDialog::ConfigurableSelectorDialog(HuiWindow* _parent, int _type, const string &old_name) :
	HuiDialog("Selector", 400, 500, _parent, false)
{
	AddTreeView("Name", 0, 0, 0, 0, "list");

	type = _type;
	if (type == Configurable::TYPE_EFFECT){
		string prefix = HuiAppDirectoryStatic + "Plugins/Buffer/";
		foreach(PluginManager::PluginFile &pf, tsunami->plugin_manager->plugin_file){
			if (pf.filename.match(prefix + "*")){
				names.add(pf.name);
				string g = pf.filename.substr(prefix.num, -1).explode("/")[0];
				groups.add(g);
				ugroups.add(g);
			}
		}
	}else if (type == Configurable::TYPE_MIDI_EFFECT){
		foreach(PluginManager::PluginFile &pf, tsunami->plugin_manager->plugin_file){
			if (pf.filename.match(HuiAppDirectoryStatic + "Plugins/Midi/*"))
				names.add(pf.name);
		}
	}else if (type == Configurable::TYPE_SYNTHESIZER){
		names = FindSynthesizers();
	}

	foreach(string &g, ugroups)
		SetString("list", g);

	foreachi(string &name, names, i){
		int n = i;
		if (groups.num > 0){
			int r = ugroups.find(groups[i]);
			AddChildString("list", r, name);
		}else
			SetString("list", name);
		if (name == old_name)
			SetInt("list", n);
	}

	EventM("hui:close", this, &ConfigurableSelectorDialog::onClose);
	EventM("list", this, &ConfigurableSelectorDialog::onSelect);

	_return = NULL;
}

ConfigurableSelectorDialog::~ConfigurableSelectorDialog()
{
}

void ConfigurableSelectorDialog::onSelect()
{
	int n = GetInt("list") - ugroups.num;
	if (n < 0)
		return;
	if (type == Configurable::TYPE_EFFECT)
		_return = CreateEffect(names[n]);
	else if (type == Configurable::TYPE_MIDI_EFFECT)
		_return = CreateMidiEffect(names[n]);
	else if (type == Configurable::TYPE_SYNTHESIZER)
		_return = CreateSynthesizer(names[n]);
	delete(this);
}

void ConfigurableSelectorDialog::onClose()
{
	delete(this);
}

Synthesizer *ChooseSynthesizer(HuiWindow *parent, const string &old_name)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent, Configurable::TYPE_SYNTHESIZER, old_name);
	dlg->Run();
	return (Synthesizer*)ConfigurableSelectorDialog::_return;
}
