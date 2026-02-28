#include "../../os/file.h"
#include "../../os/filesystem.h"
#include "../../os/formatter.h"
#include "../../os/msg.h"
#include "../../os/CommandLineParser.h"
#include "../../os/config.h"
#include "../../os/terminal.h"
#include "../../os/app.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "list.h"
#include "shared.h"
#include "../dynamic/exception.h"
#include "../../base/callable.h"

struct vec3;


namespace kaba {


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

void kaba_os_exit(int status) {
	exit(status);
}

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
	add_internal_package(c, "os", "1");

	common_types.path = add_type("Path", config.target.dynamic_array_size);

	add_class(common_types.path);
		class_add_element_x("_s", common_types.string, 0);
		class_add_func(Identifier::func::Init, common_types._void, &Path::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Init, common_types._void, &Path::__init_ext__, Flags::AutoCast | Flags::Mutable);
			func_add_param("p", common_types.string);
		class_add_func(Identifier::func::Delete, common_types._void, &Path::__delete__, Flags::Mutable);
		class_add_func("absolute", common_types.path, &Path::absolute);
		class_add_func("dirname", common_types.string, &Path::dirname, Flags::Pure);
		class_add_func("basename", common_types.string, &Path::basename, Flags::Pure);
		class_add_func("basename_no_ext", common_types.string, &Path::basename_no_ext, Flags::Pure);
		class_add_func("extension", common_types.string, &Path::extension, Flags::Pure);
		class_add_func("canonical", common_types.path, &Path::canonical, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &Path::str, Flags::Pure);
		class_add_func(Identifier::func::Repr, common_types.string, &Path::repr, Flags::Pure);
		class_add_func("is_empty", common_types._bool, &Path::is_empty, Flags::Pure);
		class_add_func("is_relative", common_types._bool, &Path::is_relative, Flags::Pure);
		class_add_func("is_absolute", common_types._bool, &Path::is_absolute, Flags::Pure);
		class_add_func("__bool__", common_types._bool, &KabaPath::__bool__, Flags::Pure);
		class_add_func("parent", common_types.path, &Path::parent, Flags::Pure);
		class_add_func("compare", common_types.i32, &Path::compare, Flags::Pure);
			func_add_param("p", common_types.path);
		class_add_func("relative_to", common_types.path, &Path::relative_to, Flags::Pure);
			func_add_param("p", common_types.path);
		class_add_const("EMPTY", common_types.path, &Path::EMPTY);
		class_add_func("@from_str", common_types.path, &KabaPath::from_str, Flags::Static | Flags::Pure);
			func_add_param("p", common_types.string);
		add_operator(OperatorID::Assign, common_types._void, common_types.path, common_types.path, InlineID::None, &Path::operator =);
		add_operator(OperatorID::Equal, common_types._bool, common_types.path, common_types.path, InlineID::None, &Path::operator ==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.path, common_types.path, InlineID::None, &Path::operator !=);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.path, common_types.path, InlineID::None, &Path::operator <);
		add_operator(OperatorID::Greater, common_types._bool, common_types.path, common_types.path, InlineID::None, &Path::operator >);
		add_operator(OperatorID::BitOr, common_types.path, common_types.path, common_types.path, InlineID::None, &KabaPath::cat_p);
		add_operator(OperatorID::BitOr, common_types.path, common_types.path, common_types.string, InlineID::None, &KabaPath::cat_s);
		add_operator(OperatorID::In, common_types._bool, common_types.path, common_types.path, InlineID::None, &KabaPath::__contains__);


	// AFTER common_types.path!
	common_types.path_list = add_type_list(common_types.path);

	add_class(common_types.path);
		class_add_func("all_parents", common_types.path_list, &Path::all_parents, Flags::Pure);

	lib_create_list<Path>(common_types.path_list);
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
	add_internal_package(c, "os", "(ignored)");

	auto TypeStream = add_type("Stream", sizeof(Stream));
	auto TypeStreamXfer = add_type_p_xfer(TypeStream);
	auto TypeFileStream = add_type("FileStream", sizeof(os::fs::FileStream));
	auto TypeFileStreamXfer = add_type_p_xfer(TypeFileStream);
	auto TypeFileStreamSharedNN = add_type_p_shared_not_null(TypeFileStream);
	auto TypeFilesystem = add_type("fs", 0);
	const_cast<Class*>(TypeFilesystem)->from_template = common_types.namespace_t;
	auto TypeFileError = add_type("FileError", sizeof(KabaFileError));
	auto TypeCommandLineParser = add_type("CommandLineParser", sizeof(CommandLineParser));
	common_types.os_configuration = add_type("Configuration", sizeof(Configuration));
	auto TypeTerminal = add_type("terminal", 0);
	const_cast<Class*>(TypeTerminal)->from_template = common_types.namespace_t;
	auto TypeApp = add_type("app", 0);
	const_cast<Class*>(TypeApp)->from_template = common_types.namespace_t;

	auto TypeCallbackStringList = add_type_func(common_types._void, {common_types.string_list});

	lib_create_pointer_xfer(TypeStreamXfer);
	lib_create_pointer_xfer(TypeFileStreamXfer);
	lib_create_pointer_shared<KabaFileStream>(TypeFileStreamSharedNN, TypeFileStreamXfer);

	add_class(TypeStream);
		class_add_element(Identifier::SharedCount, common_types.i32, evil_member_offset(os::fs::FileStream, _pointer_ref_counter));
		//class_add_func_virtual(Identifier::Func::DELETE, common_types._void, &KabaFileStream::__delete__);
		class_add_func(Identifier::func::Delete, common_types._void, &KabaFileStream::__delete__, Flags::Mutable);

		class_add_func("read", common_types.bytes, &KabaStream::_read_size, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("size", common_types.i32);
		class_add_func("read", common_types.i32, &KabaStream::_read_bytes, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", common_types.bytes);
		class_add_func("write", common_types.i32, &KabaStream::_write, Flags::RaisesExceptions | Flags::Mutable);
		//class_add_func_virtual("write", common_types.i32, &FileStream::write);
			func_add_param("s", common_types.bytes);

		class_add_func("__lshift__", common_types._void, &KabaStream::_write_bool, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("b", common_types._bool);
		class_add_func("__lshift__", common_types._void, &KabaStream::_write_int, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("i", common_types.i32);
		class_add_func("__lshift__", common_types._void, &KabaStream::_write_float, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("x", common_types.f32);
		class_add_func("__lshift__", common_types._void, &KabaStream::_write_vector, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("v", common_types.vec3);
		class_add_func("__lshift__", common_types._void, &KabaStream::_write_str, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", common_types.string);
		class_add_func("__rshift__", common_types._void, &KabaStream::_read_bool, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("b", common_types._bool, Flags::Out);
		class_add_func("__rshift__", common_types._void, &KabaStream::_read_int, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("i", common_types.i32, Flags::Out);
		class_add_func("__rshift__", common_types._void, &KabaStream::_read_float, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("x", common_types.f32, Flags::Out);
		class_add_func("__rshift__", common_types._void, &KabaStream::_read_vector, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("v", common_types.vec3, Flags::Out);
		class_add_func("__rshift__", common_types._void, &KabaStream::_read_str, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("s", common_types.string, Flags::Out);

		// FIXME &FileStream::_pointer_ref_counter does not work here
		// we get a base-class-pointer... \(O_O)/
		//class_add_func_virtual(Identifier::Func::DELETE, common_types._void, &Stream::__delete__);
		//class_add_func_virtual("write", common_types._void, Stream::write);
		//class_add_element(Identifier::SHARED_COUNT, common_types.i32, &Stream::_pointer_ref_counter);
		//const_cast<Class*>(TypeStream)->elements.back().offset = offsetof(Stream, _pointer_ref_counter);
		class_set_vtable(KabaStream);


	//KabaSharedPointer<FileStream>::declare(TypeStreamSP);

	add_class(TypeFileStream);
		class_derive_from(TypeStream);
		//class_add_func_virtual(Identifier::Func::DELETE, common_types._void, &KabaFileStream::__delete__, Flags::OVERRIDE);
		class_add_func(Identifier::func::Delete, common_types._void, &KabaFileStream::__delete__, Flags::Override | Flags::Mutable);
		//class_add_func("getCDate", TypeDate, &File::GetDateCreation);
		class_add_func("mtime", common_types.date, &os::fs::FileStream::mtime);
		//class_add_func("getADate", TypeDate, &FileStream::GetDateAccess);
		class_add_func("size", common_types.i32, &KabaFileStream::size32);
		class_add_func("pos", common_types.i32, &KabaFileStream::_pos);//os::fs::FileStream::get_pos);
		class_add_func("set_pos", common_types._void, &KabaFileStream::_set_pos, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("pos", common_types.i32);
		class_add_func("seek", common_types._void, &KabaFileStream::_seek, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("delta", common_types.i32);
		class_add_func("is_end", common_types._bool, &KabaFileStream::is_end);
		{
			os::fs::FileStream my_instance(-1, Stream::Mode::NONE);
			class_link_vtable(*(void***)&my_instance);
		}


	add_class(TypeFileError);
		class_derive_from(common_types.exception);
		class_add_func(Identifier::func::Init, common_types._void, &KabaFileError::__init__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, &KabaFileError::__delete__, Flags::Override | Flags::Mutable);
		class_set_vtable(KabaFileError);


	add_class(TypeCommandLineParser);
		class_add_func(Identifier::func::Init, common_types._void, &KabaCommandLineParser::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, &KabaCommandLineParser::__delete__, Flags::Mutable);
		class_add_func("info", common_types._void, &CommandLineParser::info, Flags::Mutable);
			func_add_param("cmd", common_types.string);
			func_add_param("i", common_types.string);
		class_add_func("show", common_types._void, &CommandLineParser::show);
		class_add_func("parse", common_types._void, &KabaCommandLineParser::parse1);
			func_add_param("arg", common_types.string_list);
		class_add_func("option", common_types._void, &KabaCommandLineParser::option1, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("comment", common_types.string);
			func_add_param("f", common_types.callback);
		class_add_func("option", common_types._void, &KabaCommandLineParser::option2, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("p", common_types.string);
			func_add_param("comment", common_types.string);
			func_add_param("f", common_types.callback_string);
		class_add_func("cmd", common_types._void, &KabaCommandLineParser::cmd1, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("p", common_types.string);
			func_add_param("comment", common_types.string);
			func_add_param("f", TypeCallbackStringList);


	add_class(common_types.os_configuration);
		class_add_element("dict", common_types.any_dict, &Configuration::map);
		class_add_func(Identifier::func::Init, common_types._void, &Configuration::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, &Configuration::__del__, Flags::Mutable);
		class_add_func("load", common_types._bool, &Configuration::load, Flags::Mutable);
			func_add_param("path", common_types.path);
		class_add_func("save", common_types._void, &Configuration::save);
			func_add_param("path", common_types.path);
		class_add_func(Identifier::func::Set, common_types._void, &Configuration::set_int, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("value", common_types.i32);
		class_add_func(Identifier::func::Set, common_types._void, &Configuration::set_float, Flags::Mutable); // FIXME: operator preference...
			func_add_param("name", common_types.string);
			func_add_param("value", common_types.f32);
		class_add_func(Identifier::func::Set, common_types._void, &Configuration::set_bool, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("value", common_types._bool);
		class_add_func(Identifier::func::Set, common_types._void, &Configuration::set_str, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("value", common_types.string);
		class_add_func(Identifier::func::Set, common_types._void, &Configuration::set, Flags::Mutable);
			func_add_param("name", common_types.string);
			func_add_param("value", common_types.any);
		class_add_func("get_int", common_types.i32, &Configuration::get_int);
			func_add_param("name", common_types.string);
			func_add_param("default", common_types.i32);
		class_add_func("get_float", common_types.f32, &Configuration::get_float);
			func_add_param("name", common_types.string);
			func_add_param("default", common_types.f32);
		class_add_func("get_bool", common_types._bool, &Configuration::get_bool);
			func_add_param("name", common_types.string);
			func_add_param("default", common_types._bool);
		class_add_func("get_str", common_types.string, &Configuration::get_str);
			func_add_param("name", common_types.string);
			func_add_param("default", common_types.string);
		class_add_func(Identifier::func::Get, common_types.any, &_os_config_get);
			func_add_param("name", common_types.string);
		class_add_func("keys", common_types.string_list, &Configuration::keys);


	// file access
	add_class(TypeFilesystem);
		class_add_func("open", TypeFileStreamXfer, &kaba_file_open, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
			func_add_param("mode", common_types.string);
		class_add_func("read", common_types.bytes, &kaba_file_read, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
		class_add_func("read_text", common_types.string, &kaba_file_read_text, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
		class_add_func("write", common_types._void, &kaba_file_write, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
			func_add_param("buffer", common_types.bytes);
		class_add_func("write_text", common_types._void, &kaba_file_write_text, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
			func_add_param("buffer", common_types.string);
		class_add_func("exists", common_types._bool, &os::fs::exists, Flags::Static);
			func_add_param("filename", common_types.path);
		class_add_func("size", common_types.i64, &os::fs::size, Flags::Static);
			func_add_param("filename", common_types.path);
		class_add_func("mtime", common_types.date, &os::fs::mtime, Flags::Static);
			func_add_param("filename", common_types.path);
		class_add_func("is_directory", common_types._bool, &os::fs::is_directory, Flags::Static);
			func_add_param("filename", common_types.path);
		class_add_func("hash", common_types.string, &kaba_file_hash, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
			func_add_param("type", common_types.string);
		class_add_func("move", common_types._void, &kaba_file_move, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", common_types.path);
			func_add_param("dest", common_types.path);
		class_add_func("rename", common_types._void, &kaba_file_rename, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", common_types.path);
			func_add_param("dest", common_types.path);
		class_add_func("copy", common_types._void, &kaba_file_copy, Flags::Static | Flags::RaisesExceptions);
			func_add_param("source", common_types.path);
			func_add_param("dest", common_types.path);
		class_add_func("delete", common_types._void, &kaba_file_delete, Flags::Static | Flags::RaisesExceptions);
			func_add_param("filename", common_types.path);
		class_add_func("search", common_types.path_list, &os::fs::search, Flags::Static);
			func_add_param("dir", common_types.path);
			func_add_param("filter", common_types.string);
			func_add_param("options", common_types.string);
		class_add_func("create_directory", common_types._void, &kaba_dir_create, Flags::Static | Flags::RaisesExceptions);
			func_add_param("dir", common_types.path);
		class_add_func("delete_directory", common_types._void, &kaba_dir_delete, Flags::Static | Flags::RaisesExceptions);
			func_add_param("dir", common_types.path);
		class_add_func("current_directory", common_types.path, &os::fs::current_directory, Flags::Static);
		class_add_func("set_current_directory", common_types._void, &os::fs::set_current_directory, Flags::Static);
			func_add_param("dir", common_types.path);
		class_add_const("home_directory", common_types.path, &os::app::home_directory);
		
		if (!_kaba_stdin)
			_kaba_stdin = new os::fs::FileStream(0, Stream::Mode::TEXT);
		add_ext_var("stdin", TypeFileStreamSharedNN, &_kaba_stdin);


	add_class(TypeTerminal);
		class_add_const("RED", common_types.string, &os::terminal::RED);
		class_add_const("GREEN", common_types.string, &os::terminal::GREEN);
		class_add_const("BLUE", common_types.string, &os::terminal::BLUE);
		class_add_const("YELLOW", common_types.string, &os::terminal::YELLOW);
		class_add_const("ORANGE", common_types.string, &os::terminal::ORANGE);
		class_add_const("CYAN", common_types.string, &os::terminal::CYAN);
		class_add_const("MAGENTA", common_types.string, &os::terminal::MAGENTA);
		class_add_const("GRAY", common_types.string, &os::terminal::GRAY);
		class_add_const("DARK_GRAY", common_types.string, &os::terminal::DARK_GRAY);
		class_add_const("BOLD", common_types.string, &os::terminal::BOLD);
		class_add_const("END", common_types.string, &os::terminal::END);


	add_class(TypeApp);
		class_add_const("directory_dynamic", common_types.path, &os::app::directory_dynamic);
		class_add_const("directory_static", common_types.path, &os::app::directory_static);
		class_add_const("user_name", common_types.string, &os::app::user_name);


	// system
	add_func("shell_execute", common_types.string, &kaba_shell_execute, Flags::Static | Flags::RaisesExceptions);
		func_add_param("cmd", common_types.string);
		func_add_param_def("verbose", common_types._bool, false);

	add_func("exit", common_types._void, &kaba_os_exit, Flags::Static);
		func_add_param("status", common_types.i32);


	add_ext_var("app_directory_dynamic", common_types.path, &os::app::directory_dynamic);
	add_ext_var("app_directory_static", common_types.path, &os::app::directory_static);


	add_type_cast(50, common_types.string, common_types.path, "os.Path.@from_str");
}

};
