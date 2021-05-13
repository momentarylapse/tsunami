#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "../dynamic/exception.h"

class vector;


namespace kaba {


extern const Class *TypeIntPs;
extern const Class *TypeFloatPs;
extern const Class *TypeBoolPs;
extern const Class *TypeDate;
extern const Class *TypeStringList;
const Class *TypePath;
const Class *TypePathList;


static File *_kaba_stdin = nullptr;


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")



class KabaFileError : public KabaException {
public:
	KabaFileError() : KabaException(){}
	KabaFileError(const string &t) : KabaException(t){}
	void __init__(const string &t) {
		new(this) KabaFileError(t);
	}
};

class KabaFile : public File {
public:
	void _cdecl __delete__() {
		this->~KabaFile();
	}
	int _cdecl _write_buffer(const string &s) {
		return write_buffer(s);
	}
	bytes _cdecl _read_buffer(int size) {
		return read_buffer(size);
	}
	void _cdecl _read_int(int &i) {
		KABA_EXCEPTION_WRAPPER(i = read_int());
	}
	void _cdecl _read_float(float &f) {
		 KABA_EXCEPTION_WRAPPER(f = read_float());
	}
	void _cdecl _read_bool(bool &b) {
		KABA_EXCEPTION_WRAPPER(b = read_bool());
	}
	void _cdecl _read_vector(vector &v) {
		KABA_EXCEPTION_WRAPPER(read_vector(&v));
	}
	void _cdecl _read_str(string &s) {
		KABA_EXCEPTION_WRAPPER(s = read_str());
	}
	void _cdecl _write_vector(const vector &v) {
		KABA_EXCEPTION_WRAPPER(write_vector(&v));
	}

