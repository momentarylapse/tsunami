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

HuiWindow *_cdecl HuiCreateResourceDialog(const string &id, HuiWindow *root);
HuiMenu *_cdecl HuiCreateResourceMenu(const string &id);



class HuiResource
{
public:
	string type;
	string id;
	bool enabled;
	bool b_param[2];
	int i_param[6];
	string s_param[2];
	string image;
	Array<HuiResource> children;
	void reset()
	{
		type = "";
		id = "";
		s_param[0] = "";
		s_param[1] = "";
		image = "";
		enabled = true;
		children.clear();
		memset(i_param, 0, sizeof(i_param));
		memset(b_param, 0, sizeof(b_param));
	}
};



class HuiResourceNew
{
public:
	string type;
	string id;
	string title;
	bool enabled;
	Array<string> options;
	int x, y, w, h;
	string image;
	Array<HuiResourceNew> children;
	void load(const string &buffer);
	void show(int indent = 0);
};

// resources
void _cdecl HuiLoadResource(const string &filename);
HuiResource *HuiGetResource(const string &id);

#endif
