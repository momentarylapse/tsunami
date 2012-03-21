/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_RESOURCE_EXISTS_
#define _HUI_RESOURCE_EXISTS_

string str_escape(const string &str);
string str_unescape(const string &str);


//----------------------------------------------------------------------------------
// resource handling

CHuiWindow *_cdecl HuiCreateResourceDialog(const string &id, CHuiWindow *root);
CHuiMenu *_cdecl HuiCreateResourceMenu(const string &id);



// resource commands (don't change the order!!!)
enum{
	HuiResMenu,
	HuiResDialog,
	HuiResToolBar,
	HuiResSizableDialog,
};
enum{
	HuiCmdMenuAddItem,
	HuiCmdMenuAddItemImage,
	HuiCmdMenuAddItemCheckable,
	HuiCmdMenuAddItemSeparator,
	HuiCmdMenuAddItemPopup,
	HuiCmdDialogAddControl,
};

struct HuiResourceCommand
{
	string type;
	string id;
	bool enabled;
	bool b_param[2];
	int i_param[6];
	string s_param[2];
	string image;
	void reset()
	{
		type = "";
		id = "";
		s_param[0] = "";
		s_param[1] = "";
		image = "";
		enabled = true;
		memset(i_param, 0, sizeof(i_param));
		memset(b_param, 0, sizeof(b_param));
	}
};

struct HuiResource : HuiResourceCommand
{
	Array<HuiResourceCommand> cmd;
	void reset()
	{	cmd.clear();	((HuiResourceCommand*)this)->reset();	}
};

// resources
void _cdecl HuiLoadResource(const string &filename);
HuiResource *HuiGetResource(const string &id);

#endif