	void _write_int(int i) { write_int(i); }
	void _write_float(float f) { write_float(f); }
	void _write_bool(bool b) { write_bool(b); }
	void _write_str(const string &s) { write_str(s); }
};

/*class KabaFileNotFoundError : public KabaFileError
{ public: KabaFileNotFoundError(const string &t) : KabaFileError(t){} };

class KabaFileNotWritableError : public KabaFileError
{ public: KabaFileNotWritableError(const string &t) : KabaFileError(t){} };*/



File* kaba_file_open(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileOpen(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_open_text(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileOpenText(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_create(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileCreate(filename), KabaFileError);
	return nullptr;
}

File* kaba_file_create_text(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileCreateText(filename), KabaFileError);
	return nullptr;
}

string kaba_file_read(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileRead(filename), KabaFileError);
	return "";
}

string kaba_file_read_text(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return FileReadText(filename), KabaFileError);
	return "";
}

void kaba_file_write(const Path &filename, const string &buffer) {
	KABA_EXCEPTION_WRAPPER2(FileWrite(filename, buffer), KabaFileError);
}

void kaba_file_write_text(const Path &filename, const string &buffer) {
	KABA_EXCEPTION_WRAPPER2(FileWriteText(filename, buffer), KabaFileError);
}

string kaba_file_hash(const Path &filename, const string &type) {
	KABA_EXCEPTION_WRAPPER2(return file_hash(filename, type), KabaFileError);
	return "";
}

void kaba_file_rename(const Path &a, const Path &b) {
	KABA_EXCEPTION_WRAPPER2(file_rename(a, b), KabaFileError);
}

void kaba_file_copy(const Path &a, const Path &b) {
	KABA_EXCEPTION_WRAPPER2(file_copy(a, b), KabaFileError);
}

void kaba_file_delete(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(file_delete(f), KabaFileError);
}

void kaba_dir_create(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(dir_create(f), KabaFileError);
}

void kaba_dir_delete(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(dir_delete(f), KabaFileError);
}


string _cdecl kaba_shell_execute(const string &cmd) {
	try {
		return shell_execute(cmd);
	} catch(::Exception &e) {
		kaba_raise_exception(new KabaException(e.message()));
	}
	return "";
}


#pragma GCC pop_options

class KabaPath : public Path {
public:
	Path lshift_p(const Path &p) const {
		return *this << p;
	}
	Path lshift_s(const string &p) const {
		return *this << p;
	}
	bool __contains__(const Path &p) const {
		return p.is_in(*this);
	}
	static Path from_str(const string &s) {
		return Path(s);
	}
};

class PathList : public Array<Path> {
public:
	void _cdecl assign(const PathList &s) {
		*this = s;
	}
	bool __contains__(const Path &s) const {
		return this->find(s) >= 0;
	}
	Array<Path> __add__(const Array<Path> &o) const {
		return *this + o;
	}
	void __adds__(const Array<Path> &o) {
		append(o);
	}
};

void SIAddPackageOSPath() {
	add_package("os");

	TypePath = add_type("Path", sizeof(Path));

	add_class(TypePath);
		class_add_element("_s", TypeString, 0);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Path::__init__);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Path::__init_ext__);
			func_add_param("p", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Path::__delete__);
		class_add_funcx("absolute", TypePath, &Path::absolute, Flags::CONST);
		class_add_funcx("dirname", TypeString, &Path::dirname, Flags::_CONST__PURE);
		class_add_funcx("basename", TypeString, &Path::basename, Flags::_CONST__PURE);
		class_add_funcx("basename_no_ext", TypeString, &Path::basename_no_ext, Flags::_CONST__PURE);
		class_add_funcx("extension", TypeString, &Path::extension, Flags::_CONST__PURE);
		class_add_funcx("canonical", TypePath, &Path::canonical, Flags::_CONST__PURE);
		class_add_funcx(IDENTIFIER_FUNC_STR, TypeString, &Path::str, Flags::_CONST__PURE);
		class_add_funcx("is_empty", TypeBool, &Path::is_empty, Flags::_CONST__PURE);
		class_add_funcx("is_relative", TypeBool, &Path::is_relative, Flags::_CONST__PURE);
		class_add_funcx("is_absolute", TypeBool, &Path::is_absolute, Flags::_CONST__PURE);
		class_add_funcx("parent", TypePath, &Path::parent, Flags::_CONST__PURE);
		class_add_funcx("compare", TypeInt, &Path::compare, Flags::_CONST__PURE);
			func_add_param("p", TypePath);
		class_add_funcx("relative_to", TypePath, &Path::relative_to, Flags::_CONST__PURE);
			func_add_param("p", TypePath);
		class_add_const("EMPTY", TypePath, &Path::EMPTY);
		class_add_funcx("@from_str", TypePath, &KabaPath::from_str, Flags::_STATIC__PURE);
			func_add_param("p", TypeString);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePath, TypePath, InlineID::NONE, mf(&Path::operator =));
		add_operator(OperatorID::EQUAL, TypeBool, TypePath, TypePath, InlineID::NONE, mf(&Path::operator ==));
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypePath, TypePath, InlineID::NONE, mf(&Path::operator !=));
		add_operator(OperatorID::SMALLER, TypeBool, TypePath, TypePath, InlineID::NONE, mf(&Path::operator <));
		add_operator(OperatorID::GREATER, TypeBool, TypePath, TypePath, InlineID::NONE, mf(&Path::operator >));
		add_operator(OperatorID::SHIFT_LEFT, TypePath, TypePath, TypePath, InlineID::NONE, mf(&KabaPath::lshift_p));
		add_operator(OperatorID::SHIFT_LEFT, TypePath, TypePath, TypeString, InlineID::NONE, mf(&KabaPath::lshift_s));
		add_operator(OperatorID::IN, TypeBool, TypePath, TypePath, InlineID::NONE, mf(&KabaPath::__contains__));


	// AFTER TypePath!
	TypePathList = add_type_l(TypePath);

	add_class(TypePath);
		class_add_funcx("all_parents", TypePathList, &Path::all_parents, Flags::_CONST__PURE);

	add_class(TypePathList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<Path>::__init__);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Array<Path>::clear);
		class_add_funcx("clear", TypeVoid, &Array<Path>::clear);
		class_add_funcx("add", TypeVoid, &Array<Path>::add);
			func_add_param("p", TypePath);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePathList, TypePathList, InlineID::NONE, mf(&PathList::assign));
		add_operator(OperatorID::IN, TypeBool, TypePathList, TypePath, InlineID::NONE, mf(&PathList::__contains__));
		add_operator(OperatorID::ADD, TypePathList, TypePathList, TypePathList, InlineID::NONE, mf(&PathList::__add__));
		add_operator(OperatorID::ADDS, TypeVoid, TypePathList, TypePathList, InlineID::NONE, mf(&PathList::__adds__));


}

