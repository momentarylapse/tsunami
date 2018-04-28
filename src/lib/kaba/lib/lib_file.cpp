#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"

namespace Kaba{

class vector;

extern Class *TypeIntPs;
extern Class *TypeFloatPs;
extern Class *TypeBoolPs;

static Date *_date;
#define	GetDADate(x)			long(&_date->x)-long(_date)
static DirEntry *_dir_entry;
#define	GetDADirEntry(x)			long(&_dir_entry->x)-long(_dir_entry)



#define KABA_EXCEPTION_WRAPPER(CODE) \
try{ \
	CODE; \
}catch(::FileError &e){ \
	kaba_raise_exception(new KabaFileError(e.message())); \
}


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")


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

class KabaFileError : public KabaException
{
public: KabaFileError() : KabaException(){}
public: KabaFileError(const string &t) : KabaException(t){}
};

class KabaFile : public File
{
public:
	void _cdecl __delete__()
	{ this->~KabaFile(); }
	int _cdecl _write_buffer(const string &s)
	{ return write_buffer(s); }
	string _cdecl _read_buffer(int size)
	{
		string s;
		s.resize(size);
		int r = read_buffer(s);
		s.resize(r);
		return s;
	}
	void _cdecl _read_int(int &i)
	{ KABA_EXCEPTION_WRAPPER(i = read_int()); }
	void _cdecl _read_float(float &f)
	{ KABA_EXCEPTION_WRAPPER(f = read_float()); }
	void _cdecl _read_bool(bool &b)
	{ KABA_EXCEPTION_WRAPPER(b = read_bool()); }
	void _cdecl _read_vector(vector &v)
	{ KABA_EXCEPTION_WRAPPER(read_vector(&v)); }
	void _cdecl _read_str(string &s)
	{ KABA_EXCEPTION_WRAPPER(s = read_str()); }
	void _cdecl _write_vector(const vector &v)
	{ KABA_EXCEPTION_WRAPPER(write_vector(&v)); }

	void _write_int(int i){ write_int(i); }
	void _write_float(float f){ write_float(f); }
	void _write_bool(bool b){ write_bool(b); }
	void _write_str(const string &s){ write_str(s); }
};

/*class KabaFileNotFoundError : public KabaFileError
{ public: KabaFileNotFoundError(const string &t) : KabaFileError(t){} };

class KabaFileNotWritableError : public KabaFileError
{ public: KabaFileNotWritableError(const string &t) : KabaFileError(t){} };*/



File* kaba_file_open(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return FileOpen(filename));
	return NULL;
}

File* kaba_file_open_text(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return FileOpenText(filename));
	return NULL;
}

File* kaba_file_create(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return FileCreate(filename));
	return NULL;
}

File* kaba_file_create_text(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return FileCreateText(filename));
	return NULL;
}

string kaba_file_read(const string &filename)
{
	KABA_EXCEPTION_WRAPPER(return FileRead(filename).replace("\\r", ""));
	return "";
}

string kaba_file_hash(const string &filename, const string &type)
{
	KABA_EXCEPTION_WRAPPER(return file_hash(filename, type));
	return "";
}

void kaba_file_rename(const string &a, const string &b)
{
	if (!file_rename(a, b))
		kaba_raise_exception(new KabaFileError("can not rename file '" + a + "' -> '" + b + "'"));
}

void kaba_file_copy(const string &a, const string &b)
{
	if (!file_copy(a, b))
		kaba_raise_exception(new KabaFileError("can not copy file '" + a + "' -> '" + b + "'"));
}

void kaba_file_delete(const string &f)
{
	if (!file_delete(f))
		kaba_raise_exception(new KabaFileError("can not delete file '" + f + "'"));
}

void kaba_dir_create(const string &f)
{
	if (!dir_create(f))
		kaba_raise_exception(new KabaFileError("can not create directory '" + f + "'"));
}

void kaba_dir_delete(const string &f)
{
	if (!dir_delete(f))
		kaba_raise_exception(new KabaFileError("can not delete directory '" + f + "'"));
}


#pragma GCC pop_options

