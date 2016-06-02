/*
 * HuiConfig.cpp
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#include "HuiConfig.h"
#include "hui.h"


#ifdef OS_WINDOWS
	#include <tchar.h>
	int _tchar_str_size_(TCHAR *str);
#endif

HuiConfiguration HuiConfig;

HuiConfiguration::HuiConfiguration()
{
	filename = "";
	loaded = false;
	changed = false;
}

HuiConfiguration::HuiConfiguration(const string &_filename)
{
	filename = _filename;
	loaded = false;
	changed = false;
}

HuiConfiguration::~HuiConfiguration()
{
}

void HuiConfiguration::__init__()
{
	new(this) HuiConfiguration;
}

void HuiConfiguration::__init_ext__(const string &_filename)
{
	new(this) HuiConfiguration(_filename);
}

void HuiConfiguration::__del__()
{
	this->~HuiConfiguration();
}

void HuiConfiguration::setInt(const string& name, int val)
{
	setStr(name, i2s(val));
}

void HuiConfiguration::setFloat(const string& name, float val)
{
	setStr(name, f2s(val, 6));
}

void HuiConfiguration::setBool(const string& name, bool val)
{
	setStr(name, b2s(val));
}

void HuiConfiguration::setStr(const string& name, const string& str)
{
	map[name] = str;
	changed = true;
}

int HuiConfiguration::getInt(const string& name, int default_val)
{
	return s2i(getStr(name, i2s(default_val)));
}

float HuiConfiguration::getFloat(const string& name, float default_val)
{
	return s2f(getStr(name, f2s(default_val, 6)));
}

bool HuiConfiguration::getBool(const string& name, bool default_val)
{
	return (getStr(name, b2s(default_val)) == "true");
}

string HuiConfiguration::getStr(const string& name, const string& default_str)
{
	if (!loaded)
		load();
	if (map.contains(name))
		return map[name];
	return default_str;
}

void HuiConfiguration::load()
{
	File *f = FileOpenSilent(filename);
	map.clear();
	if (f){
		f->ReadComment();
		int num = f->ReadInt();
		for (int i=0;i<num;i++){
			string temp = f->ReadStr();
			string key = temp.substr(3, temp.num - 3);
			string value = f->ReadStr();
			map[key] = value;
		}
		FileClose(f);
	}
	loaded = true;
	changed = false;
}

void HuiConfiguration::save()
{
	dir_create(filename.dirname());
	File *f = FileCreateSilent(filename);
	f->WriteStr("// NumConfigs");
	f->WriteInt(map.num);
	for (HuiConfigEntry &e : map){
		f->WriteStr("// " + e.key);
		f->WriteStr(e.value);
	}
	f->WriteStr("#");
	FileClose(f);
	loaded = true;
	changed = false;
}






#ifdef OS_WINDOWS
	TCHAR t_dot_end[256],t_desc[256],t_dot_end_desc[256],t_desc_shell[256],t_cmd[256],t_desc_shell_cmd[256],t_desc_shell_cmd_cmd[256],t_cmd_line[256],t_desc_icon[256],t_icon_0[256];
#endif

void HuiRegisterFileType(const string &ending, const string &description, const string &icon_path, const string &open_with, const string &command_name, bool set_default)
{
#if 0
#ifdef OS_WINDOWS
	_tcscpy(t_dot_end, hui_tchar_str("." + ending));
	_tcscpy(t_desc, hui_tchar_str(description));
	_tcscpy(t_dot_end_desc, hui_tchar_str("." + ending + "\\" + description));
	_tcscpy(t_desc_shell, hui_tchar_str(description + "\\shell"));
	_tcscpy(t_cmd, hui_tchar_str(command_name));
	_tcscpy(t_desc_shell_cmd, hui_tchar_str(description + "\\shell\\" + command_name));
	_tcscpy(t_desc_shell_cmd_cmd, hui_tchar_str(description + "\\shell\\" + command_name + "\\command"));
	_tcscpy(t_cmd_line, hui_tchar_str("\"" + open_with + "\" %1"));
	_tcscpy(t_desc_icon, hui_tchar_str(description + "\\DefaultIcon"));
	HKEY hkey;

	// $ending -> $description
	RegCreateKeyEx(HKEY_CLASSES_ROOT,t_dot_end,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
	RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_desc,_tchar_str_size_(t_desc));

	// $ending\$description -> $description
	RegCreateKeyEx(HKEY_CLASSES_ROOT,t_dot_end_desc,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
	RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_desc,_tchar_str_size_(t_desc));

	// $description -> $description
	RegCreateKeyEx(HKEY_CLASSES_ROOT,t_desc,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
	RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_desc,_tchar_str_size_(t_desc));

	if (open_with.num > 0){
		// $description\shell -> $command_name
		RegCreateKeyEx(HKEY_CLASSES_ROOT,t_desc_shell,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
		if (set_default)
			RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_cmd,_tchar_str_size_(t_cmd));

		// $description\shell\$command_name\command -> "$open_with" %1
		RegCreateKeyEx(HKEY_CLASSES_ROOT,t_desc_shell_cmd,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
		RegCreateKeyEx(HKEY_CLASSES_ROOT,t_desc_shell_cmd_cmd,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
		RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_cmd_line,_tchar_str_size_(t_cmd_line));
	}
	if (icon_path.num > 0){
		_tcscpy(t_icon_0, hui_tchar_str(icon_path + ",0"));
		// $description\DefaultIcon -> $icon_path,0
		RegCreateKeyEx(HKEY_CLASSES_ROOT,t_desc_icon,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,NULL);
		RegSetValueEx(hkey,_T(""),0,REG_SZ,(BYTE*)t_icon_0,_tchar_str_size_(t_icon_0));
	}
#endif
#endif
}

