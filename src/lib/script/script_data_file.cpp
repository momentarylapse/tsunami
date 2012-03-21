#include "../file/file.h"
#include "script.h"
#include "../00_config.h"
#include "script_data_common.h"

extern sType *TypeStringList;
extern sType *TypeBoolList;

static Date *_date;
#define	GetDADate(x)			long(&_date->x)-long(_date)

void SIAddPackageFile()
{
	msg_db_r("SIAddPackageFile", 3);

	set_cur_package("file");

	sType*
	TypeFile			= add_type  ("File",		0);
	sType*
	TypeFileP			= add_type_p("file",		TypeFile);
	sType*
	TypeDate			= add_type  ("Date",		sizeof(Date));


	add_class(TypeDate);
		class_add_element("time",			TypeInt,		GetDADate(time));
		class_add_element("year",			TypeInt,		GetDADate(year));
		class_add_element("month",			TypeInt,		GetDADate(month));
		class_add_element("day",			TypeInt,		GetDADate(day));
		class_add_element("hour",			TypeInt,		GetDADate(hour));
		class_add_element("minute",			TypeInt,		GetDADate(minute));
		class_add_element("second",			TypeInt,		GetDADate(second));
		class_add_element("milli_second",	TypeInt,		GetDADate(milli_second));
		class_add_element("day_of_week",	TypeInt,		GetDADate(day_of_week));
		class_add_element("day_of_year",	TypeInt,		GetDADate(day_of_year));
	
	add_class(TypeFile);
		class_add_func("GetDate",		TypeDate,		mf((tmf)&CFile::GetDate));
			func_add_param("type",		TypeInt);
		class_add_func("GetSize",		TypeInt,		mf((tmf)&CFile::GetSize));
		class_add_func("GetPos",		TypeInt,		mf((tmf)&CFile::GetPos));
		class_add_func("SetPos",		TypeVoid,		mf((tmf)&CFile::SetPos));
			func_add_param("pos",		TypeInt);
			func_add_param("absolute",	TypeBool);
		class_add_func("SetBinaryMode",	TypeVoid,		mf((tmf)&CFile::SetBinaryMode));
			func_add_param("binary",	TypeBool);
		class_add_func("WriteBool",		TypeVoid,			mf((tmf)&CFile::WriteBool));
			func_add_param("b",			TypeBool);
		class_add_func("WriteInt",		TypeVoid,			mf((tmf)&CFile::WriteInt));
			func_add_param("i",			TypeInt);
		class_add_func("WriteFloat",	TypeVoid,			mf((tmf)&CFile::WriteFloat));
			func_add_param("x",			TypeFloat);
		class_add_func("WriteStr",		TypeVoid,			mf((tmf)&CFile::WriteStr));
			func_add_param("s",			TypeString);
		class_add_func("ReadBool",		TypeBool,			mf((tmf)&CFile::ReadBool));
		class_add_func("ReadInt",		TypeInt,				mf((tmf)&CFile::ReadInt));
		class_add_func("ReadFloat",		TypeFloat,			mf((tmf)&CFile::ReadFloat));
		class_add_func("ReadStr",		TypeString,			mf((tmf)&CFile::ReadStr));
		class_add_func("ReadBoolC",		TypeBool,			mf((tmf)&CFile::ReadBoolC));
		class_add_func("ReadIntC",		TypeInt,			mf((tmf)&CFile::ReadIntC));
		class_add_func("ReadFloatC",	TypeFloat,			mf((tmf)&CFile::ReadFloatC));
		class_add_func("ReadStrC",		TypeString,			mf((tmf)&CFile::ReadStrC));
		class_add_func("ReadComplete",	TypeString,			mf((tmf)&CFile::ReadComplete));


	// file access
	add_func("FileOpen",			TypeFileP,				(void*)&OpenFile);
		func_add_param("filename",		TypeString);
	add_func("FileCreate",			TypeFileP,				(void*)&CreateFile);
		func_add_param("filename",		TypeString);
	add_func("FileRead",			TypeString,				(void*)&FileRead);
		func_add_param("filename",		TypeString);
	add_func("FileClose",			TypeBool,				(void*)&FileClose);
		func_add_param("f",		TypeFileP);
	add_func("FileExists",			TypeBool,		(void*)&file_test_existence);
		func_add_param("filename",		TypeString);
	add_func("FileRename",			TypeBool,			(void*)&file_rename);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("FileCopy",			TypeBool,			(void*)&file_copy);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("DirSearch",			TypeInt,			(void*)&dir_search);
		func_add_param("dir",		TypeString);
		func_add_param("filter",		TypeString);
		func_add_param("show_dirs",		TypeBool);
	add_func("DirCreate",			TypeBool,			(void*)&dir_create);
		func_add_param("dir",		TypeString);
	add_func("DirDelete",			TypeBool,			(void*)&dir_delete);
		func_add_param("dir",		TypeString);
	add_func("GetCurDir",			TypeString,			(void*)&get_current_dir);
	add_func("Dirname",				TypeString,			(void*)&dirname);
		func_add_param("path",		TypeString);
	add_func("Basename",			TypeString,			(void*)(string (*)(const string&))&basename);
		func_add_param("path",		TypeString);
	
	add_ext_var("DirSearchName",	TypeStringList,	&dir_search_name);
	add_ext_var("DirSearchIsDir",	TypeBoolList,	&dir_search_is_dir);

	// file date
	add_const("FileDateModification", TypeInt, (void*)FileDateModification);
	add_const("FileDateAccess",       TypeInt, (void*)FileDateAccess);
	add_const("FileDateCreation",     TypeInt, (void*)FileDateCreation);


	msg_db_l(3);
}
