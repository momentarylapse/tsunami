/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2010.07.14 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControl.h"
#include "HuiToolbar.h"


// for unique window identifiers
static int current_uid = 0;

extern int HuiMainLevel;

HuiWindow *HuiCurWindow = NULL;

// recursively find a menu item and execute message_function
/*bool TestMenuID(CHuiMenu *menu, const string &id, message_function *mf)
{
	if (!menu)
		return false;
	for (int i=0;i<menu->Item.num;i++){
		if (menu->Item[i].ID==id){
			mf(id);
			return true;
		}
		if (menu->Item[i].SubMenu)
			if (TestMenuID(menu->Item[i].SubMenu,id,mf))
				return true;
	}
	return false;
}*/

void add_key_to_buffer(HuiInputData *d, int key)
{
	// full -> remove the first key
	if (d->KeyBufferDepth >= HUI_MAX_KEYBUFFER_DEPTH - 1){
		for (int k=0;k<d->KeyBufferDepth-2;k++)
			d->KeyBuffer[k] = d->KeyBuffer[k+1];
		d->KeyBufferDepth --;
	}
	d->KeyBuffer[d->KeyBufferDepth ++] = key;
}


HuiWindow::HuiWindow(const string &title, int x, int y, int width, int height, HuiWindow *root, bool allow_root, int mode)
{
	_Init_(title, x, y, width, height, root, allow_root, mode);
}

HuiWindow::HuiWindow()
{
	_Init_("", -1, -1, 0, 0, NULL, true, HuiWinModeDummy);
}

HuiWindow::HuiWindow(const string &title, int x, int y, int width, int height)
{
	_Init_(title, x, y, width, height, NULL, true, HuiWinModeResizable | HuiWinModeControls);
}

void HuiWindow::__init_ext__(const string& title, int x, int y, int width, int height)
{
	msg_write("init ext");
	new(this) HuiWindow(title, x, y, width, height);
}


