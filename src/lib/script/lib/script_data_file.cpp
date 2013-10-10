#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

namespace Script{

extern Type *TypeStringList;
extern Type *TypeBoolList;

static Date *_date;
#define	GetDADate(x)			long(&_date->x)-long(_date)
static DirEntry *_dir_entry;
#define	GetDADirEntry(x)			long(&_dir_entry->x)-long(_dir_entry)

class DirEntryList : public Array<DirEntry>
{
public:
	void _cdecl __assign__(const DirEntryList &o)
	{	*this = o;	}
	string _cdecl str()
	{
		string s = "[";
		for (int i=0;i<num;i++){
			if (i > 0)
				s += ", ";
			s += (*this)[i].str();
		}
		s += "]";
		return s;
	}
};

void SIAddPackageFile()
{
	msg_db_f("SIAddPackageFile", 3);

	add_package("file", false);

	Type*
	TypeFile			= add_type  ("File", 0);
	Type*
	TypeFileP			= add_type_p("File*", TypeFile);
	Type*
	TypeDate			= add_type  ("Date", sizeof(Date));
	Type*
	TypeDirEntry		= add_type  ("DirEntry", sizeof(DirEntry));
	Type*
	TypeDirEntryList	= add_type_a("DirEntry[]", TypeDirEntry, -1);


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
		class_add_func("format",			TypeString,		mf(&Date::format));
			func_add_param("f",	TypeString);
		class_add_func("str",				TypeString,		mf(&Date::str));
	
	add_class(TypeFile);
		class_add_func("GetDateCreation",		TypeDate,		mf(&CFile::GetDateCreation));
		class_add_func("GetDateModification",		TypeDate,		mf(&CFile::GetDateModification));
		class_add_func("GetDateAccess",		TypeDate,		mf(&CFile::GetDateAccess));
		class_add_func("GetSize",		TypeInt,		mf(&CFile::GetSize));
		class_add_func("GetPos",		TypeInt,		mf(&CFile::GetPos));
		class_add_func("SetPos",		TypeVoid,		mf(&CFile::SetPos));
			func_add_param("pos",		TypeInt);
			func_add_param("absolute",	TypeBool);
		class_add_func("SetBinaryMode",	TypeVoid,		mf(&CFile::SetBinaryMode));
			func_add_param("binary",	TypeBool);
		class_add_func("WriteBool",		TypeVoid,			mf(&CFile::WriteBool));
			func_add_param("b",			TypeBool);
		class_add_func("WriteInt",		TypeVoid,			mf(&CFile::WriteInt));
			func_add_param("i",			TypeInt);
		class_add_func("WriteFloat",	TypeVoid,			mf(&CFile::WriteFloat));
			func_add_param("x",			TypeFloat);
		class_add_func("WriteStr",		TypeVoid,			mf(&CFile::WriteStr));
			func_add_param("s",			TypeString);
		class_add_func("ReadBool",		TypeBool,			mf(&CFile::ReadBool));
		class_add_func("ReadInt",		TypeInt,				mf(&CFile::ReadInt));
		class_add_func("ReadFloat",		TypeFloat,			mf(&CFile::ReadFloat));
		class_add_func("ReadStr",		TypeString,			mf(&CFile::ReadStr));
		class_add_func("ReadBoolC",		TypeBool,			mf(&CFile::ReadBoolC));
		class_add_func("ReadIntC",		TypeInt,			mf(&CFile::ReadIntC));
		class_add_func("ReadFloatC",	TypeFloat,			mf(&CFile::ReadFloatC));
		class_add_func("ReadStrC",		TypeString,			mf(&CFile::ReadStrC));
		class_add_func("ReadComplete",	TypeString,			mf(&CFile::ReadComplete));

	
	add_class(TypeDirEntry);
		class_add_element("name",			TypeString,		GetDADirEntry(name));
		class_add_element("size",			TypeInt,		GetDADirEntry(size));
		class_add_element("is_dir",			TypeBool,		GetDADirEntry(is_dir));
		class_add_func("__init__",		TypeVoid,			mf(&DirEntry::__init__));
		class_add_func("__assign__",		TypeVoid,			mf(&DirEntry::__assign__));
			func_add_param("other",		TypeDirEntry);
		class_add_func("str",		TypeString,			mf(&DirEntry::str));
	
	add_class(TypeDirEntryList);
		class_add_func("__init__",		TypeVoid,			mf(&DirEntryList::__init__));
		class_add_func("__assign__",		TypeVoid,			mf(&DirEntryList::__assign__));
			func_add_param("other",		TypeDirEntryList);
		class_add_func("str",		TypeString,			mf(&DirEntryList::str));


	// file access
	add_func("FileOpen",			TypeFileP,				(void*)&FileOpen);
		func_add_param("filename",		TypeString);
	add_func("FileCreate",			TypeFileP,				(void*)&FileCreate);
		func_add_param("filename",		TypeString);
	add_func("FileRead",			TypeString,				(void*)&FileRead);
		func_add_param("filename",		TypeString);
	add_func("FileClose",			TypeVoid,				(void*)&FileClose);
		func_add_param("f",		TypeFileP);
	add_func("FileExists",			TypeBool,		(void*)&file_test_existence);
		func_add_param("filename",		TypeString);
	add_func("FileRename",			TypeBool,			(void*)&file_rename);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("FileCopy",			TypeBool,			(void*)&file_copy);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("FileDelete",			TypeBool,			(void*)&file_delete);
		func_add_param("filename",		TypeString);
	add_func("DirSearch",			TypeDirEntryList,			(void*)&dir_search);
		func_add_param("dir",		TypeString);
		func_add_param("filter",		TypeString);
		func_add_param("show_dirs",		TypeBool);
	add_func("DirCreate",			TypeBool,			(void*)&dir_create);
		func_add_param("dir",		TypeString);
	add_func("DirDelete",			TypeBool,			(void*)&dir_delete);
		func_add_param("dir",		TypeString);
	add_func("GetCurDir",			TypeString,			(void*)&get_current_dir);
}

};
