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

Panel::Panel()
{
	win = NULL;
	parent = NULL;
	border_width = 5;
	id = "";
	num_float_decimals = 3;
	root_control = NULL;
	plugable = NULL;
	current_event_listener_uid = 0;

	unique_id = current_uid ++;

	setTarget("");
}

Panel::~Panel()
{
	_ClearPanel_();
}

void Panel::__init__()
{
	new(this) Panel;
}

void Panel::__delete__()
{
	this->Panel::~Panel();
}

// might be executed repeatedly
void Panel::_ClearPanel_()
{
	if (parent){
		// disconnect
		for (int i=0; i<parent->children.num; i++)
			if (parent->children[i] == this)
				parent->children.erase(i);
		parent = NULL;
	}
	while (children.num > 0){
		Panel *p = children[0];
		children.erase(0);
		delete(p);
	}

	while (controls.num > 0){
		Control *c = controls[0];
		controls.erase(0);
		delete(c);
	}
	id.clear();
	cur_id.clear();
	event_listeners.clear();
}

void Panel::setBorderWidth(int width)
{
	border_width = width;
}

void Panel::setDecimals(int decimals)
{
	num_float_decimals = decimals;
}

string Panel::_get_cur_id_()
{
	return cur_id;
}

void Panel::_set_cur_id_(const string &id)
{
	if (win)
		win->cur_id = id;
	cur_id = id;
}

int Panel::event(const string &id, const Callback &function)
{
	return eventX(id, ":def:", function);
}

int Panel::eventX(const string &id, const string &msg, const Callback &function)
{
	int uid = current_event_listener_uid ++;
	event_listeners.add(EventListener(uid, id, msg, function));
	return uid;
}

// hopefully deprecated soon?
int Panel::eventXP(const string &id, const string &msg, const CallbackP &function)
{
	int uid = current_event_listener_uid ++;
	event_listeners.add(EventListener(uid, id, msg, -1, function));
	return uid;
}

void Panel::removeEventHandler(int event_handler_id)
{
	for (int i=event_listeners.num-1; i>=0; i--)
		if (event_listeners[i].uid == event_handler_id)
			event_listeners.erase(i);
}

void Panel::setKeyCode(const string &id, int key_code, const string &image)
{
	event_key_codes.add(EventKeyCode(id, "", key_code));
}

int Panel::_kaba_event(const string &id, kaba_member_callback *function)
{
	return event(id, std::bind(function, this));
}

int Panel::_kaba_eventO(const string &id, EventHandler* handler, kaba_member_callback *function)
{
	return event(id, std::bind(function, handler));
}

int Panel::_kaba_eventX(const string &id, const string &msg, kaba_member_callback *function)
{
	return _kaba_eventOX(id, msg, this, function);
}

int Panel::_kaba_eventOX(const string &id, const string &msg, EventHandler* handler, kaba_member_callback *function)
{
	if (msg == "hui:draw"){
		kaba_member_callback_p *f = (kaba_member_callback_p*)function;
		return eventXP(id, msg, std::bind(f, handler, std::placeholders::_1));
	}else{
		return eventX(id, msg, std::bind(function, handler));
	}
}

bool Panel::_send_event_(Event *e, bool force_if_not_allowed)
{
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
	e->lbut = win->input.lb;
	e->mbut = win->input.mb;
	e->rbut = win->input.rb;
	e->key_code = win->input.key_code;
	e->key = (e->key_code % 256);
	e->text = GetKeyChar(e->key_code);
	e->row = win->input.row;
	e->row_target = win->input.row_target;
	e->column = win->input.column;
	_hui_event_ = *e;
	if (e->id.num > 0)
		_set_cur_id_(e->id);
	else
		_set_cur_id_(e->message);

	bool sent = false;

	for (auto &ee: event_listeners){
		if (!e->match(ee.id, ee.message))
			continue;

		// send the event

		if (e->message == "hui:draw"){
			if (ee.function_p){
				Painter p(this, e->id);
				ee.function_p(&p);
				sent = true;
			}
		}else{
			if (ee.function){
				ee.function();
				sent = true;
			}
		}

		// window closed by callback?
		if (win->gotDestroyed())
			return sent;
	}

	// reset
	win->input.dx = 0;
	win->input.dy = 0;
	win->input.scroll_x = 0;
	win->input.scroll_y = 0;

	return sent;
}

