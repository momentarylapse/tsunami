/*
 * HuiPanel.cpp
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControl.h"

// for unique window identifiers
static int current_uid = 0;

HuiPanel::HuiPanel()
{
	win = NULL;
	parent = NULL;
	border_width = 5;
	expander_indent = 20;
	id = "";
	num_float_decimals = 3;
	tab_creation_page = -1;
	root_control = NULL;
	is_resizable = true;
	plugable = NULL;

	unique_id = current_uid ++;

	SetTarget("", 0);
}

HuiPanel::~HuiPanel()
{
	_ClearPanel_();
}

void HuiPanel::__init__()
{
	new(this) HuiPanel;
}

void HuiPanel::__delete__()
{
	_ClearPanel_();
}

void HuiPanel::_ClearPanel_()
{
	HuiClosedPanel c;
	c.unique_id = unique_id;
	c.panel = this;
	c.last_id = cur_id;
	HuiClosedPanels.add(c);

	if (parent){
		// disconnect
		for (int i=0; i<parent->children.num; i++)
			if (parent->children[i] == this)
				parent->children.erase(i);
		parent = NULL;
	}
	while (children.num > 0){
		HuiPanel *p = children[0];
		children.erase(0);
		delete(p);
	}

	while (control.num > 0){
		HuiControl *c = control[0];
		control.erase(0);
		delete(c);
	}
	id.clear();
	cur_id.clear();
	event.clear();
}

void HuiPanel::SetBorderWidth(int width)
{
	border_width = width;
}

void HuiPanel::SetIndent(int indent)
{
	expander_indent = indent;
}

void HuiPanel::SetDecimals(int decimals)
{
	num_float_decimals = decimals;
}

string HuiPanel::_GetCurID_()
{
	return cur_id;
}

void HuiPanel::_SetCurID_(const string &id)
{
	cur_id = id;
}

void HuiPanel::Event(const string &id, hui_callback *function)
{
	event.add(HuiEventListener(id, "*", function));

}

void HuiPanel::EventX(const string &id, const string &msg, hui_callback *function)
{
	event.add(HuiEventListener(id, msg, function));
}

void HuiPanel::_EventM(const string &id, HuiEventHandler *handler, void (HuiEventHandler::*function)())
{
	event.add(HuiEventListener(id, ":def:", HuiCallback(handler, function)));
}

void HuiPanel::_EventMX(const string &id, const string &msg, HuiEventHandler *handler, void (HuiEventHandler::*function)())
{
	event.add(HuiEventListener(id, msg, HuiCallback(handler, function)));
}

void HuiPanel::_EventKM(const string &id, HuiEventHandler* handler, hui_kaba_callback *function)
{
	event.add(HuiEventListener(id, ":def:", HuiCallback(handler, function)));
}

void HuiPanel::_EventKMX(const string &id, const string &msg, HuiEventHandler* handler, hui_kaba_callback *function)
{
	event.add(HuiEventListener(id, msg, HuiCallback(handler, function)));
}

void HuiPanel::RemoveEventHandlers(HuiEventHandler *handler)
{
	for (int i=event.num-1;i>=0;i--)
		if (event[i].function.has_handler(handler))
			event.erase(i);
}

bool HuiPanel::_SendEvent_(HuiEvent *e)
{
	if (!win)
		return false;
	if (!win->allow_input)
		return false;
	msg_db_f("SendEvent", 2);
	//msg_write(e->id);
	//msg_write(e->message);
	HuiCurWindow = win;
	e->win = win;
	e->mx = win->input.x;
	e->my = win->input.y;
	e->dx = win->input.dx;
	e->dy = win->input.dy;
	e->dz = win->input.dz;
	e->lbut = win->input.lb;
	e->mbut = win->input.mb;
	e->rbut = win->input.rb;
	e->key_code = win->input.key_code;
	e->key = (e->key_code % 256);
	e->text = HuiGetKeyChar(e->key_code);
	e->row = win->input.row;
	e->column = win->input.column;
	_HuiEvent_ = *e;
	if (e->id.num > 0)
		_SetCurID_(e->id);
	else
		_SetCurID_(e->message);

	bool sent = false;
	foreach(HuiEventListener &ee, event){
		if (!_HuiEventMatch_(e, ee.id, ee.message))
			continue;

		// send the event
		if (ee.function.is_set()){
			ee.function.call();
			sent = true;
		}

		// window closed by callback?
		foreach(HuiClosedPanel &cp, HuiClosedPanels)
			if (cp.panel == this)
				return sent;
		_foreach_it_.update();
	}

	// reset
	win->input.dx = 0;
	win->input.dy = 0;
	win->input.dz = 0;

	return sent;
}

int HuiPanel::_GetUniqueID_()
{
	return unique_id;
}

void HuiPanel::Show()
{
	if (this == win)
		win->Show();
	else if (root_control)
		root_control->Hide(false);
	OnShow();
}

void HuiPanel::Hide()
{
	if (this == win)
		win->Hide();
	else if (root_control)
		root_control->Hide(true);
	OnHide();
}

//----------------------------------------------------------------------------------
// easy window creation functions


void HuiPanel::AddControl(const string &type, const string &title, int x, int y, int width, int height, const string &id)
{
	//msg_db_m(format("HuiPanelAddControl %s  %s  %d  %d  %d  %d  %d", type.c_str(), title.c_str(), x, y, width, height, id.c_str()).c_str(),2);
	if (type == "Button")
		AddButton(title, x, y, width, height, id);
	else if (type == "ColorButton")
		AddColorButton(title, x, y, width, height, id);
	else if (type == "DefButton")
		AddDefButton(title, x, y, width, height, id);
	else if (type == "Text")
		AddText(title, x, y, width, height, id);
	else if (type == "Edit")
		AddEdit(title, x, y, width, height, id);
	else if (type == "MultilineEdit")
		AddMultilineEdit(title, x, y, width, height, id);
	else if (type == "Group")
		AddGroup(title, x, y, width, height, id);
	else if (type == "CheckBox")
		AddCheckBox(title, x, y, width, height, id);
	else if (type == "ComboBox")
		AddComboBox(title, x, y, width, height, id);
	else if (type == "TabControl")
		AddTabControl(title, x, y, width, height, id);
	else if (type == "ListView")
		AddListView(title, x, y, width, height, id);
	else if (type == "TreeView")
		AddTreeView(title, x, y, width, height, id);
	else if (type == "IconView")
		AddIconView(title, x, y, width, height, id);
	else if (type == "ProgressBar")
		AddProgressBar(title, x, y, width, height, id);
	else if (type == "Slider")
		AddSlider(title, x, y, width, height, id);
	else if (type == "Image")
		AddImage(title, x, y, width, height, id);
	else if (type == "DrawingArea")
		AddDrawingArea(title, x, y, width, height, id);
	else if ((type == "ControlTable") || (type == "Grid"))
		AddControlTable(title, x, y, width, height, id);
	else if (type == "SpinButton")
		AddSpinButton(title, x, y, width, height, id);
	else if (type == "RadioButton")
		AddRadioButton(title, x, y, width, height, id);
	else if (type == "ToggleButton")
		AddToggleButton(title, x, y, width, height, id);
	else if (type == "Expander")
		AddExpander(title, x, y, width, height, id);
	else if (type == "Scroller")
		AddScroller(title, x, y, width, height, id);
	else if (type == "Paned")
		AddPaned(title, x, y, width, height, id);
	else if (type == "Separator")
		AddSeparator(title, x, y, width, height, id);
}

void HuiPanel::FromResource(const string &id)
{
	msg_db_f("Window.FromResource",1);
	HuiResource *res = HuiGetResource(id);
	if (!res)
		return;

	// title
	if (win)
		win->SetTitle(HuiGetLanguage(res->id));

	// size
	if (win)
		win->SetSize(res->i_param[0], res->i_param[1]);


	// dialog
	/*CHuiPanel *dlg
	if (res->type == "SizableDialog")
		dlg = HuiCreateSizableDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);
	else
		dlg = HuiCreateDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);*/

	// menu?
	if ((win) && (res->s_param[0].num > 0))
		win->SetMenu(HuiCreateResourceMenu(res->s_param[0]));

	// toolbar?
	if ((win) && (res->s_param[1].num > 0))
		win->toolbar[HuiToolbarTop]->SetByID(res->s_param[1]);

	// controls
	foreach(HuiResource &cmd, res->children){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			AddControl( cmd.type, HuiGetLanguage(cmd.id),
						cmd.i_param[0], cmd.i_param[1],
						cmd.i_param[2], cmd.i_param[3],
						cmd.id);
		}else if (res->type == "SizableDialog"){
			//msg_write("insert " + cmd.id + " (" + cmd.type + ") into " + cmd.s_param[0]);
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			AddControl( cmd.type, HuiGetLanguage(cmd.id),
						cmd.i_param[0], cmd.i_param[1],
						cmd.i_param[2], cmd.i_param[3],
						cmd.id);
		}
		Enable(cmd.id, cmd.enabled);
		if (cmd.image.num > 0)
			SetImage(cmd.id, cmd.image);
	}
	msg_db_m("  \\(^_^)/",1);
}

