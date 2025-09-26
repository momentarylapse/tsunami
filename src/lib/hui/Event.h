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
#include "../math/vec2.h"
#include "Callback.h"

namespace hui {

class Window;


class EventHandler : public VirtualBase {};



class Event {
	public:
	Window *win;
	string message, id;
	bool is_default;
	vec2 m, d;
	vec2 scroll;
	float pressure;
	int key_code;
	int width, height;
	bool lbut, mbut, rbut;
	int row, column, row_target;
	bool just_focused;
	static string _text;
	Event(){}
	Event(const string &id, const string &message);

	bool match(const string &id, const string &message) const;
	string text() const;
};

extern Event _hui_event_;
Event *get_event();

class EventKeyCode {
public:
	string id, message;
	int key_code;
	EventKeyCode(){}
	EventKeyCode(const string &id, const string &messgae, int key_code);
};

class EventListener {
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


void load_key_codes(const string &filename);
void save_key_codes(const string &filename);

// input
string _cdecl get_key_name(int key_code);
string _cdecl get_key_code_name(int key_code);
//string _cdecl GetKeyChar(int key_code);
int _cdecl parse_key_code(const string &code);



// key codes (physical keys)
enum {
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_PLUS,
	KEY_MINUS,
	KEY_FENCE,		// "Raute"???
	KEY_END,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
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
	KEY_LESS,
	KEY_SZ,
	KEY_AE,
	KEY_OE,
	KEY_UE,
	KEY_GRAVE,
	KEY_LWINDOWS,
	KEY_RWINDOWS,

	NUM_KEYS,

	KEY_ANY,
	KEY_CONTROL = 1<<8,
	KEY_SHIFT = 1<<9,
	KEY_ALT = 1<<10,
	KEY_META = 1<<11,
	KEY_SUPER = 1<<12,
	KEY_HYPER = 1<<13
};

namespace EventID {
extern const string _MATCH_DEFAULT_;
extern const string KEY_DOWN;
extern const string KEY_UP;
extern const string LEFT_BUTTON_DOWN;
extern const string LEFT_BUTTON_UP;
extern const string LEFT_DOUBLE_CLICK;
extern const string MIDDLE_BUTTON_DOWN;
extern const string MIDDLE_BUTTON_UP;
extern const string MIDDLE_DOUBLE_CLICK;
extern const string RIGHT_BUTTON_DOWN;
extern const string RIGHT_BUTTON_UP;
extern const string RIGHT_DOUBLE_CLICK;
extern const string MOUSE_MOVE;
extern const string MOUSE_WHEEL;
extern const string MOUSE_ENTER;
extern const string MOUSE_LEAVE;
extern const string GESTURE_ZOOM;
extern const string GESTURE_ZOOM_BEGIN;
extern const string GESTURE_ZOOM_END;
extern const string GESTURE_DRAG;
extern const string GESTURE_DRAG_BEGIN;
extern const string GESTURE_DRAG_END;
extern const string FOCUS_IN;
extern const string RESIZE;
extern const string DRAW;
extern const string DRAW_GL;
extern const string REALIZE;
extern const string UNREALIZE;
extern const string CLICK;
extern const string CHANGE;
extern const string SELECT;
extern const string EDIT;
extern const string ACTIVATE;
extern const string MOVE;
extern const string CLOSE;
extern const string INFO;
bool is_valid(const string &id);
}


};

#endif