int Panel::_get_unique_id_()
{
	return unique_id;
}

void Panel::show()
{
	if (this == win)
		win->show();
	else if (root_control)
		root_control->hide(false);
	onShow();
}

void Panel::hide()
{
	if (this == win)
		win->hide();
	else if (root_control)
		root_control->hide(true);
	onHide();
}

//----------------------------------------------------------------------------------
// easy window creation functions


void Panel::addControl(const string &type, const string &title, int x, int y, const string &id)
{
	//printf("HuiPanelAddControl %s  %s  %d  %d  %s\n", type.c_str(), title.c_str(), x, y, id.c_str());
	if (type == "Button")
		addButton(title, x, y, id);
	else if (type == "ColorButton")
		addColorButton(title, x, y, id);
	else if (type == "DefButton")
		addDefButton(title, x, y, id);
	else if ((type == "Label") or (type == "Text"))
		addLabel(title, x, y, id);
	else if (type == "Edit")
		addEdit(title, x, y, id);
	else if (type == "MultilineEdit")
		addMultilineEdit(title, x, y, id);
	else if (type == "Group")
		addGroup(title, x, y, id);
	else if (type == "CheckBox")
		addCheckBox(title, x, y, id);
	else if (type == "ComboBox")
		addComboBox(title, x, y, id);
	else if (type == "TabControl")
		addTabControl(title, x, y, id);
	else if (type == "ListView")
		addListView(title, x, y, id);
	else if (type == "TreeView")
		addTreeView(title, x, y, id);
	else if (type == "IconView")
		addIconView(title, x, y, id);
	else if (type == "ProgressBar")
		addProgressBar(title, x, y, id);
	else if (type == "Slider")
		addSlider(title, x, y, id);
	else if (type == "Image")
		addImage(title, x, y, id);
	else if (type == "DrawingArea")
		addDrawingArea(title, x, y, id);
	else if ((type == "ControlTable") or (type == "Grid"))
		addGrid(title, x, y, id);
	else if (type == "SpinButton")
		addSpinButton(title, x, y, id);
	else if (type == "RadioButton")
		addRadioButton(title, x, y, id);
	else if (type == "ToggleButton")
		addToggleButton(title, x, y, id);
	else if (type == "Expander")
		addExpander(title, x, y, id);
	else if (type == "Scroller")
		addScroller(title, x, y, id);
	else if (type == "Paned")
		addPaned(title, x, y, id);
	else if (type == "Separator")
		addSeparator(title, x, y, id);
	else if (type == "Revealer")
		addRevealer(title, x, y, id);
	else
		msg_error("unknown hui control: " + type);
}

void Panel::_addControl(const string &ns, Resource &cmd, const string &parent_id)
{
	//msg_write(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
	setTarget(parent_id);
	addControl(cmd.type, GetLanguageR(ns, cmd),
				cmd.x, cmd.y,
				cmd.id);

	for (string &o: cmd.options)
		setOptions(cmd.id, o);

	enable(cmd.id, cmd.enabled());
	if (cmd.has("hidden"))
		hideControl(cmd.id, true);

	if (cmd.image().num > 0)
		setImage(cmd.id, cmd.image());


	string tooltip = GetLanguageT(ns, cmd.id, cmd.tooltip);
	if (tooltip.num > 0)
		setTooltip(cmd.id, tooltip);

	for (Resource &c: cmd.children)
		_addControl(ns, c, cmd.id);
}

