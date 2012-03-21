#include "hui.h"



Array<HuiCommand> _HuiCommand_;

#ifdef HUI_API_WIN
	unsigned char HuiKeyID[256];
#endif
#ifdef HUI_API_GTK
	int HuiKeyID[256], KeyID2[256];
#endif


HuiEvent _HuiEvent_;
HuiEvent *HuiGetEvent()
{
	return &_HuiEvent_;
}

HuiEvent HuiCreateEvent(const string &id, const string &message)
{
	HuiEvent e;
	e.dx = 0;
	e.dy = 0;
	e.dz = 0;
	e.id = id;
	e.message = message;
	return e;
}

void _HuiInitInput_()
{
	#ifdef HUI_API_WIN

		for (int k=0;k<256;k++)
			HuiKeyID[k]=255;
		HuiKeyID[VK_LCONTROL	]=KeyLControl;
		HuiKeyID[VK_RCONTROL	]=KeyRControl;
		HuiKeyID[VK_LSHIFT		]=KeyLShift;
		HuiKeyID[VK_RSHIFT		]=KeyRShift;
		HuiKeyID[VK_LMENU		]=KeyLAlt;
		HuiKeyID[VK_RMENU		]=KeyRAlt;
		HuiKeyID[187			]=KeyAdd;
		HuiKeyID[189			]=KeySubtract;
		HuiKeyID[191			]=KeyFence;
		HuiKeyID[220			]=KeyGrave;
		HuiKeyID[VK_END			]=KeyEnd;
		HuiKeyID[VK_NEXT		]=KeyNext;
		HuiKeyID[VK_PRIOR		]=KeyPrior;
		HuiKeyID[VK_UP			]=KeyUp;
		HuiKeyID[VK_DOWN		]=KeyDown;
		HuiKeyID[VK_LEFT		]=KeyLeft;
		HuiKeyID[VK_RIGHT		]=KeyRight;
		HuiKeyID[VK_RETURN		]=KeyReturn;
		HuiKeyID[VK_ESCAPE		]=KeyEscape;
		HuiKeyID[VK_INSERT		]=KeyInsert;
		HuiKeyID[VK_DELETE		]=KeyDelete;
		HuiKeyID[VK_SPACE		]=KeySpace;
		HuiKeyID[VK_F1			]=KeyF1;
		HuiKeyID[VK_F2			]=KeyF2;
		HuiKeyID[VK_F3			]=KeyF3;
		HuiKeyID[VK_F4			]=KeyF4;
		HuiKeyID[VK_F5			]=KeyF5;
		HuiKeyID[VK_F6			]=KeyF6;
		HuiKeyID[VK_F7			]=KeyF7;
		HuiKeyID[VK_F8			]=KeyF8;
		HuiKeyID[VK_F9			]=KeyF9;
		HuiKeyID[VK_F10			]=KeyF10;
		HuiKeyID[VK_F11			]=KeyF11;
		HuiKeyID[VK_F12			]=KeyF12;
		HuiKeyID['1'			]=Key1;
		HuiKeyID['2'			]=Key2;
		HuiKeyID['3'			]=Key3;
		HuiKeyID['4'			]=Key4;
		HuiKeyID['5'			]=Key5;
		HuiKeyID['6'			]=Key6;
		HuiKeyID['7'			]=Key7;
		HuiKeyID['8'			]=Key8;
		HuiKeyID['9'			]=Key9;
		HuiKeyID['0'			]=Key0;
		HuiKeyID['A'			]=KeyA;
		HuiKeyID['B'			]=KeyB;
		HuiKeyID['C'			]=KeyC;
		HuiKeyID['D'			]=KeyD;
		HuiKeyID['E'			]=KeyE;
		HuiKeyID['F'			]=KeyF;
		HuiKeyID['G'			]=KeyG;
		HuiKeyID['H'			]=KeyH;
		HuiKeyID['I'			]=KeyI;
		HuiKeyID['J'			]=KeyJ;
		HuiKeyID['K'			]=KeyK;
		HuiKeyID['L'			]=KeyL;
		HuiKeyID['M'			]=KeyM;
		HuiKeyID['N'			]=KeyN;
		HuiKeyID['O'			]=KeyO;
		HuiKeyID['P'			]=KeyP;
		HuiKeyID['Q'			]=KeyQ;
		HuiKeyID['R'			]=KeyR;
		HuiKeyID['S'			]=KeyS;
		HuiKeyID['T'			]=KeyT;
		HuiKeyID['U'			]=KeyU;
		HuiKeyID['V'			]=KeyV;
		HuiKeyID['W'			]=KeyW;
		HuiKeyID['X'			]=KeyX;
		HuiKeyID['Y'			]=KeyY;
		HuiKeyID['Z'			]=KeyZ;
		HuiKeyID[VK_BACK		]=KeyBackspace;
		HuiKeyID[VK_TAB			]=KeyTab;
		HuiKeyID[VK_HOME		]=KeyHome;
		HuiKeyID[VK_NUMPAD0		]=KeyNum0;
		HuiKeyID[VK_NUMPAD1		]=KeyNum1;
		HuiKeyID[VK_NUMPAD2		]=KeyNum2;
		HuiKeyID[VK_NUMPAD3		]=KeyNum3;
		HuiKeyID[VK_NUMPAD4		]=KeyNum4;
		HuiKeyID[VK_NUMPAD5		]=KeyNum5;
		HuiKeyID[VK_NUMPAD6		]=KeyNum6;
		HuiKeyID[VK_NUMPAD7		]=KeyNum7;
		HuiKeyID[VK_NUMPAD8		]=KeyNum8;
		HuiKeyID[VK_NUMPAD9		]=KeyNum9;
		HuiKeyID[VK_ADD			]=KeyNumAdd;
		HuiKeyID[VK_SUBTRACT	]=KeyNumSubtract;
		HuiKeyID[VK_MULTIPLY	]=KeyNumMultiply;
		HuiKeyID[VK_DIVIDE		]=KeyNumDivide;
		HuiKeyID[VK_DECIMAL		]=KeyNumComma;
		//HuiKeyID[VK_RETURN	]=KeyNumEnter;
		HuiKeyID[188			]=KeyComma;
		HuiKeyID[190			]=KeyDot;
		HuiKeyID[226			]=KeySmaller;
		HuiKeyID[219			]=KeySz;
		HuiKeyID[222			]=KeyAe;
		HuiKeyID[192			]=KeyOe;
		HuiKeyID[186			]=KeyUe;
		
	#endif
	#ifdef HUI_API_GTK

		for (int k=0;k<HUI_NUM_KEYS;k++)
			HuiKeyID[k]=KeyID2[k]=0;

		HuiKeyID[KEY_LCONTROL]=GDK_Control_L;
		HuiKeyID[KEY_RCONTROL]=GDK_Control_R;
		HuiKeyID[KEY_LSHIFT]=GDK_Shift_L;
		HuiKeyID[KEY_RSHIFT]=GDK_Shift_R;
		HuiKeyID[KEY_LALT]=GDK_Alt_L;
		HuiKeyID[KEY_RALT]=GDK_Alt_R;
		HuiKeyID[KEY_ADD]=GDK_plus;
		HuiKeyID[KEY_SUBTRACT]=GDK_minus;
		HuiKeyID[KEY_FENCE]=GDK_numbersign;
		HuiKeyID[KEY_GRAVE]=GDK_asciicircum;
		HuiKeyID[KEY_HOME]=GDK_Home;
		HuiKeyID[KEY_END]=GDK_End;
		HuiKeyID[KEY_NEXT]=GDK_Page_Up;
		HuiKeyID[KEY_PRIOR]=GDK_Page_Down;
		HuiKeyID[KEY_UP]=GDK_Up;
		HuiKeyID[KEY_DOWN]=GDK_Down;
		HuiKeyID[KEY_LEFT]=GDK_Left;
		HuiKeyID[KEY_RIGHT]=GDK_Right;
		HuiKeyID[KEY_RETURN]=GDK_Return;
		HuiKeyID[KEY_ESCAPE]=GDK_Escape;
		HuiKeyID[KEY_INSERT]=GDK_Insert;
		HuiKeyID[KEY_DELETE]=GDK_Delete;
		HuiKeyID[KEY_SPACE]=GDK_space;
		HuiKeyID[KEY_F1]=GDK_F1;
		HuiKeyID[KEY_F2]=GDK_F2;
		HuiKeyID[KEY_F3]=GDK_F3;
		HuiKeyID[KEY_F4]=GDK_F4;
		HuiKeyID[KEY_F5]=GDK_F5;
		HuiKeyID[KEY_F6]=GDK_F6;
		HuiKeyID[KEY_F7]=GDK_F7;
		HuiKeyID[KEY_F8]=GDK_F8;
		HuiKeyID[KEY_F9]=GDK_F9;
		HuiKeyID[KEY_F10]=GDK_F10;
		HuiKeyID[KEY_F11]=GDK_F11;
		HuiKeyID[KEY_F12]=GDK_F12;
		HuiKeyID[KEY_1]=GDK_1;
		HuiKeyID[KEY_2]=GDK_2;
		HuiKeyID[KEY_3]=GDK_3;
		HuiKeyID[KEY_4]=GDK_4;
		HuiKeyID[KEY_5]=GDK_5;
		HuiKeyID[KEY_6]=GDK_6;
		HuiKeyID[KEY_7]=GDK_7;
		HuiKeyID[KEY_8]=GDK_8;
		HuiKeyID[KEY_9]=GDK_9;
		HuiKeyID[KEY_0]=GDK_0;
		HuiKeyID[KEY_A]=GDK_a;		KeyID2[KEY_A]=GDK_A;
		HuiKeyID[KEY_B]=GDK_b;		KeyID2[KEY_B]=GDK_B;
		HuiKeyID[KEY_C]=GDK_c;		KeyID2[KEY_C]=GDK_C;
		HuiKeyID[KEY_D]=GDK_d;		KeyID2[KEY_D]=GDK_D;
		HuiKeyID[KEY_E]=GDK_e;		KeyID2[KEY_E]=GDK_E;
		HuiKeyID[KEY_F]=GDK_f;		KeyID2[KEY_F]=GDK_F;
		HuiKeyID[KEY_G]=GDK_g;		KeyID2[KEY_G]=GDK_G;
		HuiKeyID[KEY_H]=GDK_h;		KeyID2[KEY_H]=GDK_H;
		HuiKeyID[KEY_I]=GDK_i;		KeyID2[KEY_I]=GDK_I;
		HuiKeyID[KEY_J]=GDK_j;		KeyID2[KEY_J]=GDK_J;
		HuiKeyID[KEY_K]=GDK_k;		KeyID2[KEY_K]=GDK_K;
		HuiKeyID[KEY_L]=GDK_l;		KeyID2[KEY_L]=GDK_L;
		HuiKeyID[KEY_M]=GDK_m;		KeyID2[KEY_M]=GDK_M;
		HuiKeyID[KEY_N]=GDK_n;		KeyID2[KEY_N]=GDK_N;
		HuiKeyID[KEY_O]=GDK_o;		KeyID2[KEY_O]=GDK_O;
		HuiKeyID[KEY_P]=GDK_p;		KeyID2[KEY_P]=GDK_P;
		HuiKeyID[KEY_Q]=GDK_q;		KeyID2[KEY_Q]=GDK_Q;
		HuiKeyID[KEY_R]=GDK_r;		KeyID2[KEY_R]=GDK_R;
		HuiKeyID[KEY_S]=GDK_s;		KeyID2[KEY_S]=GDK_S;
		HuiKeyID[KEY_T]=GDK_t;		KeyID2[KEY_T]=GDK_T;
		HuiKeyID[KEY_U]=GDK_u;		KeyID2[KEY_U]=GDK_U;
		HuiKeyID[KEY_V]=GDK_v;		KeyID2[KEY_V]=GDK_V;
		HuiKeyID[KEY_W]=GDK_w;		KeyID2[KEY_W]=GDK_W;
		HuiKeyID[KEY_X]=GDK_x;		KeyID2[KEY_X]=GDK_X;
		HuiKeyID[KEY_Y]=GDK_y;		KeyID2[KEY_Y]=GDK_Y;
		HuiKeyID[KEY_Z]=GDK_z;		KeyID2[KEY_Z]=GDK_Z;
		HuiKeyID[KEY_BACKSPACE]=GDK_BackSpace;
		HuiKeyID[KEY_TAB]=GDK_Tab;
		HuiKeyID[KEY_NUM_0]=GDK_KP_0;
		HuiKeyID[KEY_NUM_1]=GDK_KP_1;
		HuiKeyID[KEY_NUM_2]=GDK_KP_2;
		HuiKeyID[KEY_NUM_3]=GDK_KP_3;
		HuiKeyID[KEY_NUM_4]=GDK_KP_4;
		HuiKeyID[KEY_NUM_5]=GDK_KP_5;
		HuiKeyID[KEY_NUM_6]=GDK_KP_6;
		HuiKeyID[KEY_NUM_7]=GDK_KP_7;
		HuiKeyID[KEY_NUM_8]=GDK_KP_8;
		HuiKeyID[KEY_NUM_9]=GDK_KP_9;
		HuiKeyID[KEY_NUM_ADD]=GDK_KP_Add;
		HuiKeyID[KEY_NUM_SUBTRACT]=GDK_KP_Subtract;
		HuiKeyID[KEY_NUM_MULTIPLY]=GDK_KP_Multiply;
		HuiKeyID[KEY_NUM_DIVIDE]=GDK_KP_Divide;
		HuiKeyID[KEY_NUM_COMMA]=GDK_KP_Decimal;
		HuiKeyID[KEY_NUM_ENTER]=GDK_KP_Enter;
		HuiKeyID[KEY_COMMA]=GDK_comma;				KeyID2[KEY_COMMA]=GDK_semicolon;
		HuiKeyID[KEY_DOT]=GDK_period;
		HuiKeyID[KEY_SMALLER]=GDK_less;			KeyID2[KEY_AE]=GDK_greater;
		HuiKeyID[KEY_SZ]=GDK_ssharp;
		HuiKeyID[KEY_AE]=GDK_adiaeresis;			KeyID2[KEY_AE]=GDK_ae;
		HuiKeyID[KEY_OE]=GDK_odiaeresis;					KeyID2[KEY_AE]=GDK_oe;
		HuiKeyID[KEY_UE]=GDK_udiaeresis;
		HuiKeyID[KEY_LWINDOWS]=GDK_Super_L;
		HuiKeyID[KEY_RWINDOWS]=GDK_Super_R;
	#endif
}


