/*
 * HuiPanel.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Controls/Control.h"
#include "Controls/MenuItem.h"
#include "Controls/MenuItemToggle.h"
#include "hui.h"
#include "internal.h"
#include "../image/color.h"
#include "../os/msg.h"

#include <gtk/gtk.h>

namespace hui
{

// for unique window identifiers
static int current_uid = 0;

const int DEFAULT_SPACING = 5;
const int DEFAULT_WINDOW_BORDER = 8;

string get_gtk_action_name(const string &id, Panel *scope);


#define DEBUG_CONTROLS 0

void DBDEL_START(const string &type, const string &id, void *p) {
#if DEBUG_CONTROLS
	msg_write("del " + type + " " + id);
	msg_right();
#endif
}

void DBDEL_X(const string &m) {
#if DEBUG_CONTROLS
	msg_write(m);
#endif
}

void DBDEL_DONE() {
#if DEBUG_CONTROLS
	msg_left();
	msg_write("/del");
#endif
}


Panel::Panel(const string &_id, Panel *_parent) : Panel() {
	set_parent(_parent);
	set_id(_id);
}

Panel::Panel() {
	win = nullptr;
	parent = nullptr;
	border_width = DEFAULT_WINDOW_BORDER;
	spacing = DEFAULT_SPACING;
	id = p2s(this);
	num_float_decimals = 3;
	root_control = nullptr;
	current_event_listener_uid = 0;

	unique_id = current_uid ++;

#if GTK_CHECK_VERSION(4,0,0)
	action_group = g_simple_action_group_new();
#endif

	set_target("");
}

Panel::~Panel() {
	DBDEL_START("Panel", id, this);
	event_listeners.clear();
	if (_pointer_ref_counter > 0)
		msg_error("hui.Panel deleted while still owned: " + id);
	if (parent) {
		// disconnect from parent panel -> no owners here!
		parent = nullptr;
	}
	DBDEL_X("root");
	root_control = nullptr;

	DBDEL_DONE();
}

void Panel::set_id(const string &_id) {
	id = _id;
}

void Panel::set_parent(Panel *_parent) {
	parent = _parent;
	if (parent)
		_set_win(parent->win);
}

// might be executed repeatedly
void Panel::_ClearPanel_() {
}

void Panel::set_border_width(int width) {
	border_width = width;
}

void Panel::set_spacing(int width) {
	spacing = width;
}

void Panel::set_decimals(int decimals) {
	num_float_decimals = decimals;
}

string Panel::_get_cur_id_() {
	return cur_id;
}

void Panel::_set_cur_id_(const string &id) {
	if (win)
		win->cur_id = id;
	cur_id = id;
}

int Panel::event(const string &id, const Callback &function) {
	return event_x(id, EventID::_MATCH_DEFAULT_, function);
}

int Panel::event_x(const string &id, const string &msg, const Callback &function) {
	if (!EventID::is_valid(msg))
		msg_error(format("invalid event message: %s/%s", id, msg));
	int uid = current_event_listener_uid ++;
#if GTK_CHECK_VERSION(4,0,0)
	_try_add_action_(id, false);
#endif
	event_listeners.add(EventListener(uid, id, msg, function));
	return uid;
}

// hopefully deprecated soon?
int Panel::event_xp(const string &id, const string &msg, const CallbackP &function) {
	if (!EventID::is_valid(msg))
		msg_error(format("invalid event message: %s/%s", id, msg));
	int uid = current_event_listener_uid ++;
	event_listeners.add(EventListener(uid, id, msg, -1, function));
	return uid;
}

void Panel::remove_event_handler(int event_handler_id) {
	for (int i=event_listeners.num-1; i>=0; i--)
		if (event_listeners[i].uid == event_handler_id)
			event_listeners.erase(i);
}

bool Panel::_send_event_(Event *e, bool force_if_not_allowed) {
	if (!win)
		return false;
	if (!win->allow_input and !force_if_not_allowed)
		return false;

	CurWindow = win;
	e->win = win;
	e->m = {win->input.x, win->input.y};
	e->d = {win->input.dx, win->input.dy};
	e->scroll = {win->input.scroll_x, win->input.scroll_y};
	e->pressure = win->input.pressure;
	e->lbut = win->input.lb;
	e->mbut = win->input.mb;
	e->rbut = win->input.rb;
	e->key_code = win->input.key_code;
	e->row = win->input.row;
	e->row_target = win->input.row_target;
	e->column = win->input.column;
	e->just_focused = win->input.just_focused;
	_hui_event_ = *e;
	if (e->id.num > 0)
		_set_cur_id_(e->id);
	else
		_set_cur_id_(e->message);

	bool sent = false;

	for (auto &ee: event_listeners) {
		if (!e->match(ee.id, ee.message))
			continue;

		// send the event

		if (e->message == EventID::DRAW) {
			if (ee.function_p) {
				Painter p(this, e->id);
				ee.function_p(&p);
				sent = true;
			}
		} else {
			if (ee.function) {
				ee.function();
				sent = true;
			}
		}
	}

	// reset
	win->input.dx = 0;
	win->input.dy = 0;
	win->input.scroll_x = 0;
	win->input.scroll_y = 0;

	return sent;
}

int Panel::_get_unique_id_() {
	return unique_id;
}

void Panel::show() {
	if (this == win)
		win->show();
	else if (root_control)
		root_control->hide(false);
	on_show();
}

void Panel::hide() {
	if (this == win)
		win->hide();
	else if (root_control)
		root_control->hide(true);
	on_hide();
}

//----------------------------------------------------------------------------------
// easy window creation functions


void Panel::add_control(const string &type, const string &title, int x, int y, const string &id) {
	//printf("HuiPanelAddControl %s  %s  %d  %d  %s\n", type.c_str(), title.c_str(), x, y, id.c_str());
	if (type == "Button")
		add_button(title, x, y, id);
	else if (type == "ColorButton")
		add_color_button(title, x, y, id);
	else if (type == "DefButton")
		add_def_button(title, x, y, id);
	else if ((type == "Label") or (type == "Text"))
		add_label(title, x, y, id);
	else if (type == "Edit")
		add_edit(title, x, y, id);
	else if (type == "MultilineEdit")
		add_multiline_edit(title, x, y, id);
	else if (type == "Group")
		add_group(title, x, y, id);
	else if (type == "CheckBox")
		add_check_box(title, x, y, id);
	else if (type == "ComboBox")
		add_combo_box(title, x, y, id);
	else if (type == "TabControl")
		add_tab_control(title, x, y, id);
	else if (type == "ListView")
		add_list_view(title, x, y, id);
	else if (type == "TreeView")
		add_tree_view(title, x, y, id);
	else if (type == "IconView")
		add_icon_view(title, x, y, id);
	else if (type == "ProgressBar")
		add_progress_bar(title, x, y, id);
	else if (type == "Slider")
		add_slider(title, x, y, id);
	else if (type == "Image")
		add_image(title, x, y, id);
	else if (type == "DrawingArea")
		add_drawing_area(title, x, y, id);
	else if ((type == "ControlTable") or (type == "Grid"))
		add_grid(title, x, y, id);
	else if (type == "SpinButton")
		add_spin_button(title, x, y, id);
	else if (type == "RadioButton")
		add_radio_button(title, x, y, id);
	else if (type == "ToggleButton")
		add_toggle_button(title, x, y, id);
	else if ((type == "Expander") or (type == "Revealer"))
		add_expander(title, x, y, id);
	else if (type == "Scroller")
		add_scroller(title, x, y, id);
	else if (type == "Paned")
		add_paned(title, x, y, id);
	else if (type == "Separator")
		add_separator(title, x, y, id);
	else if (type == "MenuButton")
		add_menu_button(title, x, y, id);
	else
		msg_error("unknown hui control: " + type);
}

void Panel::_add_control(const string &ns, Resource &cmd, const string &parent_id) {
	//msg_write(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
	set_target(parent_id);
	add_control(cmd.type, get_language_r(ns, cmd),
				cmd.x, cmd.y,
				cmd.id);

	for (string &o: cmd.options)
		set_options(cmd.id, o);

	enable(cmd.id, cmd.enabled());
	if (cmd.has("hidden"))
		hide_control(cmd.id, true);

	if (cmd.image().num > 0)
		set_image(cmd.id, cmd.image());


	string tooltip = get_language_t(ns, cmd.id, cmd.tooltip);
	if (tooltip.num > 0)
		set_tooltip(cmd.id, tooltip);

	for (Resource &c: cmd.children)
		_add_control(ns, c, cmd.id);
}

void Panel::from_resource(const string &id) {
	Resource *res = get_resource(id);
	if (res)
		set_from_resource(res);
}

void Panel::set_from_resource(Resource *res) {
	if (!res)
		return;


	bool res_is_window = ((res->type == "Dialog") or (res->type == "Window"));
	bool panel_is_window = win and !parent;

	// directly change window?
	if (panel_is_window and res_is_window) {
		for (auto &o: res->options)
			win->__set_options(o);

		// title
		win->set_title(get_language(res->id, res->id));

		// size
		int width = res->value("width", "0")._int();
		int height = res->value("height", "0")._int();
		if (width + height > 0)
			win->set_size(width, height);

		// menu/toolbar?
		string toolbar = res->value("toolbar");
		string menu = res->value("menu");
		if (menu != "")
			win->set_menu(create_resource_menu(menu, this));
		if (toolbar != "")
			win->get_toolbar(TOOLBAR_TOP)->set_by_id(toolbar);

		for (auto &c: res->children)
			if (c.type == "HeaderBar") {
				win->_add_headerbar();
				for (auto &cc: c.children)
					_add_control(id, cc, ":header:");
			}
	}

	set_id(res->id);

	int bw = res->value("borderwidth", "-1")._int();
	if (bw >= 0)
		set_border_width(bw);


	// controls
	if (res_is_window) {
		if (res->children.num > 0)
			_add_control(id, res->children[0], "");
	} else {
		embed_resource(*res, "", 0, 0);
	}
}

void Panel::from_source(const string &buffer) {
	Resource res = parse_resource(buffer);
	set_from_resource(&res);
}


void Panel::embed_resource(Resource &c, const string &parent_id, int x, int y) {
	_embed_resource(c.id, c, parent_id, x, y);
}

void Panel::_embed_resource(const string &ns, Resource &c, const string &parent_id, int x, int y) {
	//_addControl(main_id, c, parent_id);

	set_target(parent_id);
	string title = get_language_r(ns, c);
	//if (c.options.num > 0)
	//	title = "!" + implode(c.options, ",") + "\\" + title;
	add_control(c.type, title, x, y, c.id);
	for (string &o: c.options)
		set_options(c.id, o);

	enable(c.id, c.enabled());
	if (c.image().num > 0)
		set_image(c.id, c.image());

	string tooltip = get_language_t(ns, c.id, c.tooltip);
	if (tooltip.num > 0)
		set_tooltip(c.id, tooltip);

	for (Resource &child : c.children)
		_embed_resource(ns, child, c.id, child.x, child.y);
}

void Panel::embed_source(const string &buffer, const string &parent_id, int x, int y) {
	Resource res = parse_resource(buffer);
	embed_resource(res, parent_id, x, y);
}

// TODO shared<> auto-cast...
void Panel::embed(shared<Panel> panel, const string &parent_id, int x, int y) {
	if (!panel)
		return;
	if (!panel->root_control) {
		msg_error("trying to embed an empty panel");
		return;
	}
	panel->set_parent(this);
	children.add(panel);

	Panel* orig = panel->root_control->panel;
	if (orig != panel)
		msg_error("hmmm, something's fishy..." + id + " : " + panel->id);

	set_target(parent_id);
	if (parent_id.num > 0 and !_get_control_(parent_id))
		msg_error(parent_id + " not found...embed");
	_insert_control_(panel->root_control, x, y);
	panel->root_control->panel = panel.get();

#if GTK_CHECK_VERSION(4,0,0)
	//msg_error("ATTACH ACTION GROUP  " + p2s(panel.get()));
	if (win) {
		gtk_widget_insert_action_group(win->window, p2s(panel.get()).c_str(), G_ACTION_GROUP(panel->action_group));
	}
#endif
}

void Panel::unembed(Panel *p) {
	if (p->parent != this)
		msg_error("Panel.unembed(): p.parent != this: " + p->id);

	// unlink embedded control
	p->root_control->parent->remove_child(p->root_control.get());

	// unlink from this
	p->parent = nullptr;
	for (int i=0; i<children.num; i++)
		if (children[i] == p)
			children.erase(i);
	// p might be deleted now
}

void Panel::_set_win(Window *_win) {
	win = _win;
	for (Panel *p: weak(children))
		p->_set_win(win);
}


//----------------------------------------------------------------------------------
// data exchanging functions for control items


// used for automatic type casting...
bool panel_equal(Panel *a, Panel *b) {
	return a == b;
}

void Panel::apply_foreach(const string &_id, std::function<void(Control*)> f) {
	string id = _id;
	if (id == "")
		id = cur_id;
	if (root_control)
		root_control->apply_foreach(id, f);

	// FIXME: might be a derived class by kaba....
	if (panel_equal(win, this)) {
		if (auto h = win->header_bar)
			h->apply_foreach(id, f);
		if (auto m = win->get_menu())
			m->apply_foreach(id, f);
		/*if (win->popup)
			win->popup->apply_foreach(id, f);*/
		for (int i=0; i<4; i++)
			if (auto t = win->get_toolbar(i))
				t->apply_foreach(id, f);
	}
}


