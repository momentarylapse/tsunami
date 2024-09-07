/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_RESOURCE_EXISTS_
#define _HUI_RESOURCE_EXISTS_

class Path;

namespace hui {

	class Panel;
	class Menu;
	class Window;

//----------------------------------------------------------------------------------
// resource handling

Window *_cdecl create_resource_dialog(const string &id, Window *root);
Menu *_cdecl create_resource_menu(const string &id, Panel *panel);



class Resource {
public:
	string type;
	string id;
	string title;
	string tooltip;
	Array<string> options;
	int x, y;
	Array<Resource> children;
	Resource();
	Resource* get_node(const string &id) const;
	bool enabled();
	string image();
	bool has(const string &key);
	string value(const string &key, const string &fallback = "");
	void show(int indent = 0);
	string to_string(int indent = 0);
};

Resource parse_resource(const string &buffer, bool literal = false);


// resources
void _cdecl load_resource(const Path &filename);
Resource *get_resource(const string &id);

};

#endif
