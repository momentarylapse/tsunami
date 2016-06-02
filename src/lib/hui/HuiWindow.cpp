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
	_init_(title, x, y, width, height, root, allow_root, mode);
}

HuiWindow::HuiWindow()
{
	_init_("", -1, -1, 0, 0, NULL, true, HuiWinModeDummy);
}

HuiWindow::HuiWindow(const string &title, int x, int y, int width, int height)
{
	_init_(title, x, y, width, height, NULL, true, HuiWinModeResizable | HuiWinModeControls);
}

void HuiWindow::__init_ext__(const string& title, int x, int y, int width, int height)
{
	new(this) HuiWindow(title, x, y, width, height);
}


HuiWindow::HuiWindow(const string &id, HuiWindow *parent)
{
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error("HuiWindow: undefined resource id: " + id);
	}

	int mode = HuiWinModeControls;
	if (res->type == "SizableDialog")
		mode = HuiWinModeControls | HuiWinModeResizable;
	bool allow_parent = false;
	for (string &o : res->options)
		if ((o == "allow-root") or (o == "allow-parent"))
			allow_parent = true;
	_init_(HuiGetLanguage(id, id), -1, -1, res->w, res->h, parent, allow_parent, mode);

	// menu/toolbar?
	for (string &o : res->options){
		if (o.head(5) == "menu=")
			setMenu(HuiCreateResourceMenu(o.substr(5, -1)));
		if (o.head(8) == "toolbar=")
			toolbar[HuiToolbarTop]->setByID(o.substr(8, -1));
	}

	// controls
	for (HuiResource &cmd : res->children)
		_addControl(id, cmd, "");

	msg_db_m("  \\(^_^)/",1);
}

void HuiWindow::_init_generic_(HuiWindow *_root, bool _allow_root, int _mode)
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

void HuiWindow::_clean_up_()
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
void HuiWindow::onCloseRequest()
{
	int level = _get_main_level_();
	delete(this);
	
	// no message function (and last window in this main level): end program
	// ...or at least end nested main level
	for (HuiWindow *w : HuiWindows)
		if (w->_get_main_level_() >= level)
			return;
	HuiEnd();
}


// identify window (for automatic title assignment with language strings)
void HuiWindow::setID(const string &_id)
{
	id = _id;
	if ((HuiLanguaged) && (id.num > 0))
		setTitle(HuiGetLanguage(id, id));
}

// align window relative to another window (like..."top right corner")
void HuiWindow::setPositionSpecial(HuiWindow *win,int mode)
{
	int pw, ph, cw, ch, px, py, cx, cy;
	win->getSize(pw, ph);
	win->getPosition(px, py);
	getSize(cw, ch);
	getPosition(cx, cy);
	if ((mode & HuiLeft)>0)
		cx = px + 2;
	if ((mode & HuiRight)>0)
		cx = px + pw - cw - 2;
	if ((mode & HuiTop)>0)
		cy = py + 20;
	if ((mode & HuiBottom)>0)
		cy = py + ph - ch - 2;
	setPosition(cx, cy);
}

int HuiWindow::_get_main_level_()
{
	return main_level;
}

HuiMenu *HuiWindow::getMenu()
{
	return menu;
}

HuiWindow *HuiWindow::getParent()
{
	return parent;
}

bool HuiWindow::getKey(int k)
{
	if (k == KEY_CONTROL)
		return (input.key[KEY_RCONTROL] || input.key[KEY_LCONTROL]);
	else if (k == KEY_SHIFT)
		return (input.key[KEY_RSHIFT] || input.key[KEY_LSHIFT]);
	else
		return input.key[k];
}

bool HuiWindow::getMouse(int &x, int &y, int button)
{
	x = (int)input.x;
	y = (int)input.y;
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
	addDrawingArea("", 0, 0, 0, 0, "nix-area");
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
	fromSource(buffer);
}


