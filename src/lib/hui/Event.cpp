#include "hui.h"
#include "common.h"

namespace hui
{


#ifdef HUI_API_WIN
	unsigned char HuiKeyID[256];
#endif
#ifdef HUI_API_GTK
	int HuiKeyID[256], HuiKeyID2[256];
#endif



EventListener::EventListener(int _uid, const string &_id, const string &_message, const Callback &_function)
{
	uid = _uid;
	id = _id;
	message = _message;
	function = _function;
	enabled = true;
	type = 0;
}

EventListener::EventListener(int _uid, const string &_id, const string &_message, int __, const CallbackP &_function)
{
	uid = _uid;
	id = _id;
	message = _message;
	function_p = _function;
	enabled = true;
	type = 0;
}

EventKeyCode::EventKeyCode(const string &_id, const string &_message, int _key_code)
{
	id = _id;
	message = _message;
	key_code = _key_code;

}

Event _hui_event_;
Event *GetEvent()
{
	return &_hui_event_;
}

Event::Event(const string &_id, const string &_message)
{
	dx = 0;
	dy = 0;
	scroll_x = 0;
	scroll_y = 0;
	id = _id;
	message = _message;
	is_default = true;
	height = width = 0;
	key = -1;
	key_code = -1;
	mx = my = 0;
	win = nullptr;
	row = row_target = 0;
	rbut = mbut = lbut = false;
	column = 0;
	just_focused = false;
}

bool Event::match(const string &_id, const string &_message) const
{
	// all events
	if (_id == "*")
		return true;

	// direct match ("extended")
	if ((_id == id) and (_message == message))
		return true;

	// default match
	if ((_id == id) and (_message == ":def:") and is_default)
		return true;
	if ((_id == message) and (id == "") and (_message == ":def:") and is_default)
		return true;

	// simple match
	if (_message == "*"){
		if (_id == id)
			return true;
		if ((_id == message) and (id == ""))
			return true;
	}
	return false;
}

void _InitInput_()
{
	#ifdef HUI_API_WIN

		for (int k=0;k<256;k++)
			HuiKeyID[k]=255;
		HuiKeyID[VK_LCONTROL	] = KEY_LCONTROL;
		HuiKeyID[VK_RCONTROL	] = KEY_RCONTROL;
		HuiKeyID[VK_LSHIFT		] = KEY_LSHIFT;
		HuiKeyID[VK_RSHIFT		] = KEY_RSHIFT;
		HuiKeyID[VK_LMENU		] = KEY_LALT;
		HuiKeyID[VK_RMENU		] = KEY_RALT;
		HuiKeyID[187			] = KEY_ADD;
		HuiKeyID[189			] = KEY_SUBTRACT;
		HuiKeyID[191			] = KEY_FENCE;
		HuiKeyID[220			] = KEY_GRAVE;
		HuiKeyID[VK_END			] = KEY_END;
		HuiKeyID[VK_NEXT		] = KEY_NEXT;
		HuiKeyID[VK_PRIOR		] = KEY_PRIOR;
		HuiKeyID[VK_UP			] = KEY_UP;
		HuiKeyID[VK_DOWN		] = KEY_DOWN;
		HuiKeyID[VK_LEFT		] = KEY_LEFT;
		HuiKeyID[VK_RIGHT		] = KEY_RIGHT;
		HuiKeyID[VK_RETURN		] = KEY_RETURN;
		HuiKeyID[VK_ESCAPE		] = KEY_ESCAPE;
		HuiKeyID[VK_INSERT		] = KEY_INSERT;
		HuiKeyID[VK_DELETE		] = KEY_DELETE;
		HuiKeyID[VK_SPACE		] = KEY_SPACE;
		HuiKeyID[VK_F1			] = KEY_F1;
		HuiKeyID[VK_F2			] = KEY_F2;
		HuiKeyID[VK_F3			] = KEY_F3;
		HuiKeyID[VK_F4			] = KEY_F4;
		HuiKeyID[VK_F5			] = KEY_F5;
		HuiKeyID[VK_F6			] = KEY_F6;
		HuiKeyID[VK_F7			] = KEY_F7;
		HuiKeyID[VK_F8			] = KEY_F8;
		HuiKeyID[VK_F9			] = KEY_F9;
		HuiKeyID[VK_F10			] = KEY_F10;
		HuiKeyID[VK_F11			] = KEY_F11;
		HuiKeyID[VK_F12			] = KEY_F12;
		HuiKeyID['1'			] = KEY_1;
		HuiKeyID['2'			] = KEY_2;
		HuiKeyID['3'			] = KEY_3;
		HuiKeyID['4'			] = KEY_4;
		HuiKeyID['5'			] = KEY_5;
		HuiKeyID['6'			] = KEY_6;
		HuiKeyID['7'			] = KEY_7;
		HuiKeyID['8'			] = KEY_8;
		HuiKeyID['9'			] = KEY_9;
		HuiKeyID['0'			] = KEY_0;
		HuiKeyID['A'			] = KEY_A;
		HuiKeyID['B'			] = KEY_B;
		HuiKeyID['C'			] = KEY_C;
		HuiKeyID['D'			] = KEY_D;
		HuiKeyID['E'			] = KEY_E;
		HuiKeyID['F'			] = KEY_F;
		HuiKeyID['G'			] = KEY_G;
		HuiKeyID['H'			] = KEY_H;
		HuiKeyID['I'			] = KEY_I;
		HuiKeyID['J'			] = KEY_J;
		HuiKeyID['K'			] = KEY_K;
		HuiKeyID['L'			] = KEY_L;
		HuiKeyID['M'			] = KEY_M;
		HuiKeyID['N'			] = KEY_N;
		HuiKeyID['O'			] = KEY_O;
		HuiKeyID['P'			] = KEY_P;
		HuiKeyID['Q'			] = KEY_Q;
		HuiKeyID['R'			] = KEY_R;
		HuiKeyID['S'			] = KEY_S;
		HuiKeyID['T'			] = KEY_T;
		HuiKeyID['U'			] = KEY_U;
		HuiKeyID['V'			] = KEY_V;
		HuiKeyID['W'			] = KEY_W;
		HuiKeyID['X'			] = KEY_X;
		HuiKeyID['Y'			] = KEY_Y;
		HuiKeyID['Z'			] = KEY_Z;
		HuiKeyID[VK_BACK		] = KEY_BACKSPACE;
		HuiKeyID[VK_TAB			] = KEY_TAB;
		HuiKeyID[VK_HOME		] = KEY_HOME;
		HuiKeyID[VK_NUMPAD0		] = KEY_NUM_0;
		HuiKeyID[VK_NUMPAD1		] = KEY_NUM_1;
		HuiKeyID[VK_NUMPAD2		] = KEY_NUM_2;
		HuiKeyID[VK_NUMPAD3		] = KEY_NUM_3;
		HuiKeyID[VK_NUMPAD4		] = KEY_NUM_4;
		HuiKeyID[VK_NUMPAD5		] = KEY_NUM_5;
		HuiKeyID[VK_NUMPAD6		] = KEY_NUM_6;
		HuiKeyID[VK_NUMPAD7		] = KEY_NUM_7;
		HuiKeyID[VK_NUMPAD8		] = KEY_NUM_8;
		HuiKeyID[VK_NUMPAD9		] = KEY_NUM_9;
		HuiKeyID[VK_ADD			] = KEY_NUM_ADD;
		HuiKeyID[VK_SUBTRACT	] = KEY_NUM_SUBTRACT;
		HuiKeyID[VK_MULTIPLY	] = KEY_NUM_MULTIPLY;
		HuiKeyID[VK_DIVIDE		] = KEY_NUM_DIVIDE;
		HuiKeyID[VK_DECIMAL		] = KEY_NUM_COMMA;
		//HuiKeyID[VK_RETURN	] = KEY_NUM_ENTER;
		HuiKeyID[188			] = KEY_COMMA;
		HuiKeyID[190			] = KEY_DOT;
		HuiKeyID[226			] = KEY_SMALLER;
		HuiKeyID[219			] = KEY_SZ;
		HuiKeyID[222			] = KEY_AE;
		HuiKeyID[192			] = KEY_OE;
		HuiKeyID[186			] = KEY_UE;
		
	#endif
	#ifdef HUI_API_GTK

		for (int k=0;k<NUM_KEYS;k++)
			HuiKeyID[k]=HuiKeyID2[k]=0;

		HuiKeyID[KEY_LCONTROL]=GDK_KEY_Control_L;
		HuiKeyID[KEY_RCONTROL]=GDK_KEY_Control_R;
		HuiKeyID[KEY_LSHIFT]=GDK_KEY_Shift_L;
		HuiKeyID[KEY_RSHIFT]=GDK_KEY_Shift_R;
		HuiKeyID[KEY_LALT]=GDK_KEY_Alt_L;
		HuiKeyID[KEY_RALT]=GDK_KEY_Alt_R;
		HuiKeyID[KEY_ADD]=GDK_KEY_plus;
		HuiKeyID[KEY_SUBTRACT]=GDK_KEY_minus;
		HuiKeyID[KEY_FENCE]=GDK_KEY_numbersign;
		HuiKeyID[KEY_GRAVE]=GDK_KEY_asciicircum;
		HuiKeyID[KEY_HOME]=GDK_KEY_Home;
		HuiKeyID[KEY_END]=GDK_KEY_End;
		HuiKeyID[KEY_NEXT]=GDK_KEY_Page_Up;
		HuiKeyID[KEY_PRIOR]=GDK_KEY_Page_Down;
		HuiKeyID[KEY_UP]=GDK_KEY_Up;
		HuiKeyID[KEY_DOWN]=GDK_KEY_Down;
		HuiKeyID[KEY_LEFT]=GDK_KEY_Left;
		HuiKeyID[KEY_RIGHT]=GDK_KEY_Right;
		HuiKeyID[KEY_RETURN]=GDK_KEY_Return;
		HuiKeyID[KEY_ESCAPE]=GDK_KEY_Escape;
		HuiKeyID[KEY_INSERT]=GDK_KEY_Insert;
		HuiKeyID[KEY_DELETE]=GDK_KEY_Delete;
		HuiKeyID[KEY_SPACE]=GDK_KEY_space;
		HuiKeyID[KEY_F1]=GDK_KEY_F1;
		HuiKeyID[KEY_F2]=GDK_KEY_F2;
		HuiKeyID[KEY_F3]=GDK_KEY_F3;
		HuiKeyID[KEY_F4]=GDK_KEY_F4;
		HuiKeyID[KEY_F5]=GDK_KEY_F5;
		HuiKeyID[KEY_F6]=GDK_KEY_F6;
		HuiKeyID[KEY_F7]=GDK_KEY_F7;
		HuiKeyID[KEY_F8]=GDK_KEY_F8;
		HuiKeyID[KEY_F9]=GDK_KEY_F9;
		HuiKeyID[KEY_F10]=GDK_KEY_F10;
		HuiKeyID[KEY_F11]=GDK_KEY_F11;
		HuiKeyID[KEY_F12]=GDK_KEY_F12;
		HuiKeyID[KEY_1]=GDK_KEY_1;
		HuiKeyID[KEY_2]=GDK_KEY_2;
		HuiKeyID[KEY_3]=GDK_KEY_3;
		HuiKeyID[KEY_4]=GDK_KEY_4;
		HuiKeyID[KEY_5]=GDK_KEY_5;
		HuiKeyID[KEY_6]=GDK_KEY_6;
		HuiKeyID[KEY_7]=GDK_KEY_7;
		HuiKeyID[KEY_8]=GDK_KEY_8;
		HuiKeyID[KEY_9]=GDK_KEY_9;
		HuiKeyID[KEY_0]=GDK_KEY_0;
		HuiKeyID[KEY_A]=GDK_KEY_a;		HuiKeyID2[KEY_A]=GDK_KEY_A;
		HuiKeyID[KEY_B]=GDK_KEY_b;		HuiKeyID2[KEY_B]=GDK_KEY_B;
		HuiKeyID[KEY_C]=GDK_KEY_c;		HuiKeyID2[KEY_C]=GDK_KEY_C;
		HuiKeyID[KEY_D]=GDK_KEY_d;		HuiKeyID2[KEY_D]=GDK_KEY_D;
		HuiKeyID[KEY_E]=GDK_KEY_e;		HuiKeyID2[KEY_E]=GDK_KEY_E;
		HuiKeyID[KEY_F]=GDK_KEY_f;		HuiKeyID2[KEY_F]=GDK_KEY_F;
		HuiKeyID[KEY_G]=GDK_KEY_g;		HuiKeyID2[KEY_G]=GDK_KEY_G;
		HuiKeyID[KEY_H]=GDK_KEY_h;		HuiKeyID2[KEY_H]=GDK_KEY_H;
		HuiKeyID[KEY_I]=GDK_KEY_i;		HuiKeyID2[KEY_I]=GDK_KEY_I;
		HuiKeyID[KEY_J]=GDK_KEY_j;		HuiKeyID2[KEY_J]=GDK_KEY_J;
		HuiKeyID[KEY_K]=GDK_KEY_k;		HuiKeyID2[KEY_K]=GDK_KEY_K;
		HuiKeyID[KEY_L]=GDK_KEY_l;		HuiKeyID2[KEY_L]=GDK_KEY_L;
		HuiKeyID[KEY_M]=GDK_KEY_m;		HuiKeyID2[KEY_M]=GDK_KEY_M;
		HuiKeyID[KEY_N]=GDK_KEY_n;		HuiKeyID2[KEY_N]=GDK_KEY_N;
		HuiKeyID[KEY_O]=GDK_KEY_o;		HuiKeyID2[KEY_O]=GDK_KEY_O;
		HuiKeyID[KEY_P]=GDK_KEY_p;		HuiKeyID2[KEY_P]=GDK_KEY_P;
		HuiKeyID[KEY_Q]=GDK_KEY_q;		HuiKeyID2[KEY_Q]=GDK_KEY_Q;
		HuiKeyID[KEY_R]=GDK_KEY_r;		HuiKeyID2[KEY_R]=GDK_KEY_R;
		HuiKeyID[KEY_S]=GDK_KEY_s;		HuiKeyID2[KEY_S]=GDK_KEY_S;
		HuiKeyID[KEY_T]=GDK_KEY_t;		HuiKeyID2[KEY_T]=GDK_KEY_T;
		HuiKeyID[KEY_U]=GDK_KEY_u;		HuiKeyID2[KEY_U]=GDK_KEY_U;
		HuiKeyID[KEY_V]=GDK_KEY_v;		HuiKeyID2[KEY_V]=GDK_KEY_V;
		HuiKeyID[KEY_W]=GDK_KEY_w;		HuiKeyID2[KEY_W]=GDK_KEY_W;
		HuiKeyID[KEY_X]=GDK_KEY_x;		HuiKeyID2[KEY_X]=GDK_KEY_X;
		HuiKeyID[KEY_Y]=GDK_KEY_y;		HuiKeyID2[KEY_Y]=GDK_KEY_Y;
		HuiKeyID[KEY_Z]=GDK_KEY_z;		HuiKeyID2[KEY_Z]=GDK_KEY_Z;
		HuiKeyID[KEY_BACKSPACE]=GDK_KEY_BackSpace;
		HuiKeyID[KEY_TAB]=GDK_KEY_Tab;
		HuiKeyID[KEY_NUM_0]=GDK_KEY_KP_0;
		HuiKeyID[KEY_NUM_1]=GDK_KEY_KP_1;
		HuiKeyID[KEY_NUM_2]=GDK_KEY_KP_2;
		HuiKeyID[KEY_NUM_3]=GDK_KEY_KP_3;
		HuiKeyID[KEY_NUM_4]=GDK_KEY_KP_4;
		HuiKeyID[KEY_NUM_5]=GDK_KEY_KP_5;
		HuiKeyID[KEY_NUM_6]=GDK_KEY_KP_6;
		HuiKeyID[KEY_NUM_7]=GDK_KEY_KP_7;
		HuiKeyID[KEY_NUM_8]=GDK_KEY_KP_8;
		HuiKeyID[KEY_NUM_9]=GDK_KEY_KP_9;
		HuiKeyID[KEY_NUM_ADD]=GDK_KEY_KP_Add;
		HuiKeyID[KEY_NUM_SUBTRACT]=GDK_KEY_KP_Subtract;
		HuiKeyID[KEY_NUM_MULTIPLY]=GDK_KEY_KP_Multiply;
		HuiKeyID[KEY_NUM_DIVIDE]=GDK_KEY_KP_Divide;
		HuiKeyID[KEY_NUM_COMMA]=GDK_KEY_KP_Decimal;
		HuiKeyID[KEY_NUM_ENTER]=GDK_KEY_KP_Enter;
		HuiKeyID[KEY_COMMA]=GDK_KEY_comma;				HuiKeyID2[KEY_COMMA]=GDK_KEY_semicolon;
		HuiKeyID[KEY_DOT]=GDK_KEY_period;
		HuiKeyID[KEY_SMALLER]=GDK_KEY_less;			HuiKeyID2[KEY_AE]=GDK_KEY_greater;
		HuiKeyID[KEY_SZ]=GDK_KEY_ssharp;
		HuiKeyID[KEY_AE]=GDK_KEY_adiaeresis;			HuiKeyID2[KEY_AE]=GDK_KEY_ae;
		HuiKeyID[KEY_OE]=GDK_KEY_odiaeresis;					HuiKeyID2[KEY_AE]=GDK_KEY_oe;
		HuiKeyID[KEY_UE]=GDK_KEY_udiaeresis;
		HuiKeyID[KEY_LWINDOWS]=GDK_KEY_Super_L;
		HuiKeyID[KEY_RWINDOWS]=GDK_KEY_Super_R;
	#endif
}

string GetKeyName(int k)
{
	if (k==KEY_LCONTROL)	return "ControlL";
	if (k==KEY_RCONTROL)	return "ControlR";
	if (k==KEY_LSHIFT)		return "ShiftL";
	if (k==KEY_RSHIFT)		return "ShiftR";
	if (k==KEY_LALT)		return "AltL";
	if (k==KEY_RALT)		return "AltR";
	if (k==KEY_ADD)			return "Add";
	if (k==KEY_SUBTRACT)	return "Subtract";
	if (k==KEY_FENCE)		return "Fence";
	if (k==KEY_GRAVE)		return "Grave";
	if (k==KEY_END)			return "End";
	if (k==KEY_NEXT)		return "Next";
	if (k==KEY_PRIOR)		return "Prior";
	if (k==KEY_UP)			return "ArrowUp";
	if (k==KEY_DOWN)		return "ArrowDown";
	if (k==KEY_LEFT)		return "ArrowLeft";
	if (k==KEY_RIGHT)		return "ArrowRight";
	if (k==KEY_RETURN)		return "Return";
	if (k==KEY_ESCAPE)		return "Escape";
	if (k==KEY_INSERT)		return "Insert";
	if (k==KEY_DELETE)		return "Delete";
	if (k==KEY_SPACE)		return "Space";
	if (k==KEY_F1)			return "F1";
	if (k==KEY_F2)			return "F2";
	if (k==KEY_F3)			return "F3";
	if (k==KEY_F4)			return "F4";
	if (k==KEY_F5)			return "F5";
	if (k==KEY_F6)			return "F6";
	if (k==KEY_F7)			return "F7";
	if (k==KEY_F8)			return "F8";
	if (k==KEY_F9)			return "F9";
	if (k==KEY_F10)			return "F10";
	if (k==KEY_F11)			return "F11";
	if (k==KEY_F12)			return "F12";
	if (k==KEY_1)			return "1";
	if (k==KEY_2)			return "2";
	if (k==KEY_3)			return "3";
	if (k==KEY_4)			return "4";
	if (k==KEY_5)			return "5";
	if (k==KEY_6)			return "6";
	if (k==KEY_7)			return "7";
	if (k==KEY_8)			return "8";
	if (k==KEY_9)			return "9";
	if (k==KEY_0)			return "0";
	if (k==KEY_A)			return "A";
	if (k==KEY_B)			return "B";
	if (k==KEY_C)			return "C";
	if (k==KEY_D)			return "D";
	if (k==KEY_E)			return "E";
	if (k==KEY_F)			return "F";
	if (k==KEY_G)			return "G";
	if (k==KEY_H)			return "H";
	if (k==KEY_I)			return "I";
	if (k==KEY_J)			return "J";
	if (k==KEY_K)			return "K";
	if (k==KEY_L)			return "L";
	if (k==KEY_M)			return "M";
	if (k==KEY_N)			return "N";
	if (k==KEY_O)			return "O";
	if (k==KEY_P)			return "P";
	if (k==KEY_Q)			return "Q";
	if (k==KEY_R)			return "R";
	if (k==KEY_S)			return "S";
	if (k==KEY_T)			return "T";
	if (k==KEY_U)			return "U";
	if (k==KEY_V)			return "V";
	if (k==KEY_W)			return "W";
	if (k==KEY_X)			return "X";
	if (k==KEY_Y)			return "Y";
	if (k==KEY_Z)			return "Z";
	if (k==KEY_BACKSPACE)	return "Backspace";
	if (k==KEY_TAB)			return "Tab";
	if (k==KEY_HOME)		return "Home";
	if (k==KEY_NUM_0)		return "Num 0";
	if (k==KEY_NUM_1)		return "Num 1";
	if (k==KEY_NUM_2)		return "Num 2";
	if (k==KEY_NUM_3)		return "Num 3";
	if (k==KEY_NUM_4)		return "Num 4";
	if (k==KEY_NUM_5)		return "Num 5";
	if (k==KEY_NUM_6)		return "Num 6";
	if (k==KEY_NUM_7)		return "Num 7";
	if (k==KEY_NUM_8)		return "Num 8";
	if (k==KEY_NUM_9)		return "Num 9";
	if (k==KEY_NUM_ADD)		return "Num Add";
	if (k==KEY_NUM_SUBTRACT)return "Num Subtract";
	if (k==KEY_NUM_MULTIPLY)return "Num Multiply";
	if (k==KEY_NUM_DIVIDE)	return "Num Divide";
	if (k==KEY_NUM_ENTER)	return "Num Enter";
	if (k==KEY_NUM_COMMA)	return "Num Comma";
	if (k==KEY_COMMA)		return "Comma";
	if (k==KEY_DOT)			return "Dot";
	if (k==KEY_SMALLER)		return "<";
	if (k==KEY_SZ)			return str_m_to_utf8("&s");
	if (k==KEY_AE)			return str_m_to_utf8("&A");
	if (k==KEY_OE)			return str_m_to_utf8("&O");
	if (k==KEY_UE)			return str_m_to_utf8("&U");
	if (k==KEY_RWINDOWS)	return "WindowsR";
	if (k==KEY_LWINDOWS)	return "WindowsL";
	return "";
}


string GetKeyCodeName(int key_code)
{
	if (key_code < 0)
		return "";
	string n;
	if ((key_code & KEY_CONTROL) == KEY_CONTROL)
		n += "Ctrl+";
	if ((key_code & KEY_SHIFT) == KEY_SHIFT)
		n += "Shift+";
	n += GetKeyName(key_code % 256);
	return n;
}

string GetKeyChar(int key_code)
{
	// TODO ... using German key table
	
	if (key_code < 0)
		return "";
	int key = (key_code % 256);
	// shift
	if ((key_code & KEY_SHIFT) > 0){
		for (int i=0;i<26;i++)
			if (key == KEY_A + i){
				string r = "A";
				r[0] += i;
				return r;
			}
		if (key==KEY_1)			return "!";
		if (key==KEY_2)			return "\"";
		if (key==KEY_3)			return "§";
		if (key==KEY_4)			return "$";
		if (key==KEY_5)			return "%";
		if (key==KEY_6)			return "&";
		if (key==KEY_7)			return "/";
		if (key==KEY_8)			return "(";
		if (key==KEY_9)			return ")";
		if (key==KEY_0)			return "=";
		if (key==KEY_COMMA)		return ";";
		if (key==KEY_DOT)		return ":";
		if (key==KEY_ADD)		return "*";
		if (key==KEY_SUBTRACT)	return "_";
		if (key==KEY_SMALLER)	return ">";
		if (key==KEY_SZ)			return "?";
		if (key==KEY_AE)			return str_m_to_utf8("&A");
		if (key==KEY_OE)			return str_m_to_utf8("&O");
		if (key==KEY_UE)			return str_m_to_utf8("&U");
		if (key==KEY_FENCE)		return "\'";
		if (key==KEY_GRAVE)		return "°";
		if (key==KEY_SPACE)		return " ";
		return "";
	}
	// alt
	if ((key_code & KEY_ALT) > 0){
		if (key==KEY_Q)			return "@";
		if (key==KEY_E)			return "€";
		//if (key==KEY_Y)			return -91;
		if (key==KEY_2)			return "²";
		if (key==KEY_3)			return "³";
		if (key==KEY_7)			return "{";
		if (key==KEY_8)			return "[";
		if (key==KEY_9)			return "]";
		if (key==KEY_0)			return "}";
		if (key==KEY_ADD)		return "~";
		if (key==KEY_SMALLER)	return "|";
		if (key==KEY_SZ)		return "\\";
		return "";
	}
	// normal
	for (int i=0;i<26;i++)
		if (key == KEY_A + i){
			string r = "a";
			r[0] += i;
			return r;
		}
	for (int i=0;i<10;i++)
		if ((key == KEY_0 + i) || (key == KEY_NUM_0 + i)){
			string r = "0";
			r[0] += i;
			return r;
		}
	if (key==KEY_COMMA)			return ",";
	if (key==KEY_DOT)			return ".";
	if (key==KEY_ADD)			return "+";
	if (key==KEY_SUBTRACT)		return "-";
	if (key==KEY_FENCE)			return "#";
	if (key==KEY_GRAVE)			return "^";
	if (key==KEY_NUM_ADD)			return "+";
	if (key==KEY_NUM_SUBTRACT)	return "-";
	if (key==KEY_NUM_MULTIPLY)	return "*";
	if (key==KEY_NUM_DIVIDE)		return "/";
	if (key==KEY_SMALLER)		return "<";
	if (key==KEY_SZ)				return str_m_to_utf8("&s");
	if (key==KEY_AE)				return str_m_to_utf8("&a");
	if (key==KEY_OE)				return str_m_to_utf8("&o");
	if (key==KEY_UE)				return str_m_to_utf8("&u");
	if (key==KEY_SPACE)			return " ";
	if (key==KEY_TAB)			return "\t";
	if (key==KEY_RETURN)			return "\n";
	if (key==KEY_NUM_ENTER)		return "\n";
	// unbekannt:
	return "";
}

extern Array<Window*> _hui_windows_;


void LoadKeyCodes(const string &filename)
{
}

void SaveKeyCodes(const string &filename)
{
}

};
