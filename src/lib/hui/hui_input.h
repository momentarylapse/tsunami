/*----------------------------------------------------------------------------*\
| Hui input                                                                    |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_INPUT_EXISTS_
#define _HUI_INPUT_EXISTS_

class HuiEventHandler;

typedef void hui_kaba_callback(HuiEventHandler *h);
typedef void hui_kaba_callback_p(HuiEventHandler *h, void *p);

class HuiCallback
{
	typedef void (HuiEventHandler::*MemberFuncP)();
	typedef void (HuiEventHandler::*MemberFuncPP)(void *p);
public:
	HuiCallback();
	HuiCallback(hui_callback *func);
	HuiCallback(HuiEventHandler *object, MemberFuncP f);
	HuiCallback(HuiEventHandler *object, hui_kaba_callback *function);
	void call();
	void call_p(void *p);
	bool is_set();
	bool has_handler(HuiEventHandler *object);

private:
	hui_callback *func;
	HuiEventHandler *object;
	hui_kaba_callback *kaba_func;
	MemberFuncP member_function;
};

struct HuiCommand
{
	string id, image;
	int type, key_code;
	bool enabled;
	HuiCallback func;
};

extern Array<HuiCommand> _HuiCommand_;


class HuiEvent
{
	public:
	HuiWindow *win;
	string message, id;
	bool is_default;
	float mx, my;
	float dx, dy;
	float scroll_x, scroll_y;
	int key, key_code;
	string text;
	int width, height;
	bool lbut, mbut, rbut;
	int row, column;
	HuiEvent(){}
	HuiEvent(const string &id, const string &message);
};

extern HuiEvent _HuiEvent_;
HuiEvent *HuiGetEvent();


class HuiEventListener
{
public:
	string id, message;
	HuiCallback function;
	HuiEventListener(){}
	HuiEventListener(const string &id, const string &message, HuiCallback function);
};


// internal
void _HuiInitInput_();
bool _HuiEventMatch_(HuiEvent *e, const string &id, const string &message);
void _HuiSendGlobalCommand_(HuiEvent *e);

// key codes and id table ("shortcuts")
void HuiAddKeyCode(const string &id, int key_code);
void HuiAddCommand(const string &id, const string &image, int default_key_code, hui_callback *func);
void HuiAddCommandToggle(const string &id, const string &image, int default_key_code, hui_callback *func);
void _HuiAddCommandM(const string &id, const string &image, int default_key_code, HuiEventHandler *object, void (HuiEventHandler::*function)());
void _HuiAddCommandMToggle(const string &id, const string &image, int default_key_code, HuiEventHandler *object, void (HuiEventHandler::*function)());
template<typename T>
void HuiAddCommandM(const string &id, const string &image, int default_key_code, void *object, T fun)
{	_HuiAddCommandM(id, image, default_key_code, (HuiEventHandler*)object, (void(HuiEventHandler::*)())fun);	}
template<typename T>
void HuiAddCommandMToggle(const string &id, const string &image, int default_key_code, void *object, T fun)
{	_HuiAddCommandMToggle(id, image, default_key_code, (HuiEventHandler*)object, (void(HuiEventHandler::*)())fun);	}

void HuiLoadKeyCodes(const string &filename);
void HuiSaveKeyCodes(const string &filename);

// input
string _cdecl HuiGetKeyName(int key_code);
string _cdecl HuiGetKeyCodeName(int key_code);
string _cdecl HuiGetKeyChar(int key_code);


#ifdef HUI_API_GTK
extern GdkEvent *HuiGdkEvent;
#endif

#endif
