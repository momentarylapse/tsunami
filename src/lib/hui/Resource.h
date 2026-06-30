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

namespace layout {
	struct Resource;
}

namespace hui {

	class Panel;
	class Menu;
	class Window;

//----------------------------------------------------------------------------------
// resource handling

Window *_cdecl create_resource_dialog(const string &id, Window *root);
Menu *_cdecl create_resource_menu(const string &id, Panel *panel);



// resources
void _cdecl load_resource(const Path &filename);
layout::Resource *get_resource(const string &id);

};

#endif