void SIAddPackageOS() {
	add_package("os");

	const Class *TypeFile = add_type("File", 0);
	const Class *TypeFileP = add_type_p(TypeFile);
	const Class *TypeFilesystem = add_type("Filesystem", 0);
	const Class *TypeFileError = add_type("FileError", sizeof(KabaFileError));
	//Class *TypeFileNotFoundError= add_type  ("FileError", sizeof(KabaFileNotFoundError));
	//Class *TypeFileNotWritableError= add_type  ("FileError", sizeof(KabaFileNotWritableError));

	add_class(TypeFile);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaFile::__delete__);
		//class_add_funcx("getCDate", TypeDate, &File::GetDateCreation);
		class_add_funcx("mtime", TypeDate, &File::mtime);
		//class_add_funcx("getADate", TypeDate, &File::GetDateAccess);
		class_add_funcx("get_size", TypeInt, &File::get_size32);
		class_add_funcx("get_pos", TypeInt, &File::get_pos);
		class_add_funcx("set_pos", TypeVoid, &File::set_pos, Flags::RAISES_EXCEPTIONS);
			func_add_param("pos", TypeInt);
		class_add_funcx("seek", TypeVoid, &File::seek, Flags::RAISES_EXCEPTIONS);
			func_add_param("delta", TypeInt);
		class_add_funcx("read", TypeString, &KabaFile::_read_buffer, Flags::RAISES_EXCEPTIONS);
			func_add_param("size", TypeInt);
		class_add_funcx("write", TypeInt, &KabaFile::_write_buffer, Flags::RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_bool, Flags::RAISES_EXCEPTIONS);
			func_add_param("b", TypeBool);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_int, Flags::RAISES_EXCEPTIONS);
			func_add_param("i", TypeInt);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_float, Flags::RAISES_EXCEPTIONS);
			func_add_param("x", TypeFloat32);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_vector, Flags::RAISES_EXCEPTIONS);
			func_add_param("v", TypeVector);
		class_add_funcx("__lshift__", TypeVoid, &KabaFile::_write_str, Flags::RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_bool, Flags::RAISES_EXCEPTIONS);
			func_add_param("b", TypeBoolPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_int, Flags::RAISES_EXCEPTIONS);
			func_add_param("i", TypeIntPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_float, Flags::RAISES_EXCEPTIONS);
			func_add_param("x", TypeFloatPs);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_vector, Flags::RAISES_EXCEPTIONS);
			func_add_param("v", TypeVector);
		class_add_funcx("__rshift__", TypeVoid, &KabaFile::_read_str, Flags::RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_funcx("end", TypeBool, &KabaFile::end);

	add_class(TypeFileError);
		class_derive_from(TypeException, false, false);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaFileError::__init__, Flags::OVERRIDE);
		class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaFileError::__delete__, Flags::OVERRIDE);
		class_set_vtable(KabaFileError);


	// file access
	add_class(TypeFilesystem);
		class_add_funcx("open", TypeFileP, &kaba_file_open, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("open_text", TypeFileP, &kaba_file_open_text, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("create", TypeFileP, &kaba_file_create, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("create_text", TypeFileP, &kaba_file_create_text, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("read", TypeString, &kaba_file_read, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("read_text", TypeString, &kaba_file_read_text, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("write", TypeVoid, &kaba_file_write, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeString);
		class_add_funcx("write_text", TypeVoid, &kaba_file_write_text, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeString);
		class_add_funcx("exists", TypeBool, &file_exists, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_funcx("size", TypeInt64, &file_size, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_funcx("mtime", TypeDate, &file_mtime, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_funcx("is_directory", TypeBool, &file_is_directory, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_funcx("hash", TypeString, &kaba_file_hash, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("type", TypeString);
		class_add_funcx("rename", TypeVoid, &kaba_file_rename, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_funcx("copy", TypeVoid, &kaba_file_copy, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_funcx("delete", TypeVoid, &kaba_file_delete, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_funcx("search", TypePathList, &dir_search, Flags::STATIC);
			func_add_param("dir", TypePath);
			func_add_param("filter", TypeString);
			func_add_param("options", TypeString);
		class_add_funcx("create_directory", TypeVoid, &kaba_dir_create, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("dir", TypePath);
		class_add_funcx("delete_directory", TypeVoid, &kaba_dir_delete, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("dir", TypePath);
		class_add_funcx("current_directory", TypePath, &get_current_dir, Flags::STATIC);
		
		_kaba_stdin = new File();
		_kaba_stdin->handle = 0;
		add_ext_var("stdin", TypeFileP, &_kaba_stdin);

	// system
	add_func("shell_execute", TypeString, (void*)&kaba_shell_execute, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("cmd", TypeString);
}

};
