#include "../../os/file.h"
#include "../../os/filesystem.h"
#include "../../os/formatter.h"
#include "../../os/msg.h"
#include "../../os/CommandLineParser.h"
#include "../../os/config.h"
#include "../../os/terminal.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "list.h"
#include "shared.h"
#include "../dynamic/exception.h"
#include "../../base/callable.h"

class vec3;


namespace kaba {


extern const Class *TypeDate;
extern const Class *TypeStringList;
const Class *TypePath;
const Class *TypePathList;
//const Class *TypeStreamP;
//const Class *TypeStreamShared;

const Class* TypeCallback;
const Class* TypeCallbackString;

extern const Class *TypeStringList;
extern const Class *TypeAny;
extern const Class *TypeAnyDict;
const Class *TypeOsConfiguration;


Any _os_config_get(Configuration &c, const string &key) {
	return c.get(key, Any());
}


static shared<Stream> _kaba_stdin;


KABA_LINK_GROUP_BEGIN

class KabaFileError : public KabaException {
public:
	KabaFileError() : KabaException(){}
	KabaFileError(const string &t) : KabaException(t){}
	void __init__(const string &t) {
		new(this) KabaFileError(t);
	}
};

class KabaFileStream : public os::fs::FileStream {
public:
	void __delete__() {
		//msg_write("~~~ FILE STREAM");
		this->~KabaFileStream();
	}
	int _pos() {
		return pos();
	}
	void _set_pos(int pos) {
		set_pos(pos);
	}
	void _seek(int delta) {
		seek(delta);
	}
};

class KabaStream : public Stream {
public:
	KabaStream() : Stream(Stream::Mode::NONE) {}
	void set_pos(int) override {}
	int pos() const override { return 0; }
	void seek(int) override {}
	int64 size() const override { return 0; }
	bool is_end() const override { return false; }

	int read_basic(void *buffer, int size) override { return 0; };
	int write_basic(const void *buffer, int size) override { return 0; };