HuiWindow::HuiWindow(const string &id, HuiWindow *parent, bool allow_parent)
{
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("HuiWindow: undefined resource id: " + id);
	}

	int mode = HuiWinModeControls;
	if (res->type == "SizableDialog")
		mode = HuiWinModeControls | HuiWinModeResizable;
	_Init_(HuiGetLanguage(id), -1, -1, res->i_param[0], res->i_param[1], parent, allow_parent, mode);

	// menu?
	if (res->s_param[0].num > 0)
		SetMenu(HuiCreateResourceMenu(res->s_param[0]));

	// toolbar?
	if (res->s_param[1].num > 0)
		toolbar[HuiToolbarTop]->SetByID(res->s_param[1]);

	// controls
	foreach(HuiResourceCommand &cmd, res->cmd){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}else if (res->type == "SizableDialog"){
			//msg_write("insert " + cmd.id + " (" + cmd.type + ") into " + cmd.s_param[0]);
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
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

void HuiWindow::_InitGeneric_(HuiWindow *_root, bool _allow_root, int _mode)
{
	msg_db_r("Window::_InitGeneric_", 2);
	_HuiMakeUsable_();
	HuiWindows.add(this);

	_HuiClosedWindow_.clear();

	used_by_nix = false;
	is_resizable = ((_mode & HuiWinModeResizable) > 0);
	border_width = 5;
	allowed = true;
	allow_keys = true;
	parent = _root;
	terror_child = NULL;
	if (parent){
		parent->allowed = _allow_root;
		if (!parent->allowed)
			parent->terror_child = this;
		parent->sub_window.add(this);
	}
	menu = popup = NULL;
	statusbar_enabled = false;
	toolbar[HuiToolbarTop] = new HuiToolbar(this);
	toolbar[HuiToolbarLeft] = new HuiToolbar(this, true);
	toolbar[HuiToolbarRight] = new HuiToolbar(this, true);
	toolbar[HuiToolbarBottom] = new HuiToolbar(this);
	input.reset();
	tab_creation_page = -1;

	id = "";
	num_float_decimals = 3;
	unique_id = current_uid ++;
	allow_input = false; // allow only if ->Show() was called
	main_level = HuiMainLevel;

	SetTarget("", 0);

	msg_db_l(2);
}

void HuiWindow::_CleanUp_()
{
	msg_db_r("Window::_CleanUp_", 2);
	HuiClosedWindow c;
	c.unique_id = unique_id;
	c.win = this;
	c.last_id = cur_id;
	_HuiClosedWindow_.add(c);

	for (int i=0;i<4;i++)
		delete(toolbar[i]);

	for (int i=0;i<control.num;i++)
		delete(control[i]);
	
	// unregister window
	for (int i=0;i<HuiWindows.num;i++)
		if (HuiWindows[i] == this){
			HuiWindows.erase(i);
			break;
		}
	msg_db_l(2);
}


// identify window (for automatic title assignment with language strings)
void HuiWindow::SetID(const string &_id)
{
	id = _id;
	if ((HuiLanguaged) && (id.num > 0))
		SetTitle(HuiGetLanguage(id));
}

// align window relative to another window (like..."top right corner")
void HuiWindow::SetPositionSpecial(HuiWindow *win,int mode)
{
	irect rp=win->GetOuterior();
	irect ro=GetOuterior();
	int x=ro.x1,y=ro.y1;
	if ((mode & HuiLeft)>0)
		x=rp.x1 + 2;
	if ((mode & HuiRight)>0)
		x=rp.x2 - (ro.x2-ro.x1) -2;
	if ((mode & HuiTop)>0)
		y=rp.y1 + 20;
	if ((mode & HuiBottom)>0)
		y=rp.y2 - (ro.y2-ro.y1) -2;
	SetPosition(x,y);
}

void HuiWindow::SetBorderWidth(int width)
{
	border_width = width;
}

void HuiWindow::SetDecimals(int decimals)
{
	num_float_decimals = decimals;
}

int HuiWindow::_GetMainLevel_()
{
	return main_level;
}

int HuiWindow::_GetUniqueID_()
{
	return unique_id;
}

string HuiWindow::_GetCurID_()
{
	return cur_id;
}

void HuiWindow::_SetCurID_(const string &id)
{
	cur_id = id;
}

HuiMenu *HuiWindow::GetMenu()
{
	return menu;
}

HuiWindow *HuiWindow::GetParent()
{
	return parent;
}

bool HuiWindow::GetKey(int k)
{
	if (k == KEY_CONTROL)
		return (input.key[KEY_RCONTROL] || input.key[KEY_LCONTROL]);
	else if (k == KEY_SHIFT)
		return (input.key[KEY_RSHIFT] || input.key[KEY_LSHIFT]);
	else
		return input.key[k];
}

bool HuiWindow::GetMouse(int &x, int &y, int button)
{
	x = input.x;
	y = input.y;
	if (button == 0){
		return input.lb;
	}else if (button == 1){
		return input.mb;
	}else{
		return input.rb;
	}
}

void HuiWindow::Event(const string &id, hui_callback *function)
{
	HuiWinEvent e;
	e.id = id;
	e.message = "*";
	e.function = function;
	e.object = NULL;
	e.member_function = NULL;
	event.add(e);
	
}

void HuiWindow::EventX(const string &id, const string &msg, hui_callback *function)
{
	HuiWinEvent e;
	e.id = id;
	e.message = msg;
	e.function = function;
	e.object = NULL;
	e.member_function = NULL;
	event.add(e);
}

void HuiWindow::_EventM(const string &id, HuiEventHandler *handler, void (HuiEventHandler::*function)())
{
	HuiWinEvent e;
	e.id = id;
	e.message = ":def:";
	e.function = NULL;
	e.object = handler;
	e.member_function = function;
	event.add(e);
}

void HuiWindow::_EventMX(const string &id, const string &msg, HuiEventHandler *handler, void (HuiEventHandler::*function)())
{
	HuiWinEvent e;
	e.id = id;
	e.message = msg;
	e.function = NULL;
	e.object = handler;
	e.member_function = function;
	event.add(e);
}

bool HuiWindow::_SendEvent_(HuiEvent *e)
{
	if (!allow_input)
		return false;
	msg_db_r("SendEvent", 2);
	//msg_write(e->id);
	//msg_write(e->message);
	HuiCurWindow = this;
	e->win = this;
	if (e->id.num > 0){
		e->mx = input.area_x;
		e->my = input.area_y;
	}else{
		e->mx = input.x;
		e->my = input.y;
	}
	e->dx = input.dx;
	e->dy = input.dy;
	e->dz = input.dz;
	e->lbut = input.lb;
	e->mbut = input.mb;
	e->rbut = input.rb;
	e->key_code = input.key_code;
	e->key = (e->key_code % 256);
	e->text = HuiGetKeyChar(e->key_code);
	e->row = input.row;
	e->column = input.column;
	_HuiEvent_ = *e;
	if (e->id.num > 0)
		_SetCurID_(e->id);
	else
		_SetCurID_(e->message);

	bool sent = false;
	foreach(HuiWinEvent &ee, event){
		if (!_HuiEventMatch_(e, ee.id, ee.message))
			continue;
		
		// send the event
		if (ee.function){
			ee.function();
			sent = true;
		}else if ((ee.object) && (ee.member_function)){
			// send the event (member)
			(ee.object->*ee.member_function)();
			sent = true;
		}

		// window closed by callback?
		foreach(HuiClosedWindow &cw, _HuiClosedWindow_)
			if (cw.win == this){
				msg_db_l(2);
				return sent;
			}
	}

	// reset
	input.dx = 0;
	input.dy = 0;
	input.dz = 0;

	msg_db_l(2);
	return sent;
}


//----------------------------------------------------------------------------------
// easy window creation functions


void HuiWindowAddControl(HuiWindow *win, const string &type, const string &title, int x, int y, int width, int height, const string &id)
{
	//msg_db_m(format("HuiWindowAddControl %s  %s  %d  %d  %d  %d  %d", type.c_str(), title.c_str(), x, y, width, height, id.c_str()).c_str(),2);
	if (type == "Button")
		win->AddButton(title, x, y, width, height, id);
	else if (type == "ColorButton")
		win->AddColorButton(title, x, y, width, height, id);
	else if (type == "DefButton")
		win->AddDefButton(title, x, y, width, height, id);
	else if (type == "Text")
		win->AddText(title, x, y, width, height, id);
	else if (type == "Edit")
		win->AddEdit(title, x, y, width, height, id);
	else if (type == "MultilineEdit")
		win->AddMultilineEdit(title, x, y, width, height, id);
	else if (type == "Group")
		win->AddGroup(title, x, y, width, height, id);
	else if (type == "CheckBox")
		win->AddCheckBox(title, x, y, width, height, id);
	else if (type == "ComboBox")
		win->AddComboBox(title, x, y, width, height, id);
	else if (type == "TabControl")
		win->AddTabControl(title, x, y, width, height, id);
	else if (type == "ListView")
		win->AddListView(title, x, y, width, height, id);
	else if (type == "TreeView")
		win->AddTreeView(title, x, y, width, height, id);
	else if (type == "IconView")
		win->AddIconView(title, x, y, width, height, id);
	else if (type == "ProgressBar")
		win->AddProgressBar(title, x, y, width, height, id);
	else if (type == "Slider")
		win->AddSlider(title, x, y, width, height, id);
	else if (type == "Image")
		win->AddImage(title, x, y, width, height, id);
	else if (type == "DrawingArea")
		win->AddDrawingArea(title, x, y, width, height, id);
	else if (type == "ControlTable")
		win->AddControlTable(title, x, y, width, height, id);
	else if (type == "SpinButton")
		win->AddSpinButton(title, x, y, width, height, id);
	else if (type == "RadioButton")
		win->AddRadioButton(title, x, y, width, height, id);
	else if (type == "ToggleButton")
		win->AddToggleButton(title, x, y, width, height, id);
}

void HuiWindow::FromResource(const string &id)
{
	msg_db_r("Window.FromResource",1);
	HuiResource *res = HuiGetResource(id);
	if (!res)
		msg_db_l(1);

	// title
	SetTitle(HuiGetLanguage(res->id));

	// size
	SetSize(res->i_param[0], res->i_param[1]);


	// dialog
	/*CHuiWindow *dlg
	if (res->type == "SizableDialog")
		dlg = HuiCreateSizableDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);
	else
		dlg = HuiCreateDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);*/

	// menu?
	if (res->s_param[0].num > 0)
		SetMenu(HuiCreateResourceMenu(res->s_param[0]));

	// toolbar?
	if (res->s_param[1].num > 0)
		toolbar[HuiToolbarTop]->SetByID(res->s_param[1]);

	// controls
	foreach(HuiResourceCommand &cmd, res->cmd){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}else if (res->type == "SizableDialog"){
			//msg_write("insert " + cmd.id + " (" + cmd.type + ") into " + cmd.s_param[0]);
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}
		Enable(cmd.id, cmd.enabled);
		if (cmd.image.num > 0)
			SetImage(cmd.id, cmd.image);
	}
	msg_db_m("  \\(^_^)/",1);
	msg_db_l(1);
}

