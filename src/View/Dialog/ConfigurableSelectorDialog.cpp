/*
 * SynthesizerDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ConfigurableSelectorDialog.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"

ConfigurableSelectorDialog::Label ConfigurableSelectorDialog::split_label(const string &s)
{
	Label l;
	l.full = s;
	auto ss = s.explode("/");
	l.name = ss.back();
	l.group = implode(ss.sub(0, ss.num-1), "/");
	return l;
}

ConfigurableSelectorDialog::ConfigurableSelectorDialog(hui::Window* _parent, ModuleType _type, Session *_session, const string &old_name) :
	hui::Window("configurable-selection-dialog", _parent)
{
	type = _type;
	session = _session;
	Array<string> tnames = session->plugin_manager->find_module_sub_types_grouped(type);

	for (string &s: tnames){
		Label l = split_label(s);
		labels.add(l);
		if (l.group.num > 0)
			ugroups.add(l.group);
	}

	for (string &g: ugroups)
		set_string("list", g);

	foreachi (Label &l, labels, i){
		int n = i;
		if (ugroups.num > 0){
			int r = ugroups.find(l.group);
			add_child_string("list", r, l.name);
		}else
			set_string("list", l.name);
		if (l.name == old_name)
			set_int("list", n);
	}

	event("hui:close", std::bind(&ConfigurableSelectorDialog::on_close, this));
	event_x("list", "hui:select", std::bind(&ConfigurableSelectorDialog::on_list_select, this));
	event("list", std::bind(&ConfigurableSelectorDialog::on_select, this));
	event("cancel", std::bind(&ConfigurableSelectorDialog::on_cancel, this));
	event("ok", std::bind(&ConfigurableSelectorDialog::on_ok, this));
	enable("ok", false);
}

ConfigurableSelectorDialog::~ConfigurableSelectorDialog()
{
}

void ConfigurableSelectorDialog::on_list_select()
{
	int n = get_int("list") - ugroups.num;
	enable("ok", n >= 0);
}

void ConfigurableSelectorDialog::on_select()
{
	int n = get_int("list") - ugroups.num;
	if (n < 0)
		return;
	_return = labels[n].name;
	destroy();
}

void ConfigurableSelectorDialog::on_close()
{
	destroy();
}

void ConfigurableSelectorDialog::on_cancel()
{
	destroy();
}

void ConfigurableSelectorDialog::on_ok()
{
	on_select();
}
