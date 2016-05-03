/*----------------------------------------------------------------------------*\
| Hui language                                                                 |
| -> string format conversion                                                  |
| -> translations                                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_LANG_EXISTS_
#define _HUI_LANG_EXISTS_

class HuiResource;


//----------------------------------------------------------------------------------
// string conversion

#ifdef OS_WINDOWS
	extern const TCHAR *hui_tchar_str(const string &str);
	extern const TCHAR *hui_tchar_str_f(const string &str);
	extern string hui_de_tchar_str(const TCHAR *str);
	extern string hui_de_tchar_str_f(const TCHAR *str);
	#ifdef UNICODE
		#define win_str LPWSTR
	#else
		#define win_str LPSTR
	#endif
#endif
#ifdef HUI_API_WIN
	#define sys_str			hui_tchar_str
	#define sys_str_f		hui_tchar_str_f
	#define de_sys_str		hui_de_tchar_str
	#define de_sys_str_f	hui_de_tchar_str_f
#else
	extern const char *sys_str(const string &str);
	extern const char *sys_str_f(const string &str);
	extern string de_sys_str(const char *str);
	extern string de_sys_str_f(const char *str);
#endif
	string str_m_to_utf8(const string &str);
	string str_utf8_to_m(const string &str);
	/*extern const char *str_ascii2m(const char *str);
	extern const char *str_m2ascii(const char *str);*/

	extern string get_lang(const string &ns,const string &id,const string &text,bool allow_keys=false);
#ifdef HUI_API_WIN
	extern const TCHAR *get_lang_sys(const string &id,const string &text,bool allow_keys=false);
#else
	extern const char *get_lang_sys(const string &id,const string &text,bool allow_keys=false);
#endif



// language
Array<string> HuiGetLanguages();
string HuiGetCurLanguage();
void _cdecl HuiSetLanguage(const string &language);
extern bool HuiLanguaged;

string _cdecl HuiGetLanguage(const string &ns, const string &id);
string _cdecl HuiGetLanguageR(const string &ns, HuiResource &cmd);
string _cdecl HuiGetLanguageT(const string &ns, const string &id);
string _cdecl HuiGetLanguageS(const string &str);
#define L(ns, id)	HuiGetLanguage(ns, id)
#define _(str)	HuiGetLanguageS(str_m_to_utf8(str))
void _cdecl HuiUpdateAll();

// internal
struct HuiLanguageTranslation
{
	string orig;
	string trans; // pre defined translation of orig
};

struct HuiLanguageCommand
{
	string _namespace, id, text, tooltip;
	bool match(const string &_ns, const string &id);
};

struct HuiLanguage
{
	string name;
	Array<HuiLanguageCommand> cmd; // text associated to ids
	Array<HuiLanguageTranslation> trans;
};

#endif
