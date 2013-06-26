#include "hui.h"
#include "../base/map.h"

#ifdef OS_WINDOWS
	#include <tchar.h>
	int _tchar_str_size_(TCHAR *str);
#endif

static bool HuiConfigLoaded = false;
bool HuiConfigChanged = false;
static Map<string, string> HuiConfig;
typedef MapEntry<string, string> HuiConfigEntry;

static void LoadConfigFile()
{
	CFile *f = OpenFileSilent(HuiAppDirectory + "Data/config.txt");
	HuiConfig.clear();
	if (f){
		int num = f->ReadIntC();
		for (int i=0;i<num;i++){
			string temp = f->ReadStr();
			string key = temp.substr(3, temp.num - 3);
			string value = f->ReadStr();
			HuiConfig[key] = value;
		}
		FileClose(f);
	}
	HuiConfigLoaded = true;
}

void HuiSaveConfigFile()
{
	dir_create(HuiAppDirectory + "Data");
	CFile *f = CreateFileSilent(HuiAppDirectory + "Data/config.txt");
	f->WriteStr("// NumConfigs");
	f->WriteInt(HuiConfig.num);
	foreach(HuiConfigEntry &e, HuiConfig){
		f->WriteStr("// " + e.key);
		f->WriteStr(e.value);
	}
	f->WriteStr("#");
	FileClose(f);
	HuiConfigLoaded = true;
	HuiConfigChanged = false;
}

void HuiConfigWriteInt(const string &name, int val)
{
	HuiConfigWriteStr(name, i2s(val));
}

void HuiConfigWriteFloat(const string &name, float val)
{
	HuiConfigWriteStr(name, f2s(val, 6));
}

void HuiConfigWriteBool(const string &name, bool val)
{
	HuiConfigWriteStr(name, b2s(val));
}

void HuiConfigWriteStr(const string &name, const string &str)
{
	HuiConfig[name] = str;
	HuiConfigChanged = true;
	//SaveConfigFile();
}

int HuiConfigReadInt(const string &name, int default_val)
{
	return s2i(HuiConfigReadStr(name, i2s(default_val)));
}

float HuiConfigReadFloat(const string &name, float default_val)
{
	return s2f(HuiConfigReadStr(name, f2s(default_val, 6)));
}

bool HuiConfigReadBool(const string &name, bool default_val)
{
	return (HuiConfigReadStr(name, b2s(default_val)) == "true");
}

string HuiConfigReadStr(const string &name, const string &default_str)
{
	if (!HuiConfigLoaded)
		LoadConfigFile();
	if (HuiConfig.contains(name))
		return HuiConfig[name];
	return default_str;
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