void Panel::fromResource(const string &id)
{
	Resource *res = GetResource(id);
	if (res)
		setFromResource(res);
}

void Panel::setFromResource(Resource *res)
{
	if (!res)
		return;

	bool res_is_window = ((res->type == "Dialog") or (res->type == "Window"));
	bool panel_is_window = win and !parent;

	// directly change window?
	if (panel_is_window and res_is_window){
		// title
		win->setTitle(GetLanguage(id, res->id));

		// size
		int width = res->value("width", "0")._int();
		int height = res->value("height", "0")._int();
		if (width + height > 0)
			win->setSize(width, height);

		// menu/toolbar?
		string toolbar = res->value("toolbar");
		string menu = res->value("menu");
		if (menu != "")
			win->setMenu(CreateResourceMenu(menu));
		if (toolbar != "")
			win->toolbar[TOOLBAR_TOP]->setByID(toolbar);
	}

	id = res->id;

	int bw = res->value("borderwidth", "-1")._int();
	if (bw >= 0)
		setBorderWidth(bw);


	// controls
	if (res_is_window){
		for (Resource &cmd: res->children)
			_addControl(id, cmd, "");
	}else{
		embedResource(*res, "", 0, 0);
	}
}

void Panel::fromSource(const string &buffer)
{
	Resource res = ParseResource(buffer);
	setFromResource(&res);
}


void Panel::embedResource(Resource &c, const string &parent_id, int x, int y)
{
	_embedResource(c.id, c, parent_id, x, y);
}

void Panel::_embedResource(const string &ns, Resource &c, const string &parent_id, int x, int y)
{
	//_addControl(main_id, c, parent_id);

	setTarget(parent_id);
	string title = GetLanguageR(ns, c);
	//if (c.options.num > 0)
	//	title = "!" + implode(c.options, ",") + "\\" + title;
	addControl(c.type, title, x, y, c.id);
	for (string &o: c.options)
		setOptions(c.id, o);

	enable(c.id, c.enabled());
	if (c.image().num > 0)
		setImage(c.id, c.image());

	string tooltip = GetLanguageT(ns, c.id, c.tooltip);
	if (tooltip.num > 0)
		setTooltip(c.id, tooltip);

	for (Resource &child : c.children)
		_embedResource(ns, child, c.id, child.x, child.y);
}

void Panel::embedSource(const string &buffer, const string &parent_id, int x, int y)
{
	Resource res = ParseResource(buffer);
	embedResource(res, parent_id, x, y);
}

void Panel::embed(Panel *panel, const string &parent_id, int x, int y)
{
	if (!panel->root_control){
		msg_error("trying to embed an empty panel");
		return;
	}
	panel->parent = this;
	panel->set_win(win);
	children.add(panel);

	Panel* orig = panel->root_control->panel;

	setTarget(parent_id);
	_insert_control_(panel->root_control, x, y);
	controls.pop(); // dont' really add to us
	panel->root_control->panel = orig;//panel;
}

void Panel::set_win(Window *_win)
{
	win = _win;
	for (Panel *p: children)
		p->set_win(win);
}


//----------------------------------------------------------------------------------
// data exchanging functions for control items


#define test_controls(_id, c)	\
	string tid = (_id.num == 0) ? cur_id : _id; \
	for (Control *c: controls) \
		if (c->id == tid)

