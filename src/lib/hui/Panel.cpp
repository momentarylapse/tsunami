/*
 * HuiPanel.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "Controls/Control.h"
#include "hui.h"
#include "internal.h"

namespace hui
{

// for unique window identifiers
static int current_uid = 0;

Panel::Panel() {
	win = nullptr;
	parent = nullptr;
	border_width = 5;
	id = "";
	num_float_decimals = 3;
	root_control = nullptr;
	plugable = nullptr;
	current_event_listener_uid = 0;

	unique_id = current_uid ++;

	set_target("");
}

Panel::~Panel() {
	_ClearPanel_();
}

void Panel::__init__() {
	new(this) Panel;
}

void Panel::__delete__() {
	this->Panel::~Panel();
}

void DBDEL(const string &type, const string &id, void *p) {
	//msg_write("<del " + type + " " + id + " " + p2s(p) + ">");
	//msg_right();
}

void DBDEL_DONE() {
	//msg_left();
	//msg_write("</>");
}

// might be executed repeatedly
void Panel::_ClearPanel_() {
	DBDEL("panel", id, this);
	event_listeners.clear();
	if (parent) {
		// disconnect
		for (int i=0; i<parent->children.num; i++)
			if (parent->children[i] == this) {
				parent->children.erase(i);
			}
		parent = nullptr;
	}
	while (children.num > 0) {
		Panel *p = children.pop();
		delete(p);
	}

	if (root_control)
		delete root_control;
	root_control = nullptr;

	id.clear();
	cur_id.clear();
	DBDEL_DONE();
}

void Panel::set_border_width(int width) {
	border_width = width;
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
	return event_x(id, ":def:", function);
}

int Panel::event_x(const string &id, const string &msg, const Callback &function) {
	int uid = current_event_listener_uid ++;
	event_listeners.add(EventListener(uid, id, msg, function));
	return uid;
}

// hopefully deprecated soon?
int Panel::event_xp(const string &id, const string &msg, const CallbackP &function) {
	int uid = current_event_listener_uid ++;
	event_listeners.add(EventListener(uid, id, msg, -1, function));
	return uid;
}

void Panel::remove_event_handler(int event_handler_id) {
	for (int i=event_listeners.num-1; i>=0; i--)
		if (event_listeners[i].uid == event_handler_id)
			event_listeners.erase(i);
}

void Panel::set_key_code(const string &id, int key_code, const string &image) {
	// make sure, each id has only 1 code
	//   (multiple ids may have the same code)
	for (auto &e: event_key_codes)
		if (e.id == id) {
			e.key_code = key_code;
			return;
		}
	event_key_codes.add(EventKeyCode(id, "", key_code));
}

bool Panel::_send_event_(Event *e, bool force_if_not_allowed) {
	if (!win)
		return false;
	if (!win->allow_input and !force_if_not_allowed)
		return false;

	CurWindow = win;
	e->win = win;
	e->mx = win->input.x;
	e->my = win->input.y;
	e->dx = win->input.dx;
	e->dy = win->input.dy;
	e->scroll_x = win->input.scroll_x;
	e->scroll_y = win->input.scroll_y;
	e->pressure = win->input.pressure;
	e->lbut = win->input.lb;
	e->mbut = win->input.mb;
	e->rbut = win->input.rb;
	e->key_code = win->input.key_code;
	e->key = (e->key_code % 256);
	e->text = GetKeyChar(e->key_code);
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

		if (e->message == "hui:draw") {
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

		// window closed by callback?
		if (win->got_destroyed())
			return sent;
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
	else if (type == "Expander")
		add_expander(title, x, y, id);
	else if (type == "Scroller")
		add_scroller(title, x, y, id);
	else if (type == "Paned")
		add_paned(title, x, y, id);
	else if (type == "Separator")
		add_separator(title, x, y, id);
	else if (type == "Revealer")
		add_revealer(title, x, y, id);
	else if (type == "MenuButton")
		add_menu_button(title, x, y, id);
	else
		msg_error("unknown hui control: " + type);
}

void Panel::_add_control(const string &ns, Resource &cmd, const string &parent_id) {
	//msg_write(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
	set_target(parent_id);
	add_control(cmd.type, GetLanguageR(ns, cmd),
				cmd.x, cmd.y,
				cmd.id);

	for (string &o: cmd.options)
		set_options(cmd.id, o);

	enable(cmd.id, cmd.enabled());
	if (cmd.has("hidden"))
		hide_control(cmd.id, true);

	if (cmd.image().num > 0)
		set_image(cmd.id, cmd.image());


	string tooltip = GetLanguageT(ns, cmd.id, cmd.tooltip);
	if (tooltip.num > 0)
		set_tooltip(cmd.id, tooltip);

	for (Resource &c: cmd.children)
		_add_control(ns, c, cmd.id);
}

void Panel::from_resource(const string &id) {
	Resource *res = GetResource(id);
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
		win->set_title(GetLanguage(res->id, res->id));

		// size
		int width = res->value("width", "0")._int();
		int height = res->value("height", "0")._int();
		if (width + height > 0)
			win->set_size(width, height);

		// menu/toolbar?
		string toolbar = res->value("toolbar");
		string menu = res->value("menu");
		if (menu != "")
			win->set_menu(CreateResourceMenu(menu));
		if (toolbar != "")
			win->toolbar[TOOLBAR_TOP]->set_by_id(toolbar);
	}

	id = res->id;

	int bw = res->value("borderwidth", "-1")._int();
	if (bw >= 0)
		set_border_width(bw);


	// controls
	if (res_is_window) {
		for (Resource &cmd: res->children)
			_add_control(id, cmd, "");
	} else {
		embed_resource(*res, "", 0, 0);
	}
}

void Panel::from_source(const string &buffer) {
	Resource res = ParseResource(buffer);
	set_from_resource(&res);
}


void Panel::embed_resource(Resource &c, const string &parent_id, int x, int y) {
	_embed_resource(c.id, c, parent_id, x, y);
}

void Panel::_embed_resource(const string &ns, Resource &c, const string &parent_id, int x, int y) {
	//_addControl(main_id, c, parent_id);

	set_target(parent_id);
	string title = GetLanguageR(ns, c);
	//if (c.options.num > 0)
	//	title = "!" + implode(c.options, ",") + "\\" + title;
	add_control(c.type, title, x, y, c.id);
	for (string &o: c.options)
		set_options(c.id, o);

	enable(c.id, c.enabled());
	if (c.image().num > 0)
		set_image(c.id, c.image());

	string tooltip = GetLanguageT(ns, c.id, c.tooltip);
	if (tooltip.num > 0)
		set_tooltip(c.id, tooltip);

	for (Resource &child : c.children)
		_embed_resource(ns, child, c.id, child.x, child.y);
}

void Panel::embed_source(const string &buffer, const string &parent_id, int x, int y) {
	Resource res = ParseResource(buffer);
	embed_resource(res, parent_id, x, y);
}

void Panel::embed(Panel *panel, const string &parent_id, int x, int y) {
	if (!panel)
		return;
	if (!panel->root_control) {
		msg_error("trying to embed an empty panel");
		return;
	}
	panel->parent = this;
	panel->set_win(win);
	children.add(panel);

	Panel* orig = panel->root_control->panel;

	set_target(parent_id);
	_insert_control_(panel->root_control, x, y);
//	if (cur_control) // don't really add... (stop some information propagation between Panels)
//		cur_control->children.pop();    ...no...now checked in apply_foreach()
	panel->root_control->panel = orig;//panel;
}

void Panel::set_win(Window *_win) {
	win = _win;
	for (Panel *p: children)
		p->set_win(win);
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
		if (win->get_menu())
			win->get_menu()->apply_foreach(id, f);
		/*if (win->popup)
			win->popup->apply_foreach(id, f);*/
		for (int i=0; i<4; i++)
			win->toolbar[i]->apply_foreach(id, f);
	}
}


