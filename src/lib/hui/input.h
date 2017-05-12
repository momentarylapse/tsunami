/*----------------------------------------------------------------------------*\
| Hui input                                                                    |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_INPUT_EXISTS_
#define _HUI_INPUT_EXISTS_

namespace hui
{

class EventHandler;

struct Command
{
	string id, image;
	int type, key_code;
	bool enabled;
	Callback func;
};

extern Array<Command> _hui_commands_;


class Event
{
	public:
	Window *win;
	string message, id;
	bool is_default;
	float mx, my;
	float dx, dy;
	float scroll_x, scroll_y;
	int key, key_code;
	string text;
	int width, height;
	bool lbut, mbut, rbut;
	int row, column, row_target;
	Event(){}
	Event(const string &id, const string &message);
};

extern Event _hui_event_;
Event *GetEvent();


class EventListener
{
public:
	string id, message;
	Callback function;
	CallbackP function_p;
	EventListener(){}
	EventListener(const string &id, const string &message, const Callback &function);
	EventListener(const string &id, const string &message, int __, const CallbackP &function);
};


// internal
void _InitInput_();
bool _EventMatch_(Event *e, const string &id, const string &message);
void _SendGlobalCommand_(Event *e);

// key codes and id table ("shortcuts")
void AddKeyCode(const string &id, int key_code);
void AddCommand(const string &id, const string &image, int default_key_code, const Callback &func);
void AddCommandToggle(const string &id, const string &image, int default_key_code, const Callback &func);

void LoadKeyCodes(const string &filename);
void SaveKeyCodes(const string &filename);

// input
string _cdecl GetKeyName(int key_code);
string _cdecl GetKeyCodeName(int key_code);
string _cdecl GetKeyChar(int key_code);


#ifdef HUI_API_GTK
extern GdkEvent *_hui_gdk_event_;
#endif

};

#endif
