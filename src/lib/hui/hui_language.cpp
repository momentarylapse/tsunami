#include "hui.h"
#include "hui_internal.h"

// character set....
#ifdef OS_WINDOWS

	#include <tchar.h>

	#define NUM_TCHAR_STRINGS				32
	#define TCHAR_STRING_LENGTH			1024
	static int cur_tchar_str=0;
	static TCHAR _tchar_str_[NUM_TCHAR_STRINGS][TCHAR_STRING_LENGTH];
	#define _get_tchar_str_()		_tchar_str_[(cur_tchar_str++)%NUM_TCHAR_STRINGS]

	const TCHAR *hui_tchar_str(const string &str)
	{
		#ifdef _UNICODE
			TCHAR *w=_get_tchar_str_();
			MultiByteToWideChar(CP_UTF8,0,(LPCSTR)str.c_str(),-1,w,TCHAR_STRING_LENGTH);
			return w;
		#else
			return str.c_str();
		#endif
	}

	const TCHAR *hui_tchar_str_f(const string &str)
	{
		return hui_tchar_str(str.sys_filename().c_str());
	}

	string hui_de_tchar_str(const TCHAR *str)
	{
		#ifdef _UNICODE
			string s;
			s.resize(4096);
			int r = WideCharToMultiByte(CP_UTF8,0,str,-1,(LPSTR)s.data,s.num,NULL,NULL);
			s.resize(r);
			return s;
		#else
			return str;
		#endif
	}

	string de_sys_str_f(const TCHAR *str)
	{
		return hui_de_tchar_str(str).replace("/", "\\");
	}

	int _tchar_str_size_(TCHAR *str)
	{
		return _tcslen(str)*sizeof(TCHAR);
	}
#endif

#ifdef HUI_API_GTK

	const char *sys_str(const string &str)
	{	return str.c_str();	}

	const char *sys_str_f(const string &str)
	{	return str.sys_filename().c_str();	}

	string de_sys_str(const char *str)
	{	return string(str);	}

	//const char *de_sys_str_f(const char *str)
	//{	return (char*)str;	}

	int _sys_str_size_(const char *str)
	{	return strlen(str);	}
#endif


	string str_m_to_utf8(const string &str)
	{
		string r;
		for (int i=0;i<str.num;i++){
			if ((str[i] == '&') and (i < str.num - 1)){
				if (str[i+1]=='a'){
					r.add(0xc3);
					r.add(0xa4);
				}else if (str[i+1]=='o'){
					r.add(0xc3);
					r.add(0xb6);
				}else if (str[i+1]=='u'){
					r.add(0xc3);
					r.add(0xbc);
				}else if (str[i+1]=='s'){
					r.add(0xc3);
					r.add(0x9f);
				}else if (str[i+1]=='A'){
					r.add(0xc3);
					r.add(0x84);
				}else if (str[i+1]=='O'){
					r.add(0xc3);
					r.add(0x96);
				}else if (str[i+1]=='U'){
					r.add(0xc3);
					r.add(0x9c);
				}else if (str[i+1]=='&'){
					r.add('&');
				}else{
					r.add(str[i]);
					i --;
				}
				i ++;
			}else
				r.add(str[i]);
		}
		return r;
	}

	// Umlaute zu Vokalen mit & davor zerlegen
	string str_utf8_to_m(const string &str)
	{
		string r;
		const unsigned char *us = (const unsigned char*)str.c_str();

		for (int i=0;i<str.num;i++){
			if ((us[i]==0xc3) and (us[i+1]==0xa4)){
				r += "&a";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0xb6)){
				r += "&o";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0xbc)){
				r += "&u";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0x9f)){
				r += "&s";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0x84)){
				r += "&A";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0x96)){
				r += "&O";
				i ++;
			}else if ((us[i]==0xc3) and (us[i+1]==0x9c)){
				r += "&U";
				i ++;
			}else if (us[i]=='&'){
				r += "&&";
			}else
				r.add(str[i]);
		}
		return r;
	}

/*	// Umlaute zu Vokalen mit & davor zerlegen
	const char *str_ascii2m(const char *str)
	{
		unsigned char *us=(unsigned char *)str;
		char *ss=_file_get_str_();

		int l=0;
		for (unsigned int i=0;i<strlen(str)+1;i++){
			ss[l]=str[i];
			if (us[i]==0xe4){	ss[l]='&';	ss[l+1]='a';	l++;	}
			if (us[i]==0xf6){	ss[l]='&';	ss[l+1]='o';	l++;	}
			if (us[i]==0xfc){	ss[l]='&';	ss[l+1]='u';	l++;	}
			if (us[i]==0xdf){	ss[l]='&';	ss[l+1]='s';	l++;	}
			if (us[i]==0xc4){	ss[l]='&';	ss[l+1]='A';	l++;	}
			if (us[i]==0xd6){	ss[l]='&';	ss[l+1]='O';	l++;	}
			if (us[i]==0xdc){	ss[l]='&';	ss[l+1]='U';	l++;	}
			if (us[i]=='&'){	ss[l]='&';	ss[l+1]='&';	l++;	}
			if (us[i]=='\r')	continue;
			l++;
		}
		return ss;
	}

	const char *str_m2ascii(const char *str)
	{
		char *ss=_file_get_str_();
		unsigned char *us=(unsigned char *)ss;

		int l=0;
		for (unsigned int i=0;i<strlen(str)+1;i++){
			ss[l]=str[i];
			if (str[i]=='&'){
				if      (str[i+1]=='a'){	us[l]=0xe4;	i++;	}
				else if (str[i+1]=='o'){	us[l]=0xf6;	i++;	}
				else if (str[i+1]=='u'){	us[l]=0xfc;	i++;	}
				else if (str[i+1]=='s'){	us[l]=0xdf;	i++;	}
				else if (str[i+1]=='A'){	us[l]=0xc4;	i++;	}
				else if (str[i+1]=='O'){	us[l]=0xd6;	i++;	}
				else if (str[i+1]=='U'){	us[l]=0xdc;	i++;	}
				else if (str[i+1]=='&'){	us[l]='&';	i++;	}
			}
			l++;
		}
		return ss;
	}*/

