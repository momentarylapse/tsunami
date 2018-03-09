/*
 * SynthesizerDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ConfigurableSelectorDialog.h"
#include "../../Session.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Plugins/Plugin.h"

ConfigurableSelectorDialog::Label ConfigurableSelectorDialog::split_label(const string &s)
{
	Label l;
	l.full = s;
	auto ss = s.explode("/");
	l.name = ss.back();
	l.group = implode(ss.sub(0, ss.num-1), "/");
	return l;
}

ConfigurableSelectorDialog::ConfigurableSelectorDialog(hui::Window* _parent, int _type, Session *_session, const string &old_name) :
	hui::Window("configurable-selection-dialog", _parent)
{
	type = _type;
	session = _session;
	Array<string> tnames = session->plugin_manager->FindConfigurable(type);

	for (string &s: tnames){
		Label l = split_label(s);
		labels.add(l);
		if (l.group.num > 0)
			ugroups.add(l.group);
	}

	for (string &g: ugroups)
		setString("list", g);

	foreachi (Label &l, labels, i){
		int n = i;
		if (ugroups.num > 0){
			int r = ugroups.find(l.group);
			addChildString("list", r, l.name);
		}else
			setString("list", l.name);
		if (l.name == old_name)
			setInt("list", n);
	}

	event("hui:close", std::bind(&ConfigurableSelectorDialog::onClose, this));
	eventX("list", "hui:select", std::bind(&ConfigurableSelectorDialog::onListSelect, this));
	event("list", std::bind(&ConfigurableSelectorDialog::onSelect, this));
	event("cancel", std::bind(&ConfigurableSelectorDialog::onCancel, this));
	event("ok", std::bind(&ConfigurableSelectorDialog::onOk, this));
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
		_return = CreateEffect(session, labels[n].name);
	else if (type == Configurable::TYPE_MIDI_EFFECT)
		_return = CreateMidiEffect(session, labels[n].name);
	else if (type == Configurable::TYPE_SYNTHESIZER)
		_return = session->plugin_manager->CreateSynthesizer(session, labels[n].name);
	destroy();
}

void ConfigurableSelectorDialog::onClose()
{
	destroy();
}

void ConfigurableSelectorDialog::onCancel()
{
	destroy();
}

void ConfigurableSelectorDialog::onOk()
{
	onSelect();
}
