/*
 * HuiToolbar.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "Toolbar.h"
#include "Resource.h"
#include "Menu.h"
#include "Window.h"
#include "language.h"
#include "Controls/ToolItemButton.h"
#include "Controls/ToolItemMenuButton.h"
#include "Controls/ToolItemSeparator.h"
#include "Controls/ToolItemToggleButton.h"
#include "../os/msg.h"
#include <lib/layout/Resource.h>

#include <gtk/gtk.h>

namespace hui
{

//void control_delete_rec(Control *c);
Menu *_create_res_menu_(const string &ns, const layout::Resource *res, Panel *p);


// add a default button
void Toolbar::add(const string &title, const string &image, const string &id) {
	_add(new ToolItemButton(title, image, id));
}

// add a checkable button
void Toolbar::add_checkable(const string &title, const string &image, const string &id) {
	_add(new ToolItemToggleButton(title, image, id));
}

void Toolbar::add_menu(const string &title, const string &image, Menu *menu, const string &id) {
	if (!menu)
		return;
	_add(new ToolItemMenuButton(title, menu, image, id, win));
	menu->set_panel(win);
}

void Toolbar::add_menu_by_id(const string &title, const string &image, const string &menu_id, const string &id) {
	Menu *menu = create_resource_menu(menu_id, win);
	add_menu(title, image, menu, id);
}

void Toolbar::add_separator() {
	_add(new ToolItemSeparator());
}

// remove all items from the toolbar
void Toolbar::reset() {
	auto _items = weak(items);
	for (auto i: _items) {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_box_remove(GTK_BOX(widget), i->get_frame());
#else
		gtk_container_remove(GTK_CONTAINER(widget), i->widget);
#endif
		//control_delete_rec(i);
	}
	items.clear();
}

// create and apply a toolbar bar resource id
void Toolbar::set_by_id(const string &id) {
	auto *res = get_resource(id);
	if (!res) {
		msg_error("Toolbar.SetByID  :~~(");
		return;
	}
	from_resource(res);
}

void Toolbar::from_resource(const layout::Resource *res) {
	reset();
	id = res->id;
	//Configure(res->b_param[0], res->b_param[1]);
	for (const auto &cmd: res->children) {
		string title = get_lang(id, cmd.id, cmd.title, false);
		string tooltip = get_language_t(id, cmd.id, cmd.value("tooltip"));
		if (tooltip.num == 0)
			tooltip = title;

		if (cmd.type == "Item") {
			if (cmd.has("checkable"))
				add_checkable(title, cmd.value("image"), cmd.id);
			else
				add(title, cmd.value("image"), cmd.id);
			items.back()->set_tooltip(tooltip);
		} else if (cmd.type == "Separator") {
			add_separator();
		} else if (cmd.type == "Menu") {
			bool ok = false;
			for (const auto &o: cmd.options)
				if (o.key == "menu=") {
					add_menu_by_id(title, cmd.value("image"), o.value, cmd.id);
					items.back()->set_tooltip(get_language_t(id, cmd.id, cmd.value("tooltip")));
					ok = true;
				}
			if ((!ok) and (cmd.children.num > 0)) {
				add_menu(title, cmd.value("image"), _create_res_menu_(id, &cmd, win), cmd.id);
				items.back()->set_tooltip(get_language_t(id, cmd.id, cmd.value("tooltip")));
			}
		}
		for (auto &o: cmd.options)
			items.back()->set_options(o.str());
	}
	enable(true);
}

void Toolbar::from_source(const string &source) {
	auto res = layout::parse_resource(source);
	from_resource(&res);
}


void Toolbar::apply_foreach(const string &id, std::function<void(Control*)> f) {
	for (Control *c: weak(items))
		c->apply_foreach(id, f);

}

};

