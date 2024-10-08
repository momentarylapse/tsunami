#include "hui.h"
#include "internal.h"
#include "../os/msg.h"

// character set....
#ifdef OS_WINDOWS
	#include <windows.h>

	#define NUM_TCHAR_STRINGS				32
	#define TCHAR_STRING_LENGTH			1024
	static int cur_tchar_str=0;
	static TCHAR _tchar_str_[NUM_TCHAR_STRINGS][TCHAR_STRING_LENGTH];
	#define _get_tchar_str_()		_tchar_str_[(cur_tchar_str++)%NUM_TCHAR_STRINGS]

	const TCHAR *hui_tchar_str(const string &str) {
		#ifdef _UNICODE
			TCHAR *w=_get_tchar_str_();
			MultiByteToWideChar(CP_UTF8,0,(LPCSTR)str.c_str(),-1,w,TCHAR_STRING_LENGTH);
			return w;
		#else
			return str.c_str();
		#endif
	}

	//const TCHAR *hui_tchar_str_f(const string &str) {
		//return hui_tchar_str(str.sys_filename().c_str());
	//}

	string hui_de_tchar_str(const TCHAR *str) {
		#ifdef _UNICODE
			string s;
			s.resize(4096);
			int r = WideCharToMultiByte(CP_UTF8,0,str,-1,(LPSTR)s.data,s.num,nullptr,nullptr);
			s.resize(r);
			return s;
		#else
			return str;
		#endif
	}

	/*string de_sys_str_f(const TCHAR *str) {
		return hui_de_tchar_str(str).replace("/", "\\");
	}

	int _tchar_str_size_(TCHAR *str)
	{
		return _tcslen(str)*sizeof(TCHAR);
	}*/
#endif

#ifdef HUI_API_GTK

	const char *sys_str(const string &str)
	{	return str.c_str();	}

	const char *sys_str_f(const Path &str)
	{	return str.str().c_str();	}

	string de_sys_str(const char *str)
	{	return string(str);	}

	//const char *de_sys_str_f(const char *str)
	//{	return (char*)str;	}

	int _sys_str_size_(const char *str)
	{	return strlen(str);	}
#endif



namespace hui
{

// language
bool _using_language_;
Array<Language> _languages_;
Language *cur_lang = nullptr;


bool Language::Command::match(const string &_ns, const string &_id) {
	if (this->id != _id)
		return false;
	return ((this->_namespace == "") or (this->_namespace == _ns));
}


void set_language(const string &language) {
	cur_lang = nullptr;
	_using_language_ = false;
	for (Language &l: _languages_)
		if (l.name == language) {
			cur_lang = &l;
			_using_language_ = true;
		}
	if (!_using_language_)
		msg_error("HuiSetLanguage: language not found: " + language);

	UpdateAll();
}

// first try the specific namespace, then the global one
#define FOR_LANG_RET(NS, ID, CMD) \
for (auto &c: cur_lang->cmd) \
	if (c.match(NS, ID) and c._namespace != "") \
		return CMD; \
for (auto &c: cur_lang->cmd) \
	if (c.match(NS, ID)) \
		return CMD;

string get_language(const string &ns, const string &id) {
	if ((!_using_language_) or (id.num == 0))
		return "";
	for (auto &c: cur_lang->cmd)
		if (c.match(ns, id))
			return c.text;
	/*if (cur_lang->cmd[id].num == 0)
		return "???";*/
	return "";
}

string get_language_r(const string &ns, Resource &cmd) {
	string pre;
	if (cmd.options.num > 0)
		pre = "!" + implode(cmd.options, ",") + "\\";

	if ((!_using_language_) or (cmd.id.num == 0))
		return pre + cmd.title;

	FOR_LANG_RET(ns, cmd.id, pre + c.text);

	if (cmd.options.num > 0) {
		if (cmd.title.head(1) == "!")
			return "!" + implode(cmd.options, ",") + "," + cmd.title.sub(1);
		else
			return pre + cmd.title;
	}
	return pre + cmd.title;
}

// tooltip
string get_language_t(const string &ns, const string &id, const string &tooltip) {
	if (tooltip.num > 0)
		return tooltip;
	if ((!_using_language_) or (id.num == 0))
		return "";
	FOR_LANG_RET(ns, id, c.tooltip);
	return "";
}

// pre-translated...translations
string get_language_s(const string &str) {
	if (!_using_language_)
		return str;
	if (str.head(4) == ":##:")
		return get_language_s(str.sub_ref(4));
	for (Language::Translation &t: cur_lang->trans){
		if (str == t.orig)
			return t.trans;
	}
	return str;
}


string get_lang(const string &ns, const string &id, const string &text, bool allow_keys) {
	if (text.num > 0)
		return text;
	if ((!_using_language_) or (id.num == 0))
		return text;
	FOR_LANG_RET(ns, id, c.text);
	return text;
}

const char *get_lang_sys(const string &id, const string &text, bool allow_keys) {
	return sys_str(get_lang("", id, text, allow_keys));
}


void UpdateAll() {
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

Array<string> get_languages() {
	Array<string> n;
	for (Language &l: _languages_)
		n.add(l.name);
	return n;
}

string get_cur_language() {
	if (cur_lang)
		return cur_lang->name;
	return "";
}

};