void HuiAddKeyCode(const string &id, int key_code)
{
	HuiCommand c;
	c.key_code = key_code;
	c.id = id;
	c.enabled = true;
	c.type = 0;
	c.func = NULL;
	c.object = NULL;
	c.member_function = NULL;
	_HuiCommand_.add(c);
}

string HuiGetKeyName(int k)
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


string HuiGetKeyCodeName(int key_code)
{
	if (key_code < 0)
		return "";
	string n;
	if ((key_code & KEY_CONTROL) == KEY_CONTROL)
		n += "Ctrl+";
	if ((key_code & KEY_SHIFT) == KEY_SHIFT)
		n += "Shift+";
	n += HuiGetKeyName(key_code % 256);
	return n;
}

string HuiGetKeyChar(int key_code)
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

extern Array<CHuiWindow*> HuiWindow;

void HuiAddCommand(const string &id, const string &image, int default_key_code, hui_callback *func)
{
	HuiCommand c;
	c.type = 0;
	c.id = id;
	c.key_code = default_key_code;
	c.func = func;
	c.object = NULL;
	c.member_function = NULL;
	_HuiCommand_.add(c);

	foreach(HuiWindow, w)
		if (func)
			w->Event(id, func);
}

void HuiAddCommandToggle(const string &id, const string &image, int default_key_code, hui_callback *func)
{
	HuiCommand c;
	c.type = 1;
	c.id = id;
	c.key_code = default_key_code;
	c.func = func;
	c.object = NULL;
	c.member_function = NULL;
	_HuiCommand_.add(c);

	foreach(HuiWindow, w)
		if (func)
			w->Event(id, func);
}

void HuiAddCommandM(const string &id, const string &image, int default_key_code, CHuiWindow *object, void (CHuiWindow::*function)())
{
	HuiCommand c;
	c.type = 0;
	c.id = id;
	c.key_code = default_key_code;
	c.func = NULL;
	c.object = object;
	c.member_function = function;
	_HuiCommand_.add(c);

	foreach(HuiWindow, w)
		if ((object) && (function))
			w->EventM(id, object, function);
}

void HuiAddCommandMToggle(const string &id, const string &image, int default_key_code, CHuiWindow *object, void (CHuiWindow::*function)())
{
	HuiCommand c;
	c.type = 1;
	c.id = id;
	c.key_code = default_key_code;
	c.func = NULL;
	c.object = object;
	c.member_function = function;
	_HuiCommand_.add(c);

	foreach(HuiWindow, w)
		if ((object) && (function))
			w->EventM(id, object, function);
}

void HuiLoadKeyCodes(const string &filename)
{
}

void HuiSaveKeyCodes(const string &filename)
{
}
