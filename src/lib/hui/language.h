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

class Path;

//----------------------------------------------------------------------------------
// string conversion

#ifdef OS_WINDOWS
	/*extern const TCHAR *hui_tchar_str(const string &str);
	extern const TCHAR *hui_tchar_str_f(const string &str);
	extern string hui_de_tchar_str(const TCHAR *str);
	extern string hui_de_tchar_str_f(const TCHAR *str);
	#ifdef UNICODE
		#define win_str LPWSTR
	#else
		#define win_str LPSTR
	#endif*/
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



namespace hui {


extern string get_lang(const string &ns, const string &id, const string &text, bool allow_keys=false);
extern const char *get_lang_sys(const string &id, const string &text, bool allow_keys=false);

class Resource;


// language
Array<string> get_languages();
string get_cur_language();
void _cdecl set_language(const string &language);
extern bool _using_language_;

string _cdecl get_language(const string &ns, const string &id);
string _cdecl get_language_r(const string &ns, Resource &cmd);
string _cdecl get_language_t(const string &ns, const string &id, const string &tooltip);
string _cdecl get_language_s(const string &str);
#define L(ns, id)	hui::get_language(ns, id)
#define _(str)	hui::get_language_s(str)
void _cdecl UpdateAll();

// internal

struct Language {
	struct Translation {
		string orig;
		string trans; // pre defined translation of orig
	};

	struct Command {
		string _namespace, id, text, tooltip;
		bool match(const string &_ns, const string &id);
	};

	string name;
	Array<Command> cmd; // text associated to ids
	Array<Translation> trans;
};

};

#endif