//----------------------------------------------------------------------------------
// data exchanging functions for control items


#define test_controls(_id, c)	\
	string tid = (_id.num == 0) ? cur_id : _id; \
	foreach(HuiControl *c, control) \
		if (c->id == tid)

// replace all the text
//    for all
void HuiWindow::SetString(const string &_id, const string &str)
{
	if (id == _id)
		SetTitle(str);
	test_controls(_id, c)
		c->SetString(str);
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void HuiWindow::SetInt(const string &_id, int n)
{
	test_controls(_id, c)
		c->SetInt(n);
}

// replace all the text with a float
//    for all
void HuiWindow::SetFloat(const string &_id, float f)
{
	test_controls(_id, c)
		c->SetFloat(f);
}

void HuiWindow::SetImage(const string &_id, const string &image)
{
	test_controls(_id, c)
		c->SetImage(image);
}

void HuiWindow::SetTooltip(const string &_id, const string &tip)
{
	test_controls(_id, c)
		c->SetTooltip(tip);
}


// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void HuiWindow::AddString(const string &_id, const string &str)
{
	test_controls(_id, c)
		c->AddString(str);
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::AddChildString(const string &_id, int parent_row, const string &str)
{
	test_controls(_id, c)
		c->AddChildString(parent_row, str);
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void HuiWindow::ChangeString(const string &_id,int row,const string &str)
{
	test_controls(_id, c)
		c->ChangeString(row, str);
}

// listview / treeview
string HuiWindow::GetCell(const string &_id, int row, int column)
{
	test_controls(_id, c)
		return c->GetCell(row, column);
	return "";
}

// listview / treeview
void HuiWindow::SetCell(const string &_id, int row, int column, const string &str)
{
	test_controls(_id, c)
		c->SetCell(row, column, str);
}

void HuiWindow::SetColor(const string &_id, const color &col)
{
	test_controls(_id, c)
		c->SetColor(col);
}

// retrieve the text
//    for edit
string HuiWindow::GetString(const string &_id)
{
	test_controls(_id, c)
		return c->GetString();
	return "";
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int HuiWindow::GetInt(const string &_id)
{
	test_controls(_id, c)
		return c->GetInt();
	return 0;
}

// retrieve the text as a numerical value (float)
//    for edit
float HuiWindow::GetFloat(const string &_id)
{
	test_controls(_id, c)
		return c->GetFloat();
	return 0;
}

color HuiWindow::GetColor(const string &_id)
{
	test_controls(_id, c)
		return c->GetColor();
	return Black;
}

// switch control to usable/unusable
//    for all
void HuiWindow::Enable(const string &_id,bool enabled)
{
	test_controls(_id, c)
		c->Enable(enabled);
}

// show/hide control
//    for all
void HuiWindow::HideControl(const string &_id,bool hide)
{
	test_controls(_id, c)
		c->Hide(hide);
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void HuiWindow::Check(const string &_id,bool checked)
{
	test_controls(_id, c)
		c->Check(checked);
}

// is marked as "checked"?
//    for CheckBox
bool HuiWindow::IsChecked(const string &_id)
{
	test_controls(_id, c)
		return c->IsChecked();
	return false;
}

// which lines are selected?
//    for ListView
Array<int> HuiWindow::GetMultiSelection(const string &_id)
{
	test_controls(_id, c)
		return c->GetMultiSelection();
	Array<int> sel;
	return sel;
}

void HuiWindow::SetMultiSelection(const string &_id, Array<int> &sel)
{
	test_controls(_id, c)
		c->SetMultiSelection(sel);
}

// delete all the content
//    for ComboBox, ListView
void HuiWindow::Reset(const string &_id)
{
	test_controls(_id, c)
		c->Reset();
}

void HuiWindow::CompletionAdd(const string &_id, const string &text)
{
	test_controls(_id, c)
		c->CompletionAdd(text);
}

void HuiWindow::CompletionClear(const string &_id)
{
	test_controls(_id, c)
		c->CompletionClear();
}

// expand a single row
//    for TreeView
void HuiWindow::Expand(const string &_id, int row, bool expand)
{
	test_controls(_id, c)
		c->Expand(row, expand);
}

// expand all rows
//    for TreeView
void HuiWindow::ExpandAll(const string &_id, bool expand)
{
	test_controls(_id, c)
		c->ExpandAll(expand);
}

// is column in tree expanded?
//    for TreeView
bool HuiWindow::IsExpanded(const string &_id, int row)
{
	test_controls(_id, c)
		return false;
	return false;
}




HuiWindow *HuiCreateDialog(const string &title,int width,int height,HuiWindow *root,bool allow_root)
{
	return new HuiWindow(	title,
							-1, -1, width, height,
							root, allow_root,
							HuiWinModeControls);
}

HuiWindow *HuiCreateSizableDialog(const string &title,int width,int height,HuiWindow *root,bool allow_root)
{
	return new HuiWindow(	title,
							-1, -1, width, height,
							root, allow_root,
							HuiWinModeControls | HuiWinModeResizable);
}

void HuiFuncIgnore()
{
}

void HuiFuncClose()
{
	delete(HuiGetEvent()->win);
}

HuiNixWindow::HuiNixWindow(const string& title, int x, int y, int width, int height) :
	HuiWindow(title, x, y, width, height, NULL, true, HuiWinModeResizable | HuiWinModeNix)
{
}

void HuiNixWindow::__init_ext__(const string& title, int x, int y, int width, int height)
{
	new(this) HuiNixWindow(title, x, y, width, height);
}

HuiDialog::HuiDialog(const string& title, int width, int height, HuiWindow* root, bool allow_root) :
	HuiWindow(title, -1, -1, width, height, root, allow_root, HuiWinModeControls | HuiWinModeResizable)
{
}

void HuiDialog::__init_ext__(const string& title, int width, int height, HuiWindow* root, bool allow_root)
{
	new(this) HuiDialog(title, width, height, root, allow_root);
}

HuiFixedDialog::HuiFixedDialog(const string& title, int width, int height, HuiWindow* root, bool allow_root) :
	HuiWindow(title, -1, -1, width, height, root, allow_root, HuiWinModeControls)
{
}

void HuiFixedDialog::__init_ext__(const string& title, int width, int height, HuiWindow* root, bool allow_root)
{
	new(this) HuiFixedDialog(title, width, height, root, allow_root);
}