void HuiPanel::FromSource(const string &buffer)
{
	msg_db_f("FromSource",1);
	HuiResourceNew res;
	res.load(buffer);
	if (res.type == "Dialog"){
		if (win)
			win->SetSize(res.w, res.h);

		if (res.children.num > 0)
			EmbedResource(res.children[0], "", 0, 0);
	}else{
		EmbedResource(res, "", 0, 0);
	}

}


void HuiPanel::EmbedResource(HuiResourceNew &c, const string &parent_id, int x, int y)
{
	SetTarget(parent_id, x);
	string title = c.title;
	if (c.options.num > 0)
		title = "!" + implode(c.options, ",") + "\\" + title;
	AddControl(c.type, title, x, y, c.w, c.h, c.id);

	Enable(c.id, c.enabled);
	if (c.image.num > 0)
		SetImage(c.id, c.image);

	foreach(HuiResourceNew &child, c.children)
		EmbedResource(child, c.id, child.x, child.y);
}

void HuiPanel::EmbedSource(const string &buffer, const string &parent_id, int x, int y)
{
	HuiResourceNew res;
	res.load(buffer);
	EmbedResource(res, parent_id, x, y);
}

void HuiPanel::Embed(HuiPanel *panel, const string &parent_id, int x, int y)
{
	if (!panel->root_control){
		msg_error("trying to embed an empty panel");
		return;
	}
	panel->parent = this;
	panel->set_win(win);
	children.add(panel);

	SetTarget(parent_id, x);
	_InsertControl_(panel->root_control, x, y, 0, 0);
	control.pop(); // dont' really add to us
	panel->root_control->panel = panel;
}

