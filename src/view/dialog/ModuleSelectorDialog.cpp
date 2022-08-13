/*
 * ModuleSelectorDialog.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ModuleSelectorDialog.h"

#include "../../Session.h"
#include "../../plugins/PluginManager.h"
#include "../../module/Module.h"

ModuleSelectorDialog::Label ModuleSelectorDialog::split_label(const string &s) {
	Label l;
	l.full = s;
	auto ss = s.explode("/");
	l.name = ss.back();
	l.group = implode(ss.sub_ref(0, -1), "/");
	return l;
}

ModuleSelectorDialog::ModuleSelectorDialog(hui::Window* _parent, ModuleCategory _type, Session *_session, const string &old_name) :
	hui::Dialog("configurable-selection-dialog", _parent)
{
	type = _type;
	session = _session;
	set_title(Module::category_to_str(type));
	auto tnames = session->plugin_manager->find_module_sub_types_grouped(type);

	for (string &s: tnames) {
		Label l = split_label(s);
		labels.add(l);
		if (l.group.num > 0)
			ugroups.add(l.group);
	}

	for (string &g: ugroups)
		set_string("list", g);

	foreachi (Label &l, labels, i) {
		int n = i;
		auto ll = format("%s\\%s", l.name, session->plugin_manager->is_favorite(session, type, l.name) ? "\u2764" : "");
		if (ugroups.num > 0) {
			int r = ugroups.find(l.group);
			add_child_string("list", r, ll);
		} else {
			set_string("list", ll);
		}
		if (l.name == old_name)
			set_int("list", n);
	}
	expand("list", true);

	event("hui:close", [this] { request_destroy(); });
	event_x("list", "hui:select", [this] { on_list_select(); });
	event("list", [this] { on_select(); });
	event("toggle-favorite", [this] { on_toggle_favorite(); });
	event("cancel", [this] { request_destroy(); });
	event("ok", [this] { on_select(); });
	enable("ok", false);
}

void ModuleSelectorDialog::on_list_select() {
	int n = get_int("list") - ugroups.num;
	enable("ok", n >= 0);
	enable("toggle-favorite", n >= 0);
	if (n >= 0)
		check("toggle-favorite", session->plugin_manager->is_favorite(session, type, labels[n].name));
}

void ModuleSelectorDialog::on_select() {
	int n = get_int("list") - ugroups.num;
	if (n < 0)
		return;
	_return = labels[n].name;
	request_destroy();
}

void ModuleSelectorDialog::on_toggle_favorite() {
	int n = get_int("list") - ugroups.num;
	if (n < 0)
		return;
	auto &l = labels[n];
	session->plugin_manager->set_favorite(session, type, l.name, is_checked("toggle-favorite"));

	set_cell("list", ugroups.num + n, 1, session->plugin_manager->is_favorite(session, type, l.name) ? "\u2764" : "");
}