// replace all the text
//    for all
void Panel::setString(const string &_id, const string &str)
{
	if (win and (id == _id))
		win->setTitle(str);
	test_controls(_id, c)
		c->setString(str);
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void Panel::setInt(const string &_id, int n)
{
	test_controls(_id, c)
		c->setInt(n);
}

// replace all the text with a float
//    for all
void Panel::setFloat(const string &_id, float f)
{
	test_controls(_id, c)
		c->setFloat(f);
}

void Panel::setImage(const string &_id, const string &image)
{
	test_controls(_id, c)
		c->setImage(image);
}

void Panel::setTooltip(const string &_id, const string &tip)
{
	test_controls(_id, c)
		c->setTooltip(tip);
}


// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void Panel::addString(const string &_id, const string &str)
{
	test_controls(_id, c)
		c->addString(str);
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void Panel::addChildString(const string &_id, int parent_row, const string &str)
{
	test_controls(_id, c)
		c->addChildString(parent_row, str);
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::changeString(const string &_id, int row, const string &str)
{
	test_controls(_id, c)
		c->changeString(row, str);
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void Panel::removeString(const string &_id, int row)
{
	test_controls(_id, c)
		c->removeString(row);
}

// listview / treeview
string Panel::getCell(const string &_id, int row, int column)
{
	test_controls(_id, c)
		return c->getCell(row, column);
	return "";
}

// listview / treeview
void Panel::setCell(const string &_id, int row, int column, const string &str)
{
	test_controls(_id, c)
		c->setCell(row, column, str);
}

void Panel::setColor(const string &_id, const color &col)
{
	test_controls(_id, c)
		c->setColor(col);
}

// retrieve the text
//    for edit
string Panel::getString(const string &_id)
{
	test_controls(_id, c)
		return c->getString();
	return "";
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int Panel::getInt(const string &_id)
{
	test_controls(_id, c)
		return c->getInt();
	return 0;
}

// retrieve the text as a numerical value (float)
//    for edit
float Panel::getFloat(const string &_id)
{
	test_controls(_id, c)
		return c->getFloat();
	return 0;
}

color Panel::getColor(const string &_id)
{
	test_controls(_id, c)
		return c->getColor();
	return Black;
}

// switch control to usable/unusable
//    for all
void Panel::enable(const string &_id,bool enabled)
{
	test_controls(_id, c)
		c->enable(enabled);
}

// show/hide control
//    for all
void Panel::hideControl(const string &_id,bool hide)
{
	test_controls(_id, c)
		c->hide(hide);
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void Panel::check(const string &_id,bool checked)
{
	test_controls(_id, c)
		c->check(checked);
}

// is marked as "checked"?
//    for CheckBox
bool Panel::isChecked(const string &_id)
{
	test_controls(_id, c)
		return c->isChecked();
	return false;
}

// which lines are selected?
//    for ListView
Array<int> Panel::getSelection(const string &_id)
{
	test_controls(_id, c)
		return c->getSelection();
	Array<int> sel;
	return sel;
}

void Panel::setSelection(const string &_id, const Array<int> &sel)
{
	test_controls(_id, c)
		c->setSelection(sel);
}

// delete all the content
//    for ComboBox, ListView
void Panel::reset(const string &_id)
{
	test_controls(_id, c)
		c->reset();
}

// expand a single row
//    for TreeView
void Panel::expand(const string &_id, int row, bool expand)
{
	test_controls(_id, c)
		c->expand(row, expand);
}

// expand all rows
//    for TreeView
void Panel::expandAll(const string &_id, bool expand)
{
	test_controls(_id, c)
		c->expandAll(expand);
}

// is column in tree expanded?
//    for TreeView
bool Panel::isExpanded(const string &_id, int row)
{
	test_controls(_id, c)
		return false;
	return false;
}

//    for Revealer
void Panel::reveal(const string &_id, bool reveal)
{
	test_controls(_id, c)
		c->reveal(reveal);
}

//    for Revealer
bool Panel::isRevealed(const string &_id)
{
	test_controls(_id, c)
		return c->isRevealed();
	return false;
}

void Panel::deleteControl(const string &_id)
{
	for(int i=controls.num-1;i>=0;i--)
		if (controls[i]->id == _id)
			delete(controls[i]);
}

void Panel::setOptions(const string &_id, const string &options)
{
	test_controls(_id, c)
		c->setOptions(options);
}

};

