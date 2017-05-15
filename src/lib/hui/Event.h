/*----------------------------------------------------------------------------*\
| Hui input                                                                    |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef SRC_LIB_HUI_EVENT_
#define SRC_LIB_HUI_EVENT_

namespace hui
{

class EventHandler;


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

	bool match(const string &id, const string &message) const;
};

extern Event _hui_event_;
Event *GetEvent();


class EventListener
{
public:
	int type, key_code;
	bool enabled;
	string id, message;
	string image;
	Callback function;
	CallbackP function_p;
	EventListener(){}
	EventListener(const string &id, const string &message, const Callback &function);
	EventListener(const string &id, const string &message, int __, const CallbackP &function);
};


// internal
void _InitInput_();


void LoadKeyCodes(const string &filename);
void SaveKeyCodes(const string &filename);

// input
string _cdecl GetKeyName(int key_code);
string _cdecl GetKeyCodeName(int key_code);
string _cdecl GetKeyChar(int key_code);


};

#endif
