#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

namespace Script{

class vector;

extern Type *TypeIntPs;
extern Type *TypeFloatPs;
extern Type *TypeBoolPs;
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

class KabaFile : public File
{
public:
	void _cdecl __delete__()
	{ this->~KabaFile(); }
	int _cdecl _WriteBuffer(const string &s)
	{ return WriteBuffer(s.data, s.num); }
	string _cdecl _ReadBuffer(int size)
	{
		string s;
		s.resize(size);
		int r = ReadBuffer(s.data, size);
		s.resize(r);
		return s;
	}
	void _cdecl _ReadInt(int &i)
	{ i = ReadInt(); }
	void _cdecl _ReadFloat(float &f)
	{ f = ReadFloat(); }
	void _cdecl _ReadBool(bool &b)
	{ b = ReadBool(); }
	void _cdecl _ReadVector(vector &v)
	{ ReadVector(&v); }
	void _cdecl _ReadStr(string &s)
	{ s = ReadStr(); }
	void _cdecl _WriteVector(const vector &v)
	{ WriteVector(&v); }
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
		class_add_func(NAME_FUNC_DELETE,		TypeVoid,		mf(&KabaFile::__delete__));
		class_add_func("getCDate",		TypeDate,		mf(&File::GetDateCreation));
		class_add_func("getMDate",		TypeDate,		mf(&File::GetDateModification));
		class_add_func("getADate",		TypeDate,		mf(&File::GetDateAccess));
		class_add_func("getSize",		TypeInt,		mf(&File::GetSize));
		class_add_func("getPos",		TypeInt,		mf(&File::GetPos));
		class_add_func("seek",		TypeVoid,		mf(&File::SetPos));
			func_add_param("pos",		TypeInt);
			func_add_param("absolute",	TypeBool);
		class_add_func("setBinaryMode",	TypeVoid,		mf(&File::SetBinaryMode));
			func_add_param("binary",	TypeBool);
		class_add_func("read",		TypeString,			mf(&KabaFile::_ReadBuffer));
			func_add_param("size",			TypeInt);
		class_add_func("write",		TypeInt,			mf(&KabaFile::_WriteBuffer));
			func_add_param("s",			TypeString);
		class_add_func("__lshift__",		TypeVoid,			mf(&File::WriteBool));
			func_add_param("b",			TypeBool);
		class_add_func("__lshift__",		TypeVoid,			mf(&File::WriteInt));
			func_add_param("i",			TypeInt);
		class_add_func("__lshift__",	TypeVoid,			mf(&File::WriteFloat));
			func_add_param("x",			TypeFloat32);
		class_add_func("__lshift__",	TypeVoid,			mf(&File::WriteVector));
			func_add_param("v",			TypeVector);
		class_add_func("__lshift__",		TypeVoid,			mf(&File::WriteStr));
			func_add_param("s",			TypeString);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_ReadBool));
			func_add_param("b",			TypeBoolPs);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_ReadInt));
			func_add_param("i",			TypeIntPs);
		class_add_func("__rshift__",	TypeVoid,			mf(&KabaFile::_ReadFloat));
			func_add_param("x",			TypeFloatPs);
		class_add_func("__rshift__",	TypeVoid,			mf(&KabaFile::_ReadVector));
			func_add_param("v",			TypeVector);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_ReadStr));
			func_add_param("s",			TypeString);
		class_add_func("readComplete",	TypeString,			mf(&File::ReadComplete));

	
	add_class(TypeDirEntry);
		class_add_element("name",			TypeString,		GetDADirEntry(name));
		class_add_element("size",			TypeInt,		GetDADirEntry(size));
		class_add_element("is_dir",			TypeBool,		GetDADirEntry(is_dir));
		class_add_func(NAME_FUNC_INIT,		TypeVoid,			mf(&DirEntry::__init__));
		class_add_func(NAME_FUNC_ASSIGN,		TypeVoid,			mf(&DirEntry::__assign__));
			func_add_param("other",		TypeDirEntry);
		class_add_func("str",		TypeString,			mf(&DirEntry::str));
	
	add_class(TypeDirEntryList);
		class_add_func(NAME_FUNC_INIT,		TypeVoid,			mf(&DirEntryList::__init__));
		class_add_func(NAME_FUNC_ASSIGN,		TypeVoid,			mf(&DirEntryList::__assign__));
			func_add_param("other",		TypeDirEntryList);
		class_add_func("str",		TypeString,			mf(&DirEntryList::str));


	// file access
	add_func("FileOpen",			TypeFileP,				(void*)&FileOpen);
		func_add_param("filename",		TypeString);
	add_func("FileCreate",			TypeFileP,				(void*)&FileCreate);
		func_add_param("filename",		TypeString);
	add_func("FileRead",			TypeString,				(void*)&FileRead);
		func_add_param("filename",		TypeString);
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
	add_func("GetCurDate",			TypeDate,			(void*)&get_current_date);
}

};