void HuiPanel::set_win(HuiWindow *_win)
{
	win = _win;
	foreach(HuiPanel *p, children)
		p->set_win(win);
}


//----------------------------------------------------------------------------------
// data exchanging functions for control items


#define test_controls(_id, c)	\
	string tid = (_id.num == 0) ? cur_id : _id; \
	foreach(HuiControl *c, control) \
		if (c->id == tid)

// replace all the text
//    for all
void HuiPanel::SetString(const string &_id, const string &str)
{
	if (win && (id == _id))
		win->SetTitle(str);
	test_controls(_id, c)
		c->SetString(str);
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void HuiPanel::SetInt(const string &_id, int n)
{
	test_controls(_id, c)
		c->SetInt(n);
}

// replace all the text with a float
//    for all
void HuiPanel::SetFloat(const string &_id, float f)
{
	test_controls(_id, c)
		c->SetFloat(f);
}

void HuiPanel::SetImage(const string &_id, const string &image)
{
	test_controls(_id, c)
		c->SetImage(image);
}

void HuiPanel::SetTooltip(const string &_id, const string &tip)
{
	test_controls(_id, c)
		c->SetTooltip(tip);
}


// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void HuiPanel::AddString(const string &_id, const string &str)
{
	test_controls(_id, c)
		c->AddString(str);
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void HuiPanel::AddChildString(const string &_id, int parent_row, const string &str)
{
	test_controls(_id, c)
		c->AddChildString(parent_row, str);
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void HuiPanel::ChangeString(const string &_id,int row,const string &str)
{
	test_controls(_id, c)
		c->ChangeString(row, str);
}

// listview / treeview
string HuiPanel::GetCell(const string &_id, int row, int column)
{
	test_controls(_id, c)
		return c->GetCell(row, column);
	return "";
}

// listview / treeview
void HuiPanel::SetCell(const string &_id, int row, int column, const string &str)
{
	test_controls(_id, c)
		c->SetCell(row, column, str);
}

void HuiPanel::SetColor(const string &_id, const color &col)
{
	test_controls(_id, c)
		c->SetColor(col);
}

// retrieve the text
//    for edit
string HuiPanel::GetString(const string &_id)
{
	test_controls(_id, c)
		return c->GetString();
	return "";
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int HuiPanel::GetInt(const string &_id)
{
	test_controls(_id, c)
		return c->GetInt();
	return 0;
}

// retrieve the text as a numerical value (float)
//    for edit
float HuiPanel::GetFloat(const string &_id)
{
	test_controls(_id, c)
		return c->GetFloat();
	return 0;
}

color HuiPanel::GetColor(const string &_id)
{
	test_controls(_id, c)
		return c->GetColor();
	return Black;
}

// switch control to usable/unusable
//    for all
void HuiPanel::Enable(const string &_id,bool enabled)
{
	test_controls(_id, c)
		c->Enable(enabled);
}

// show/hide control
//    for all
void HuiPanel::HideControl(const string &_id,bool hide)
{
	test_controls(_id, c)
		c->Hide(hide);
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void HuiPanel::Check(const string &_id,bool checked)
{
	test_controls(_id, c)
		c->Check(checked);
}

// is marked as "checked"?
//    for CheckBox
bool HuiPanel::IsChecked(const string &_id)
{
	test_controls(_id, c)
		return c->IsChecked();
	return false;
}

// which lines are selected?
//    for ListView
Array<int> HuiPanel::GetMultiSelection(const string &_id)
{
	test_controls(_id, c)
		return c->GetMultiSelection();
	Array<int> sel;
	return sel;
}

void HuiPanel::SetMultiSelection(const string &_id, Array<int> &sel)
{
	test_controls(_id, c)
		c->SetMultiSelection(sel);
}

// delete all the content
//    for ComboBox, ListView
void HuiPanel::Reset(const string &_id)
{
	test_controls(_id, c)
		c->Reset();
}

// expand a single row
//    for TreeView
void HuiPanel::Expand(const string &_id, int row, bool expand)
{
	test_controls(_id, c)
		c->Expand(row, expand);
}

// expand all rows
//    for TreeView
void HuiPanel::ExpandAll(const string &_id, bool expand)
{
	test_controls(_id, c)
		c->ExpandAll(expand);
}

// is column in tree expanded?
//    for TreeView
bool HuiPanel::IsExpanded(const string &_id, int row)
{
	test_controls(_id, c)
		return false;
	return false;
}

void HuiPanel::DeleteControl(const string &_id)
{
	for(int i=control.num-1;i>=0;i--)
		if (control[i]->id == _id)
			delete(control[i]);
}

void HuiPanel::SetOptions(const string &_id, const string &options)
{
	test_controls(_id, c)
		c->SetOptions(options);
}