// language
bool HuiLanguaged;
Array<HuiLanguage> _HuiLanguage_;
HuiLanguage *cur_lang = NULL;


bool HuiLanguageCommand::match(const string &_ns, const string &_id)
{
	if (this->id != _id)
		return false;
	return ((this->_namespace.num == 0) or (this->_namespace == _ns));
}


void HuiSetLanguage(const string &language)
{
	msg_db_f("HuiSetLang", 1);
	cur_lang = NULL;
	HuiLanguaged = false;
	for (HuiLanguage &l : _HuiLanguage_)
		if (l.name == language){
			cur_lang = &l;
			HuiLanguaged = true;
		}
	if (!HuiLanguaged)
		msg_error("HuiSetLanguage: language not found: " + language);

	HuiUpdateAll();
}

string HuiGetLanguage(const string &ns, const string &id)
{
	if ((!HuiLanguaged) or (id.num == 0))
		return "";
	for (HuiLanguageCommand &c : cur_lang->cmd)
		if (c.match(ns, id))
			return c.text;
	/*if (cur_lang->cmd[id].num == 0)
		return "???";*/
	return "";
}

string HuiGetLanguageR(const string &ns, HuiResource &cmd)
{
	if ((!HuiLanguaged) or (cmd.id.num == 0))
		return "";
	for (HuiLanguageCommand &c : cur_lang->cmd)
		if (c.match(ns, cmd.id)){
			if (cmd.options.num > 0)
				return "!" + implode(cmd.options, ",") + "\\" + c.text;
			return c.text;
		}
	if (cmd.options.num > 0)
		return "!" + implode(cmd.options, ",") + "\\";
	return "";
}

string HuiGetLanguageT(const string &ns, const string &id)
{
	if ((!HuiLanguaged) or (id.num == 0))
		return "";
	for (HuiLanguageCommand &c : cur_lang->cmd)
		if (c.match(ns, id))
			return c.tooltip;
	return "";
}

// pre-translated...translations
string HuiGetLanguageS(const string &str)
{
	if (!HuiLanguaged)
		return str;
	for (HuiLanguageTranslation &t : cur_lang->trans){
		if (str == t.orig)
			return t.trans;
	}
	return str;
}


string get_lang(const string &ns, const string &id, const string &text, bool allow_keys)
{
	if (text.num > 0)
		return text;
	if ((!HuiLanguaged) or (id.num == 0))
		return text;
	string r = "";
	for (HuiLanguageCommand &c : cur_lang->cmd)
		if (c.match(ns, id))
			r = c.text;
	if (r.num == 0)
		return text;
#ifdef HUI_API_WIN
	if (allow_keys)
		for (int i=0;i<_HuiCommand_.num;i++)
			if (id == _HuiCommand_[i].id)
				return r + "\t", HuiGetKeyCodeName(_HuiCommand_[i].key_code);
#endif
	return r;
}

#ifdef HUI_API_WIN
	const TCHAR *get_lang_sys(const string &id, const string &text, bool allow_keys)
#else
	const char *get_lang_sys(const string &id, const string &text, bool allow_keys)
#endif
{
	return sys_str(get_lang("", id, text, allow_keys));
}


void HuiUpdateAll()
{
	msg_db_f("HuiUpdateAll", 1);
/*	// update windows
	for (int i=0;i<HuiWindow.num;i++){
		for (int j=0;j<HuiWindow[i]->Control.num;j++){
			string id = HuiWindow[i]->Control[j]->ID;
			if (cur_lang->Text[id].num > 0)
				HuiWindow[i]->SetString(id,HuiGetLanguage(id));
		}

		// update menu
		if (HuiWindow[i]->Menu)
			UpdateMenuLanguage(HuiWindow[i]->Menu);
	}*/
}

Array<string> HuiGetLanguages()
{
	Array<string> n;
	for (HuiLanguage &l : _HuiLanguage_)
		n.add(l.name);
	return n;
}

string HuiGetCurLanguage()
{
	if (cur_lang)
		return cur_lang->name;
	return "";
}

