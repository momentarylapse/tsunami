/*----------------------------------------------------------------------------*\
| Hui input                                                                    |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef SRC_LIB_HUI_EVENT_
#define SRC_LIB_HUI_EVENT_

#include "../base/base.h"
#include "Callback.h"

namespace hui
{

class Window;


class EventHandler : public VirtualBase
{
public:
};



class Event
{
	public:
	Window *win;
	string message, id;
	bool is_default;
	float mx, my;
	float dx, dy;
	float scroll_x, scroll_y;
	float pressure;
	int key, key_code;
	string text;
	int width, height;
	bool lbut, mbut, rbut;
	int row, column, row_target;
	bool just_focused;
	Event(){}
	Event(const string &id, const string &message);

	bool match(const string &id, const string &message) const;
};

extern Event _hui_event_;
Event *GetEvent();

class EventKeyCode
{
public:
	string id, message;
	int key_code;
	EventKeyCode(){}
	EventKeyCode(const string &id, const string &messgae, int key_code);
};

class EventListener
{
public:
	int uid;
	int type;
	bool enabled;
	string id, message;
	string image;
	Callback function;
	CallbackP function_p;
	EventListener(){}
	EventListener(int uid, const string &id, const string &message, const Callback &function);
	EventListener(int uid, const string &id, const string &message, int __, const CallbackP &function);
};


// internal
void _InitInput_();


void LoadKeyCodes(const string &filename);
void SaveKeyCodes(const string &filename);

// input
string _cdecl GetKeyName(int key_code);
string _cdecl GetKeyCodeName(int key_code);
string _cdecl GetKeyChar(int key_code);



// key codes
enum{
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_ADD,
	KEY_SUBTRACT,
	KEY_FENCE,		// "Raute"???
	KEY_END,
	KEY_NEXT,
	KEY_PRIOR,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_RETURN,
	KEY_ESCAPE,
	KEY_INSERT,
	KEY_DELETE,
	KEY_SPACE,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_HOME,
	KEY_NUM_0,
	KEY_NUM_1,
	KEY_NUM_2,
	KEY_NUM_3,
	KEY_NUM_4,
	KEY_NUM_5,
	KEY_NUM_6,
	KEY_NUM_7,
	KEY_NUM_8,
	KEY_NUM_9,
	KEY_NUM_ADD,
	KEY_NUM_SUBTRACT,
	KEY_NUM_MULTIPLY,
	KEY_NUM_DIVIDE,
	KEY_NUM_COMMA,
	KEY_NUM_ENTER,
	KEY_COMMA,
	KEY_DOT,
	KEY_SMALLER,
	KEY_SZ,
	KEY_AE,
	KEY_OE,
	KEY_UE,
	KEY_GRAVE,
	KEY_LWINDOWS,
	KEY_RWINDOWS,

	NUM_KEYS,

	KEY_ANY,
	KEY_CONTROL = 256,
	KEY_SHIFT = 512,
	KEY_ALT = 1024
};


};

#endif