// replace all the text
//    for all
void Panel::set_string(const string &_id, const string &str) {
	if (win and (id == _id))
		win->set_title(str);
	apply_foreach(_id, [&str](Control *c) {
		c->set_string(str);
	});
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void Panel::set_int(const string &_id, int n) {
	apply_foreach(_id, [n](Control *c) {
		c->set_int(n);
	});
}

// replace all the text with a float
//    for all
void Panel::set_float(const string &_id, float f) {
	apply_foreach(_id, [f](Control *c) {
		c->set_float(f);
	});
}

void Panel::set_image(const string &_id, const string &image) {
	apply_foreach(_id, [&image](Control *c) {
		c->set_image(image);
	});
}

void Panel::set_tooltip(const string &_id, const string &tip) {
	apply_foreach(_id, [&tip](Control *c) {
		c->set_tooltip(tip);
	});
}


// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void Panel::add_string(const string &_id, const string &str) {
	apply_foreach(_id, [&str](Control *c) {
		c->add_string(str);
	});
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void Panel::add_child_string(const string &_id, int parent_row, const string &str) {
	apply_foreach(_id, [parent_row,&str](Control *c) {
		c->add_child_string(parent_row, str);
	});
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::change_string(const string &_id, int row, const string &str) {
	apply_foreach(_id, [row,&str](Control *c) {
		c->change_string(row, str);
	});
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::remove_string(const string &_id, int row) {
	apply_foreach(_id, [row](Control *c) {
		c->remove_string(row);
	});
}

// listview / treeview
string Panel::get_cell(const string &_id, int row, int column) {
	string r = "";
	apply_foreach(_id, [&r,row,column](Control *c) {
		r = c->get_cell(row, column);
	});
	return r;
}

// listview / treeview
void Panel::set_cell(const string &_id, int row, int column, const string &str) {
	apply_foreach(_id, [row,column,&str](Control *c) {
		c->set_cell(row, column, str);
	});
}

void Panel::set_color(const string &_id, const color &col) {
	apply_foreach(_id, [&col](Control *c) {
		c->set_color(col);
	});
}

// retrieve the text
//    for edit
string Panel::get_string(const string &_id) {
	string r = "";
	apply_foreach(_id, [&r](Control *c) {
		r = c->get_string();
	});
	return r;
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int Panel::get_int(const string &_id) {
	int r = 0;
	apply_foreach(_id, [&r](Control *c) {
		r = c->get_int();
	});
	return r;
}

// retrieve the text as a numerical value (float)
//    for edit
float Panel::get_float(const string &_id) {
	float r = 0;
	apply_foreach(_id, [&r](Control *c) {
		r = c->get_float();
	});
	return r;
}

color Panel::get_color(const string &_id) {
	color r = Black;
	apply_foreach(_id, [&r](Control *c) {
		r = c->get_color();
	});
	return r;
}


// might be called from menus in preparation
GAction *panel_get_action(Panel *panel, const string &id) {
#if GTK_CHECK_VERSION(4,0,0)
	if (!panel) {
		//msg_error("NO PANEL..." + id);
		return nullptr;
	}
	return panel->_get_action(id);
	if (!panel->win) {
		//msg_error("NO WINDOW..." + id);
		return nullptr;
	}

	//return panel->win->_get_action(id);
#else
	return nullptr;
#endif
}

// switch control to usable/unusable
//    for all
void Panel::enable(const string &_id,bool enabled) {
	apply_foreach(_id, [enabled](Control *c) {
		c->enable(enabled);
	});
}

// show/hide control
//    for all
void Panel::hide_control(const string &_id,bool hide) {
	apply_foreach(_id, [hide](Control *c) {
		c->hide(hide);
	});
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void Panel::check(const string &_id,bool checked) {
	apply_foreach(_id, [checked](Control *c) {
		c->check(checked);
	});
}

// is marked as "checked"?
//    for CheckBox
bool Panel::is_checked(const string &_id) {
	bool r = false;
	apply_foreach(_id, [&r](Control *c) {
		r = c->is_checked();
	});
	return r;
}

// which lines are selected?
//    for ListView
Array<int> Panel::get_selection(const string &_id) {
	Array<int> r;
	apply_foreach(_id, [&r](Control *c) {
		r = c->get_selection();
	});
	return r;
}

void Panel::set_selection(const string &_id, const Array<int> &sel) {
	apply_foreach(_id, [&sel](Control *c) {
		c->set_selection(sel);
	});
}

// delete all the content
//    for ComboBox, ListView
void Panel::reset(const string &_id) {
	apply_foreach(_id, [](Control *c) {
		c->reset();
	});
}

// expand a single row
//    for TreeView, Expander
void Panel::expand_row(const string &_id, int row, bool expanded) {
	apply_foreach(_id, [row,expanded](Control *c) {
		c->expand(row, expanded);
	});
}

// expand all rows
//    for TreeView, Expander
void Panel::expand(const string &_id, bool expanded) {
	expand_row(_id, -1, expanded);
}

// is column in tree expanded?
//    for TreeView
bool Panel::is_expanded(const string &_id, int row) {
	bool r = false;
	apply_foreach(_id, [&r, row](Control *c) {
		r = c->is_expanded(row);
	});
	return r;
}

void Panel::set_options(const string &_id, const string &options) {
	if (id == "toolbar[0]" and win == this) {
		if (auto t = win->get_toolbar(0))
			t->set_options(options);
	} else if (_id != "") {
		apply_foreach(_id, [&options](Control *c) {
			c->set_options(options);
		});
	} else if (win == this) {
		win->__set_options(options);
	}
}

string panel_scope(Panel *p) {
	if (!p)
		return "";
	if (p == p->win)
		return "win.";
	return p->id + ".";
}

#if GTK_CHECK_VERSION(4,0,0)
void Panel::_try_add_action_(const string &id, bool as_checkable) {
	if ((id == "") or (id == "*") or (id == "hui:close"))
		return;
	string name = get_gtk_action_name(id, nullptr);
	//msg_write(this->id + ":" + id + "   (...)  ");
	auto aa = g_action_map_lookup_action(G_ACTION_MAP(action_group), name.c_str());
	if (aa)
		return;
	if (as_checkable) {
		//msg_write("ACTION C   " + panel_scope(this) + id);
		GVariant *state = g_variant_new_boolean(FALSE);
		auto a = g_simple_action_new_stateful(name.c_str(), /*G_VARIANT_TYPE_BOOLEAN*/ nullptr, state);
		//auto a = g_simple_action_new_stateful(name.c_str(), G_VARIANT_TYPE_BOOLEAN, state);
		g_signal_connect(G_OBJECT(a), "activate", G_CALLBACK(_on_menu_action_), this);
		g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(a));
	} else {
		//msg_write("ACTION     " + panel_scope(this) + id + "   " + get_gtk_action_name(id, this));
		auto a = g_simple_action_new(name.c_str(), nullptr);
		g_signal_connect(G_OBJECT(a), "activate", G_CALLBACK(_on_menu_action_), this);
		g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(a));
	}
}


GAction *Panel::_get_action(const string &id) {
	if (action_group)
		return g_action_map_lookup_action(G_ACTION_MAP(action_group), get_gtk_action_name(id, nullptr).c_str());
	return nullptr;
}

#endif

void Panel::_connect_menu_to_panel(Menu *menu) {
	menu->set_panel(this);

#if GTK_CHECK_VERSION(4,0,0)
	for (auto c: menu->get_all_controls()) {
		if (c->type == MENU_ITEM_TOGGLE) {
			_try_add_action_(c->id, true);
			if (auto i = static_cast<MenuItemToggle*>(c)) {
				if (i->checked)
					i->__check(true);
			}
		} else if (c->type == MENU_ITEM_BUTTON) {
			_try_add_action_(c->id, false);
		} else {
			//_try_add_action_(c->id, false);
		}
		if (!c->enabled) {
			c->enable(false); // update action...
			//enable(c->id, false);
		}
	}
#endif
}

string decode_gtk_action(const string &name) {
	return name.sub_ref(10).unhex();
}

#if GTK_CHECK_VERSION(4,0,0)
void Panel::_on_menu_action_(GSimpleAction *simple, GVariant *parameter, gpointer user_data) {
	auto panel = static_cast<Panel*>(user_data);
	//auto win = panel->win;
	//string id = g_variant_get_string(parameter, nullptr);

	GValue value = {0,};
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(simple), "name", &value);
	string id = string(g_value_get_string(&value));
	id = decode_gtk_action(id);
	//msg_write("ACTION CALLBACK " + id);
	g_value_unset(&value);

	panel->_set_cur_id_(id);
	if (id.num == 0)
		return;
	//notify_push(this);
	Event e = Event(id, EventID::CLICK);
	panel->_send_event_(&e);
}
#endif

};

