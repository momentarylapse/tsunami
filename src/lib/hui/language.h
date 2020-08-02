/*----------------------------------------------------------------------------*\
| Hui language                                                                 |
| -> string format conversion                                                  |
| -> translations                                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _LANG_EXISTS_
#define _LANG_EXISTS_

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
	extern const char *sys_str_f(const Path &str);
	extern string de_sys_str(const char *str);
	extern Path de_sys_str_f(const char *str);
#endif
	/*extern const char *str_ascii2m(const char *str);
	extern const char *str_m2ascii(const char *str);*/



namespace hui
{


extern string get_lang(const string &ns,const string &id,const string &text,bool allow_keys=false);
#ifdef HUI_API_WIN
	extern const TCHAR *get_lang_sys(const string &id,const string &text,bool allow_keys=false);
#else
	extern const char *get_lang_sys(const string &id,const string &text,bool allow_keys=false);
#endif

class Resource;


// language
Array<string> GetLanguages();
string GetCurLanguage();
void _cdecl SetLanguage(const string &language);
extern bool _using_language_;

string _cdecl GetLanguage(const string &ns, const string &id);
string _cdecl GetLanguageR(const string &ns, Resource &cmd);
string _cdecl GetLanguageT(const string &ns, const string &id, const string &tooltip);
string _cdecl GetLanguageS(const string &str);
#define L(ns, id)	hui::GetLanguage(ns, id)
#define _(str)	hui::GetLanguageS(str_m_to_utf8(str))
void _cdecl UpdateAll();

// internal

struct Language
{
	struct Translation
	{
		string orig;
		string trans; // pre defined translation of orig
	};

	struct Command
	{
		string _namespace, id, text, tooltip;
		bool match(const string &_ns, const string &id);
	};

	string name;
	Array<Command> cmd; // text associated to ids
	Array<Translation> trans;
};

};

#endif