	void __delete__() {
		//msg_write("~~~ STREAM");
		this->~KabaStream();
	}
	int _cdecl _write(const string &s) {
		return write(s);
	}
	bytes _cdecl _read_size(int size) {
		bytes data;
		data.resize(size);
		int r = read(data);
		if (r < 0)
			return bytes();
		data.resize(r);
		return data;
	}
	int _cdecl _read_bytes(bytes &data) {
		return read(data);
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
	void _cdecl _read_vector(vec3 &v) {
		KABA_EXCEPTION_WRAPPER(read_vector(&v));
	}
	void _cdecl _read_str(string &s) {
		KABA_EXCEPTION_WRAPPER(s = read_str());
	}
	void _cdecl _write_vector(const vec3 &v) {
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



xfer<os::fs::FileStream> kaba_file_open(const Path &filename, const string &mode) {
	KABA_EXCEPTION_WRAPPER2(return os::fs::open(filename, mode), KabaFileError);
	return nullptr;
}

string kaba_file_read(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return os::fs::read_binary(filename), KabaFileError);
	return "";
}

string kaba_file_read_text(const Path &filename) {
	KABA_EXCEPTION_WRAPPER2(return os::fs::read_text(filename), KabaFileError);
	return "";
}

void kaba_file_write(const Path &filename, const string &buffer) {
	KABA_EXCEPTION_WRAPPER2(os::fs::write_binary(filename, buffer), KabaFileError);
}

void kaba_file_write_text(const Path &filename, const string &buffer) {
	KABA_EXCEPTION_WRAPPER2(os::fs::write_text(filename, buffer), KabaFileError);
}

string kaba_file_hash(const Path &filename, const string &type) {
	KABA_EXCEPTION_WRAPPER2(return os::fs::hash(filename, type), KabaFileError);
	return "";
}

void kaba_file_rename(const Path &a, const Path &b) {
	KABA_EXCEPTION_WRAPPER2(os::fs::rename(a, b), KabaFileError);
}

void kaba_file_move(const Path &a, const Path &b) {
	KABA_EXCEPTION_WRAPPER2(os::fs::move(a, b), KabaFileError);
}

void kaba_file_copy(const Path &a, const Path &b) {
	KABA_EXCEPTION_WRAPPER2(os::fs::copy(a, b), KabaFileError);
}

void kaba_file_delete(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(os::fs::_delete(f), KabaFileError);
}

void kaba_dir_create(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(os::fs::create_directory(f), KabaFileError);
}

void kaba_dir_delete(const Path &f) {
	KABA_EXCEPTION_WRAPPER2(os::fs::delete_directory(f), KabaFileError);
}


string _cdecl kaba_shell_execute(const string &cmd, bool verbose) {
	try {
		return os::terminal::shell_execute(cmd, verbose);
	} catch(::Exception &e) {
		kaba_raise_exception(new KabaException(e.message()));
	}
	return "";
}

KABA_LINK_GROUP_END

class KabaPath : public Path {
public:
	Path cat_p(const Path &p) const {
		return *this | p;
	}
	Path cat_s(const string &p) const {
		return *this | p;
	}
	bool __contains__(const Path &p) const {
		return p.is_in(*this);
	}
	bool __bool__() const {
		return !this->is_empty();
	}
	static Path from_str(const string &s) {
		return Path(s);
	}
};

void SIAddPackageOSPath(Context *c) {
	add_package(c, "os");

	TypePath = add_type("Path", config.target.dynamic_array_size);

	add_class(TypePath);
		class_add_element_x("_s", TypeString, 0);
		class_add_func(Identifier::func::Init, TypeVoid, &Path::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Init, TypeVoid, &Path::__init_ext__, Flags::AutoCast | Flags::Mutable);
			func_add_param("p", TypeString);
		class_add_func(Identifier::func::Delete, TypeVoid, &Path::__delete__, Flags::Mutable);
		class_add_func("absolute", TypePath, &Path::absolute);
		class_add_func("dirname", TypeString, &Path::dirname, Flags::Pure);
		class_add_func("basename", TypeString, &Path::basename, Flags::Pure);
		class_add_func("basename_no_ext", TypeString, &Path::basename_no_ext, Flags::Pure);
		class_add_func("extension", TypeString, &Path::extension, Flags::Pure);
		class_add_func("canonical", TypePath, &Path::canonical, Flags::Pure);
		class_add_func(Identifier::func::Str, TypeString, &Path::str, Flags::Pure);
		class_add_func(Identifier::func::Repr, TypeString, &Path::repr, Flags::Pure);
		class_add_func("is_empty", TypeBool, &Path::is_empty, Flags::Pure);
		class_add_func("is_relative", TypeBool, &Path::is_relative, Flags::Pure);
		class_add_func("is_absolute", TypeBool, &Path::is_absolute, Flags::Pure);
		class_add_func("__bool__", TypeBool, &KabaPath::__bool__, Flags::Pure);
		class_add_func("parent", TypePath, &Path::parent, Flags::Pure);
		class_add_func("compare", TypeInt32, &Path::compare, Flags::Pure);
			func_add_param("p", TypePath);
		class_add_func("relative_to", TypePath, &Path::relative_to, Flags::Pure);
			func_add_param("p", TypePath);
		class_add_const("EMPTY", TypePath, &Path::EMPTY);
		class_add_func("@from_str", TypePath, &KabaPath::from_str, Flags::Static | Flags::Pure);
			func_add_param("p", TypeString);
		add_operator(OperatorID::Assign, TypeVoid, TypePath, TypePath, InlineID::None, &Path::operator =);
		add_operator(OperatorID::Equal, TypeBool, TypePath, TypePath, InlineID::None, &Path::operator ==);
		add_operator(OperatorID::NotEqual, TypeBool, TypePath, TypePath, InlineID::None, &Path::operator !=);
		add_operator(OperatorID::Smaller, TypeBool, TypePath, TypePath, InlineID::None, &Path::operator <);
		add_operator(OperatorID::Greater, TypeBool, TypePath, TypePath, InlineID::None, &Path::operator >);
		add_operator(OperatorID::BitOr, TypePath, TypePath, TypePath, InlineID::None, &KabaPath::cat_p);
		add_operator(OperatorID::BitOr, TypePath, TypePath, TypeString, InlineID::None, &KabaPath::cat_s);
		add_operator(OperatorID::In, TypeBool, TypePath, TypePath, InlineID::None, &KabaPath::__contains__);


	// AFTER TypePath!
	TypePathList = add_type_list(TypePath);

	add_class(TypePath);
		class_add_func("all_parents", TypePathList, &Path::all_parents, Flags::Pure);

	lib_create_list<Path>(TypePathList);
}


char _el_off_data[1024];
#define evil_member_offset(C, M)	((int_p)((char*)&(reinterpret_cast<C*>(&_el_off_data[0])->M) - &_el_off_data[0]))


class KabaCommandLineParser : CommandLineParser {
public:
	void __init__() {
		new(this) CommandLineParser;
	}
	void __delete__() {
		CommandLineParser::~CommandLineParser();
	}
	void option1(const string &name, const string &comment, Callable<void()> &cb) {
		option(name, comment, [&cb] { cb(); });
	}
	void option2(const string &name, const string &p, const string &comment, Callable<void(const string&)> &cb) {
		option(name, p, comment, [&cb] (const string &s) { cb(s); });
	}
	void cmd1(const string &name, const string &p, const string &comment, Callable<void(const Array<string>&)> &cb) {
		cmd(name, p, comment, [&cb] (const Array<string> &s) { cb(s); });
	}
	void parse1(const Array<string> &arg) {
		Array<string> a = {"?"};
		parse(a + arg);
	}
};

void SIAddPackageOS(Context *c) {
	add_package(c, "os");

	auto TypeStream = add_type("Stream", sizeof(Stream));
	auto TypeStreamXfer = add_type_p_xfer(TypeStream);
	auto TypeFileStream = add_type("FileStream", sizeof(os::fs::FileStream));
	auto TypeFileStreamXfer = add_type_p_xfer(TypeFileStream);
	auto TypeFileStreamSharedNN = add_type_p_shared_not_null(TypeFileStream);
	auto TypeFilesystem = add_type("fs", 0);
	const_cast<Class*>(TypeFilesystem)->from_template = TypeNamespaceT;
	auto TypeFileError = add_type("FileError", sizeof(KabaFileError));
	auto TypeCommandLineParser = add_type("CommandLineParser", sizeof(CommandLineParser));
	TypeOsConfiguration = add_type("Configuration", sizeof(Configuration));
	auto TypeTerminal = add_type("terminal", 0);

	TypeCallback = add_type_func(TypeVoid, {});
	TypeCallbackString = add_type_func(TypeVoid, {TypeString});
	auto TypeCallbackStringList = add_type_func(TypeVoid, {TypeStringList});

	lib_create_pointer_xfer(TypeStreamXfer);
	lib_create_pointer_xfer(TypeFileStreamXfer);
	lib_create_pointer_shared<KabaFileStream>(TypeFileStreamSharedNN, TypeFileStreamXfer);

	add_class(TypeStream);
		class_add_element(Identifier::SharedCount, TypeInt32, evil_member_offset(os::fs::FileStream, _pointer_ref_counter));
		//class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &KabaFileStream::__delete__);
		class_add_func(Identifier::func::Delete, TypeVoid, &KabaFileStream::__delete__, Flags::Mutable);

		class_add_func("read", TypeBytes, &KabaStream::_read_size, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("size", TypeInt32);
		class_add_func("read", TypeInt32, &KabaStream::_read_bytes, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", TypeBytes);
		class_add_func("write", TypeInt32, &KabaStream::_write, Flags::RaisesExceptions | Flags::Mutable);
		//class_add_func_virtual("write", TypeInt32, &FileStream::write);
			func_add_param("s", TypeBytes);

		class_add_func("__lshift__", TypeVoid, &KabaStream::_write_bool, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("b", TypeBool);
		class_add_func("__lshift__", TypeVoid, &KabaStream::_write_int, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("i", TypeInt32);
		class_add_func("__lshift__", TypeVoid, &KabaStream::_write_float, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("x", TypeFloat32);
		class_add_func("__lshift__", TypeVoid, &KabaStream::_write_vector, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("v", TypeVec3);
		class_add_func("__lshift__", TypeVoid, &KabaStream::_write_str, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", TypeString);
		class_add_func("__rshift__", TypeVoid, &KabaStream::_read_bool, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("b", TypeBool, Flags::Out);
		class_add_func("__rshift__", TypeVoid, &KabaStream::_read_int, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("i", TypeInt32, Flags::Out);
		class_add_func("__rshift__", TypeVoid, &KabaStream::_read_float, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("x", TypeFloat32, Flags::Out);
		class_add_func("__rshift__", TypeVoid, &KabaStream::_read_vector, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("v", TypeVec3, Flags::Out);
		class_add_func("__rshift__", TypeVoid, &KabaStream::_read_str, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", TypeString, Flags::Out);

		// FIXME &FileStream::_pointer_ref_counter does not work here
		// we get a base-class-pointer... \(O_O)/
		//class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &Stream::__delete__);
		//class_add_func_virtual("write", TypeVoid, Stream::write);
		//class_add_element(Identifier::SHARED_COUNT, TypeInt32, &Stream::_pointer_ref_counter);
		//const_cast<Class*>(TypeStream)->elements.back().offset = offsetof(Stream, _pointer_ref_counter);
		class_set_vtable(KabaStream);


	//KabaSharedPointer<FileStream>::declare(TypeStreamSP);

	add_class(TypeFileStream);
		class_derive_from(TypeStream);
		//class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &KabaFileStream::__delete__, Flags::OVERRIDE);
		class_add_func(Identifier::func::Delete, TypeVoid, &KabaFileStream::__delete__, Flags::Override | Flags::Mutable);
		//class_add_func("getCDate", TypeDate, &File::GetDateCreation);
		class_add_func("mtime", TypeDate, &os::fs::FileStream::mtime);
		//class_add_func("getADate", TypeDate, &FileStream::GetDateAccess);
		class_add_func("size", TypeInt32, &KabaFileStream::size32);
		class_add_func("pos", TypeInt32, &KabaFileStream::_pos);//os::fs::FileStream::get_pos);
		class_add_func("set_pos", TypeVoid, &KabaFileStream::_set_pos, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("pos", TypeInt32);
		class_add_func("seek", TypeVoid, &KabaFileStream::_seek, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("delta", TypeInt32);
		class_add_func("is_end", TypeBool, &KabaFileStream::is_end);
		{
			os::fs::FileStream my_instance(-1, Stream::Mode::NONE);
			class_link_vtable(*(void***)&my_instance);
		}


	add_class(TypeFileError);
		class_derive_from(TypeException);
		class_add_func(Identifier::func::Init, TypeVoid, &KabaFileError::__init__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, &KabaFileError::__delete__, Flags::Override | Flags::Mutable);
		class_set_vtable(KabaFileError);


	add_class(TypeCommandLineParser);
		class_add_func(Identifier::func::Init, TypeVoid, &KabaCommandLineParser::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &KabaCommandLineParser::__delete__, Flags::Mutable);
		class_add_func("info", TypeVoid, &CommandLineParser::info, Flags::Mutable);
			func_add_param("cmd", TypeString);
			func_add_param("i", TypeString);
		class_add_func("show", TypeVoid, &CommandLineParser::show);
		class_add_func("parse", TypeVoid, &KabaCommandLineParser::parse1);
			func_add_param("arg", TypeStringList);
		class_add_func("option", TypeVoid, &KabaCommandLineParser::option1, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallback);
		class_add_func("option", TypeVoid, &KabaCommandLineParser::option2, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("p", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallbackString);
		class_add_func("cmd", TypeVoid, &KabaCommandLineParser::cmd1, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("p", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallbackStringList);


	add_class(TypeOsConfiguration);
		class_add_element("dict", TypeAnyDict, &Configuration::map);
		class_add_func(Identifier::func::Init, TypeVoid, &Configuration::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &Configuration::__del__, Flags::Mutable);
		class_add_func("load", TypeBool, &Configuration::load, Flags::Mutable);
			func_add_param("path", TypePath);
		class_add_func("save", TypeVoid, &Configuration::save);
			func_add_param("path", TypePath);
		class_add_func(Identifier::func::Set, TypeVoid, &Configuration::set_int, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("value", TypeInt32);
		class_add_func(Identifier::func::Set, TypeVoid, &Configuration::set_float, Flags::Mutable); // FIXME: operator preference...
			func_add_param("name", TypeString);
			func_add_param("value", TypeFloat32);
		class_add_func(Identifier::func::Set, TypeVoid, &Configuration::set_bool, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("value", TypeBool);
		class_add_func(Identifier::func::Set, TypeVoid, &Configuration::set_str, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("value", TypeString);
		class_add_func(Identifier::func::Set, TypeVoid, &Configuration::set, Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("value", TypeAny);
		class_add_func("get_int", TypeInt32, &Configuration::get_int);
			func_add_param("name", TypeString);
			func_add_param("default", TypeInt32);
		class_add_func("get_float", TypeFloat32, &Configuration::get_float);
			func_add_param("name", TypeString);
			func_add_param("default", TypeFloat32);
		class_add_func("get_bool", TypeBool, &Configuration::get_bool);
			func_add_param("name", TypeString);
			func_add_param("default", TypeBool);
		class_add_func("get_str", TypeString, &Configuration::get_str);
			func_add_param("name", TypeString);
			func_add_param("default", TypeString);
		class_add_func(Identifier::func::Get, TypeAny, &_os_config_get);
			func_add_param("name", TypeString);
		class_add_func("keys", TypeStringList, &Configuration::keys);


	// file access
	add_class(TypeFilesystem);
		class_add_func("open", TypeFileStreamXfer, &kaba_file_open, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
			func_add_param("mode", TypeString);
		class_add_func("read", TypeBytes, &kaba_file_read, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
		class_add_func("read_text", TypeString, &kaba_file_read_text, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
		class_add_func("write", TypeVoid, &kaba_file_write, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeBytes);
		class_add_func("write_text", TypeVoid, &kaba_file_write_text, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeString);
		class_add_func("exists", TypeBool, &os::fs::exists, Flags::Static);
			func_add_param("filename", TypePath);
		class_add_func("size", TypeInt64, &os::fs::size, Flags::Static);
			func_add_param("filename", TypePath);
		class_add_func("mtime", TypeDate, &os::fs::mtime, Flags::Static);
			func_add_param("filename", TypePath);
		class_add_func("is_directory", TypeBool, &os::fs::is_directory, Flags::Static);
			func_add_param("filename", TypePath);
		class_add_func("hash", TypeString, &kaba_file_hash, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
			func_add_param("type", TypeString);
		class_add_func("move", TypeVoid, &kaba_file_move, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("rename", TypeVoid, &kaba_file_rename, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("copy", TypeVoid, &kaba_file_copy, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("delete", TypeVoid, &kaba_file_delete, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", TypePath);
		class_add_func("search", TypePathList, &os::fs::search, Flags::Static);
			func_add_param("dir", TypePath);
			func_add_param("filter", TypeString);
			func_add_param("options", TypeString);
		class_add_func("create_directory", TypeVoid, &kaba_dir_create, Flags::Static | Flags::RaisesExceptions);
			func_add_param("dir", TypePath);
		class_add_func("delete_directory", TypeVoid, &kaba_dir_delete, Flags::Static | Flags::RaisesExceptions);
			func_add_param("dir", TypePath);
		class_add_func("current_directory", TypePath, &os::fs::current_directory, Flags::Static);
		class_add_func("set_current_directory", TypeVoid, &os::fs::set_current_directory, Flags::Static);
			func_add_param("dir", TypePath);
		
		if (!_kaba_stdin)
			_kaba_stdin = new os::fs::FileStream(0, Stream::Mode::TEXT);
		add_ext_var("stdin", TypeFileStreamSharedNN, &_kaba_stdin);
	
	add_class(TypeTerminal);
		class_add_const("RED", TypeString, &os::terminal::RED);
		class_add_const("GREEN", TypeString, &os::terminal::GREEN);
		class_add_const("BLUE", TypeString, &os::terminal::BLUE);
		class_add_const("YELLOW", TypeString, &os::terminal::YELLOW);
		class_add_const("ORANGE", TypeString, &os::terminal::ORANGE);
		class_add_const("CYAN", TypeString, &os::terminal::CYAN);
		class_add_const("MAGENTA", TypeString, &os::terminal::MAGENTA);
		class_add_const("GRAY", TypeString, &os::terminal::GRAY);
		class_add_const("DARK_GRAY", TypeString, &os::terminal::DARK_GRAY);
		class_add_const("BOLD", TypeString, &os::terminal::BOLD);
		class_add_const("END", TypeString, &os::terminal::END);

	// system
	add_func("shell_execute", TypeString, &kaba_shell_execute, Flags::Static | Flags::RaisesExceptions);
		func_add_param("cmd", TypeString);
		func_add_param_def("verbose", TypeBool, false);


	add_type_cast(50, TypeString, TypePath, "os.Path.@from_str");
}

};
