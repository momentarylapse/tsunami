#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"

class vector;


namespace Kaba{


extern const Class *TypeIntPs;
extern const Class *TypeFloatPs;
extern const Class *TypeBoolPs;



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


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

class KabaFileError : public KabaException {
public:
	KabaFileError() : KabaException(){}
	KabaFileError(const string &t) : KabaException(t){}
	void __init__(const string &t) {
		new(this) KabaFileError(t);
	}
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
	KABA_EXCEPTION_WRAPPER2(return FileOpen(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_open_text(const string &filename)
{
	KABA_EXCEPTION_WRAPPER2(return FileOpenText(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_create(const string &filename)
{
	KABA_EXCEPTION_WRAPPER2(return FileCreate(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_create_text(const string &filename)
{
	KABA_EXCEPTION_WRAPPER2(return FileCreateText(filename), KabaFileError);
	return nullptr;
}

string kaba_file_read(const string &filename)
{
	KABA_EXCEPTION_WRAPPER2(return FileRead(filename), KabaFileError);
	return "";
}

string kaba_file_read_text(const string &filename)
{
	KABA_EXCEPTION_WRAPPER2(return FileReadText(filename), KabaFileError);
	return "";
}

void kaba_file_write(const string &filename, const string &buffer)
{
	KABA_EXCEPTION_WRAPPER2(FileWrite(filename, buffer), KabaFileError);
}

void kaba_file_write_text(const string &filename, const string &buffer)
{
	KABA_EXCEPTION_WRAPPER2(FileWriteText(filename, buffer), KabaFileError);
}

string kaba_file_hash(const string &filename, const string &type)
{
	KABA_EXCEPTION_WRAPPER2(return file_hash(filename, type), KabaFileError);
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

	const Class *TypeFile = add_type("File", 0);
	const Class *TypeFileP = add_type_p("File*", TypeFile);
	const Class *TypeFilesystem = add_type("Filesystem", 0);
	const Class *TypeDate = add_type("Date", sizeof(Date));
	const Class *TypeDirEntry = add_type("DirEntry", sizeof(DirEntry));
	const Class *TypeDirEntryList = add_type_a("DirEntry[]", TypeDirEntry, -1);
	const Class *TypeFileError = add_type("FileError", sizeof(KabaFileError));
	//Class *TypeFileNotFoundError= add_type  ("FileError", sizeof(KabaFileNotFoundError));
	//Class *TypeFileNotWritableError= add_type  ("FileError", sizeof(KabaFileNotWritableError));


	add_class(TypeDate);
		class_add_elementx("time", TypeInt, &Date::time);
		class_add_funcx("format", TypeString, &Date::format);
			func_add_param("f", TypeString);
		class_add_funcx("str", TypeString, &Date::str);
		class_add_funcx("now", TypeDate, &get_current_date, FLAG_STATIC);
	
	add_class(TypeFile);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaFile::__delete__);
		//class_add_funcx("getCDate", TypeDate, &File::GetDateCreation);
		class_add_funcx("get_date", TypeDate, &File::GetDateModification);
		//class_add_funcx("getADate", TypeDate, &File::GetDateAccess);
		class_add_funcx("get_size", TypeInt, &File::get_size);
		class_add_funcx("get_pos", TypeInt, &File::get_pos);
		class_add_funcx("set_pos", TypeVoid, &File::set_pos, FLAG_RAISES_EXCEPTIONS);
			func_add_param("pos", TypeInt);
		class_add_funcx("seek", TypeVoid, &File::seek, FLAG_RAISES_EXCEPTIONS);
			func_add_param("delta", TypeInt);
		class_add_funcx("read", TypeString, &KabaFile::_read_buffer, FLAG_RAISES_EXCEPTIONS);
			func_add_param("size", TypeInt);
		class_add_funcx("write", TypeInt, &KabaFile::_write_buffer, FLAG_RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_bool, FLAG_RAISES_EXCEPTIONS);
			func_add_param("b", TypeBool);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_int, FLAG_RAISES_EXCEPTIONS);
			func_add_param("i", TypeInt);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_float, FLAG_RAISES_EXCEPTIONS);
			func_add_param("x", TypeFloat32);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_vector, FLAG_RAISES_EXCEPTIONS);
			func_add_param("v", TypeVector);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_str, FLAG_RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_bool, FLAG_RAISES_EXCEPTIONS);
			func_add_param("b", TypeBoolPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_int, FLAG_RAISES_EXCEPTIONS);
			func_add_param("i", TypeIntPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_float, FLAG_RAISES_EXCEPTIONS);
			func_add_param("x", TypeFloatPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_vector, FLAG_RAISES_EXCEPTIONS);
			func_add_param("v", TypeVector);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_str, FLAG_RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("eof", TypeBool, &KabaFile::eof);

	
	add_class(TypeDirEntry);
		class_add_elementx("name", TypeString, &DirEntry::name);
		class_add_elementx("size", TypeInt, &DirEntry::size);
		class_add_elementx("is_dir", TypeBool, &DirEntry::is_dir);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &DirEntry::__init__);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &DirEntry::__assign__);
			func_add_param("other", TypeDirEntry);
		class_add_funcx("str", TypeString, &DirEntry::str);
	
	add_class(TypeDirEntryList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &DirEntryList::__init__);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &DirEntryList::__assign__);
			func_add_param("other", TypeDirEntryList);
		class_add_funcx("str", TypeString, &DirEntryList::str);

	add_class(TypeFileError);
		class_derive_from(TypeException, false, false);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaFileError::__init__, FLAG_OVERRIDE);
		class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaFileError::__delete__, FLAG_OVERRIDE);
		class_set_vtable(KabaFileError);

	// file access
	add_class(TypeFilesystem);
		class_add_funcx("open", TypeFileP, &kaba_file_open, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("open_text", TypeFileP, &kaba_file_open_text, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("create", TypeFileP, &kaba_file_create, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("create_text", TypeFileP, &kaba_file_create_text, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("read", TypeString, &kaba_file_read, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("read_text", TypeString, &kaba_file_read_text, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("write", TypeVoid, &kaba_file_write, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
			func_add_param("buffer", TypeString);
		class_add_funcx("write_text", TypeVoid, &kaba_file_write_text, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
			func_add_param("buffer", TypeString);
		class_add_funcx("exists", TypeBool, &file_test_existence, FLAG_STATIC);
			func_add_param("filename", TypeString);
		class_add_funcx("is_directory", TypeBool, &file_is_directory, FLAG_STATIC);
			func_add_param("filename", TypeString);
		class_add_funcx("hash", TypeString, &kaba_file_hash, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
			func_add_param("type", TypeString);
		class_add_funcx("rename", TypeVoid, &kaba_file_rename, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("source", TypeString);
			func_add_param("dest", TypeString);
		class_add_funcx("copy", TypeVoid, &kaba_file_copy, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("source", TypeString);
			func_add_param("dest", TypeString);
		class_add_funcx("delete", TypeVoid, &kaba_file_delete, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
		class_add_funcx("search", TypeDirEntryList, &dir_search, FLAG_STATIC);
			func_add_param("dir", TypeString);
			func_add_param("filter", TypeString);
			func_add_param("show_dirs", TypeBool);
		class_add_funcx("create_directory", TypeVoid, &kaba_dir_create, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("dir", TypeString);
		class_add_funcx("delete_directory", TypeVoid, &kaba_dir_delete, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("dir", TypeString);
		class_add_funcx("current_directory", TypeString, &get_current_dir, FLAG_STATIC);
}

};
