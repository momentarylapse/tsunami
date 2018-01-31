/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_RESOURCE_EXISTS_
#define _HUI_RESOURCE_EXISTS_

namespace hui
{

//----------------------------------------------------------------------------------
// resource handling

Window *_cdecl CreateResourceDialog(const string &id, Window *root);
Menu *_cdecl CreateResourceMenu(const string &id);



class Resource
{
public:
	string type;
	string id;
	string title;
	string tooltip;
	Array<string> options;
	bool enabled;
	int x, y, w, h;
	int page;
	string image;
	Array<Resource> children;
	void reset();
	Resource* get_node(const string &id) const;
	void show(int indent = 0);
	string to_string(int indent = 0);
};

Resource ParseResource(const string &buffer, bool literal = false);


// resources
void _cdecl LoadResource(const string &filename);
Resource *GetResource(const string &id);

};

#endif
