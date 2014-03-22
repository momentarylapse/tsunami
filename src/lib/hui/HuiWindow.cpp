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
	foreach(HuiResource &cmd, res->children){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			SetTarget(cmd.s_param[0], cmd.i_param[4]);
			AddControl(	cmd.type, HuiGetLanguage(cmd.id),
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

void HuiWindow::_InitGeneric_(HuiWindow *_root, bool _allow_root, int _mode)
{
	msg_db_f("Window::_InitGeneric_", 2);
	_HuiMakeUsable_();
	HuiWindows.add(this);

	is_resizable = ((_mode & HuiWinModeResizable) > 0);
	allowed = true;
	allow_keys = true;
	parent = _root;
	main_input_control = NULL;
	if (parent){
		parent->allowed = _allow_root;
	}
	menu = popup = NULL;
	statusbar_enabled = false;
	toolbar[HuiToolbarTop] = new HuiToolbar(this);
	toolbar[HuiToolbarLeft] = new HuiToolbar(this, true);
	toolbar[HuiToolbarRight] = new HuiToolbar(this, true);
	toolbar[HuiToolbarBottom] = new HuiToolbar(this);
	input.reset();

	allow_input = false; // allow only if ->Show() was called
	main_level = HuiMainLevel;
}

void HuiWindow::_CleanUp_()
{
	msg_db_f("Window::_CleanUp_", 2);

	for (int i=0;i<4;i++)
		delete(toolbar[i]);

	_ClearPanel_();
	input.reset();
	
	// unregister window
	for (int i=0;i<HuiWindows.num;i++)
		if (HuiWindows[i] == this){
			HuiWindows.erase(i);
			break;
		}
}

// default handler when trying to close the windows
void HuiWindow::OnCloseRequest()
{
	int level = _GetMainLevel_();
	delete(this);
	
	// no message function (and last window in this main level): end program
	// ...or at least end nested main level
	foreach(HuiWindow *w, HuiWindows)
		if (w->_GetMainLevel_() >= level)
			return;
	HuiEnd();
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
	int pw, ph, cw, ch, px, py, cx, cy;
	win->GetSize(pw, ph);
	win->GetPosition(px, py);
	GetSize(cw, ch);
	GetPosition(cx, cy);
	if ((mode & HuiLeft)>0)
		cx = px + 2;
	if ((mode & HuiRight)>0)
		cx = px + pw - cw - 2;
	if ((mode & HuiTop)>0)
		cy = py + 20;
	if ((mode & HuiBottom)>0)
		cy = py + ph - ch - 2;
	SetPosition(cx, cy);
}

int HuiWindow::_GetMainLevel_()
{
	return main_level;
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





HuiWindow *HuiCreateDialog(const string &title,int width,int height,HuiWindow *root,bool allow_root)
{
	return new HuiFixedDialog(title, width, height, root, allow_root);
}

HuiWindow *HuiCreateSizableDialog(const string &title,int width,int height,HuiWindow *root,bool allow_root)
{
	return new HuiDialog(title, width, height, root, allow_root);
}

void HuiFuncIgnore()
{
}

void HuiFuncClose()
{
	delete(HuiGetEvent()->win);
}

HuiNixWindow::HuiNixWindow(const string& title, int x, int y, int width, int height) :
	HuiWindow(title, x, y, width, height, NULL, true, HuiWinModeResizable)
{
	AddDrawingArea("", 0, 0, 0, 0, "nix-area");
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



HuiSourceDialog::HuiSourceDialog(const string &buffer, HuiWindow *root) :
	HuiWindow("", -1, -1, 300, 200, root, buffer.find("allow-parent") > 0, HuiWinModeControls | HuiWinModeResizable)
{
	FromSource(buffer);
}