// replace all the text
//    for all
void Panel::set_string(const string &_id, const string &str) {
	if (win and (id == _id))
		win->set_title(str);
	apply_foreach(_id, [=](Control *c) { c->set_string(str); });
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void Panel::set_int(const string &_id, int n) {
	apply_foreach(_id, [=](Control *c) { c->set_int(n); });
}

// replace all the text with a float
//    for all
void Panel::set_float(const string &_id, float f) {
	apply_foreach(_id, [=](Control *c) { c->set_float(f); });
}

void Panel::set_image(const string &_id, const string &image) {
	apply_foreach(_id, [=](Control *c) { c->set_image(image); });
}

void Panel::set_tooltip(const string &_id, const string &tip) {
	apply_foreach(_id, [=](Control *c) { c->set_tooltip(tip); });
}


// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void Panel::add_string(const string &_id, const string &str) {
	apply_foreach(_id, [=](Control *c) { c->add_string(str); });
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void Panel::add_child_string(const string &_id, int parent_row, const string &str) {
	apply_foreach(_id, [=](Control *c) { c->add_child_string(parent_row, str); });
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::change_string(const string &_id, int row, const string &str) {
	apply_foreach(_id, [=](Control *c) { c->change_string(row, str); });
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::remove_string(const string &_id, int row) {
	apply_foreach(_id, [=](Control *c) { c->remove_string(row); });
}

// listview / treeview
string Panel::get_cell(const string &_id, int row, int column) {
	string r = "";
	apply_foreach(_id, [&](Control *c) { r = c->get_cell(row, column); });
	return r;
}

// listview / treeview
void Panel::set_cell(const string &_id, int row, int column, const string &str) {
	apply_foreach(_id, [=](Control *c) { c->set_cell(row, column, str); });
}

void Panel::set_color(const string &_id, const color &col) {
	apply_foreach(_id, [=](Control *c) { c->set_color(col); });
}

// retrieve the text
//    for edit
string Panel::get_string(const string &_id) {
	string r = "";
	apply_foreach(_id, [&](Control *c) { r = c->get_string(); });
	return r;
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int Panel::get_int(const string &_id) {
	int r = 0;
	apply_foreach(_id, [&](Control *c) { r = c->get_int(); });
	return r;
}

// retrieve the text as a numerical value (float)
//    for edit
float Panel::get_float(const string &_id) {
	float r = 0;
	apply_foreach(_id, [&](Control *c) { r = c->get_float(); });
	return r;
}

color Panel::get_color(const string &_id) {
	color r = Black;
	apply_foreach(_id, [&](Control *c) { r = c->get_color(); });
	return r;
}

// switch control to usable/unusable
//    for all
void Panel::enable(const string &_id,bool enabled) {
	apply_foreach(_id, [=](Control *c) { c->enable(enabled); });
}

// show/hide control
//    for all
void Panel::hide_control(const string &_id,bool hide) {
	apply_foreach(_id, [=](Control *c) { c->hide(hide); });
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void Panel::check(const string &_id,bool checked) {
	apply_foreach(_id, [=](Control *c) { c->check(checked); });
}

// is marked as "checked"?
//    for CheckBox
bool Panel::is_checked(const string &_id) {
	bool r = false;
	apply_foreach(_id, [&](Control *c) { r = c->is_checked(); });
	return r;
}

// which lines are selected?
//    for ListView
Array<int> Panel::get_selection(const string &_id) {
	Array<int> r;
	apply_foreach(_id, [&](Control *c) { r = c->get_selection(); });
	return r;
}

void Panel::set_selection(const string &_id, const Array<int> &sel) {
	apply_foreach(_id, [=](Control *c) { c->set_selection(sel); });
}

// delete all the content
//    for ComboBox, ListView
void Panel::reset(const string &_id) {
	apply_foreach(_id, [=](Control *c) { c->reset(); });
}

// expand a single row
//    for TreeView
void Panel::expand(const string &_id, int row, bool expand) {
	apply_foreach(_id, [=](Control *c) { c->expand(row, expand); });
}

// expand all rows
//    for TreeView
void Panel::expand_all(const string &_id, bool expand) {
	apply_foreach(_id, [=](Control *c) { c->expand_all(expand); });
}

// is column in tree expanded?
//    for TreeView
bool Panel::is_expanded(const string &_id, int row) {
	bool r = false;
//	apply_foreach(_id, [&](Control *c) { r = c->isExpanded(); });
	return r;
}

//    for Revealer
void Panel::reveal(const string &_id, bool reveal) {
	apply_foreach(_id, [=](Control *c) { c->reveal(reveal); });
}

//    for Revealer
bool Panel::is_revealed(const string &_id) {
	bool r = false;
	apply_foreach(_id, [&](Control *c) { r = c->is_revealed(); });
	return r;
}

void Panel::delete_control(const string &_id) {
	apply_foreach(_id, [=](Control *c) { delete c; });
}

void Panel::set_options(const string &_id, const string &options) {
	if (id == "toolbar[0]" and win == this) {
		win->toolbar[0]->set_options(options);
	} else if (_id != "") {
		apply_foreach(_id, [=](Control *c) { c->set_options(options); });
	} else if (win == this) {
		win->__set_options(options);
	}
}

};

