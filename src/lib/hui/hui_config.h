/*----------------------------------------------------------------------------*\
| Hui config                                                                   |
| -> configuration database for hui                                            |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_CONFIG_EXISTS_
#define _HUI_CONFIG_EXISTS_




// configuration
void _cdecl HuiConfigWriteInt(const string &name, int val);
void _cdecl HuiConfigWriteFloat(const string &name, float val);
void _cdecl HuiConfigWriteBool(const string &name, bool val);
void _cdecl HuiConfigWriteStr(const string &name, const string &str);
int _cdecl HuiConfigReadInt(const string &name, int default_val = 0);
float _cdecl HuiConfigReadFloat(const string &name, float default_val = 0);
bool _cdecl HuiConfigReadBool(const string &name, bool default_val = false);
string _cdecl HuiConfigReadStr(const string &name, const string &default_str);
void _cdecl HuiSaveConfigFile();
void _cdecl HuiRegisterFileType(const string &ending, const string &description, const string &icon_path, const string &open_with, const string &command_name, bool set_default);

#endif