void SIAddPackageFile()
{
	add_package("file", false);

	Class*
	TypeFile			= add_type  ("File", 0);
	Class*
	TypeFileP			= add_type_p("File*", TypeFile);
	Class*
	TypeDate			= add_type  ("Date", sizeof(Date));
	Class*
	TypeDirEntry		= add_type  ("DirEntry", sizeof(DirEntry));
	Class*
	TypeDirEntryList	= add_type_a("DirEntry[]", TypeDirEntry, -1);
	Class*
	TypeFileError		= add_type  ("FileError", sizeof(KabaFileError));
	//Class*
	//TypeFileNotFoundError= add_type  ("FileError", sizeof(KabaFileNotFoundError));
	//Class*
	//TypeFileNotWritableError= add_type  ("FileError", sizeof(KabaFileNotWritableError));


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
		class_add_func(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&KabaFile::__delete__));
		class_add_func("getCDate",		TypeDate,		mf(&File::GetDateCreation));
		class_add_func("getMDate",		TypeDate,		mf(&File::GetDateModification));
		class_add_func("getADate",		TypeDate,		mf(&File::GetDateAccess));
		class_add_func("getSize",		TypeInt,		mf(&File::get_size));
		class_add_func("getPos",		TypeInt,		mf(&File::get_pos));
		class_add_func("set_pos",		TypeVoid,		mf(&File::set_pos), FLAG_RAISES_EXCEPTIONS);
			func_add_param("pos",		TypeInt);
		class_add_func("seek",		TypeVoid,		mf(&File::seek), FLAG_RAISES_EXCEPTIONS);
			func_add_param("delta",		TypeInt);
		class_add_func("read",		TypeString,			mf(&KabaFile::_read_buffer), FLAG_RAISES_EXCEPTIONS);
			func_add_param("size",			TypeInt);
		class_add_func("write",		TypeInt,			mf(&KabaFile::_write_buffer), FLAG_RAISES_EXCEPTIONS);
			func_add_param("s",			TypeString);
		class_add_func("__lshift__",		TypeVoid,			mf(&KabaFile::_write_bool), FLAG_RAISES_EXCEPTIONS);
			func_add_param("b",			TypeBool);
		class_add_func("__lshift__",		TypeVoid,			mf(&KabaFile::_write_int), FLAG_RAISES_EXCEPTIONS);
			func_add_param("i",			TypeInt);
		class_add_func("__lshift__",	TypeVoid,			mf(&KabaFile::_write_float), FLAG_RAISES_EXCEPTIONS);
			func_add_param("x",			TypeFloat32);
		class_add_func("__lshift__",	TypeVoid,			mf(&KabaFile::_write_vector), FLAG_RAISES_EXCEPTIONS);
			func_add_param("v",			TypeVector);
		class_add_func("__lshift__",		TypeVoid,			mf(&KabaFile::_write_str), FLAG_RAISES_EXCEPTIONS);
			func_add_param("s",			TypeString);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_read_bool), FLAG_RAISES_EXCEPTIONS);
			func_add_param("b",			TypeBoolPs);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_read_int), FLAG_RAISES_EXCEPTIONS);
			func_add_param("i",			TypeIntPs);
		class_add_func("__rshift__",	TypeVoid,			mf(&KabaFile::_read_float), FLAG_RAISES_EXCEPTIONS);
			func_add_param("x",			TypeFloatPs);
		class_add_func("__rshift__",	TypeVoid,			mf(&KabaFile::_read_vector), FLAG_RAISES_EXCEPTIONS);
			func_add_param("v",			TypeVector);
		class_add_func("__rshift__",		TypeVoid,			mf(&KabaFile::_read_str), FLAG_RAISES_EXCEPTIONS);
			func_add_param("s",			TypeString);
		class_add_func("eof",		TypeBool,			mf(&KabaFile::eof));

	
	add_class(TypeDirEntry);
		class_add_element("name",			TypeString,		GetDADirEntry(name));
		class_add_element("size",			TypeInt,		GetDADirEntry(size));
		class_add_element("is_dir",			TypeBool,		GetDADirEntry(is_dir));
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,			mf(&DirEntry::__init__));
		class_add_func(IDENTIFIER_FUNC_ASSIGN,		TypeVoid,			mf(&DirEntry::__assign__));
			func_add_param("other",		TypeDirEntry);
		class_add_func("str",		TypeString,			mf(&DirEntry::str));
	
	add_class(TypeDirEntryList);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,			mf(&DirEntryList::__init__));
		class_add_func(IDENTIFIER_FUNC_ASSIGN,		TypeVoid,			mf(&DirEntryList::__assign__));
			func_add_param("other",		TypeDirEntryList);
		class_add_func("str",		TypeString,			mf(&DirEntryList::str));

	add_class(TypeFileError);
		TypeFileError->derive_from(TypeException, false);
		class_set_vtable(KabaFileError);

	// file access
	add_func("FileOpen",			TypeFileP,				(void*)&kaba_file_open, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("FileOpenText",			TypeFileP,				(void*)&kaba_file_open_text, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("FileCreate",			TypeFileP,				(void*)&kaba_file_create, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("FileCreateText",			TypeFileP,				(void*)&kaba_file_create_text, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("FileRead",			TypeString,				(void*)&kaba_file_read, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("FileExists",			TypeBool,		(void*)&file_test_existence);
		func_add_param("filename",		TypeString);
	add_func("FileIsDirectory",			TypeBool,		(void*)&file_is_directory);
		func_add_param("filename",		TypeString);
	add_func("FileHash",			TypeString,		(void*)&kaba_file_hash, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
		func_add_param("type",		TypeString);
	add_func("FileRename",			TypeVoid,			(void*)&kaba_file_rename, FLAG_RAISES_EXCEPTIONS);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("FileCopy",			TypeVoid,			(void*)&kaba_file_copy, FLAG_RAISES_EXCEPTIONS);
		func_add_param("source",		TypeString);
		func_add_param("dest",		TypeString);
	add_func("FileDelete",			TypeVoid,			(void*)&kaba_file_delete, FLAG_RAISES_EXCEPTIONS);
		func_add_param("filename",		TypeString);
	add_func("DirSearch",			TypeDirEntryList,			(void*)&dir_search);
		func_add_param("dir",		TypeString);
		func_add_param("filter",		TypeString);
		func_add_param("show_dirs",		TypeBool);
	add_func("DirCreate",			TypeVoid,			(void*)&kaba_dir_create, FLAG_RAISES_EXCEPTIONS);
		func_add_param("dir",		TypeString);
	add_func("DirDelete",			TypeVoid,			(void*)&kaba_dir_delete, FLAG_RAISES_EXCEPTIONS);
		func_add_param("dir",		TypeString);
	add_func("GetCurDir",			TypeString,			(void*)&get_current_dir);
	add_func("GetCurDate",			TypeDate,			(void*)&get_current_date);
}

};
