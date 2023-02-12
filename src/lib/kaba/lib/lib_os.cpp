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
#include "../dynamic/exception.h"
#include "../../base/callable.h"

class vec3;


namespace kaba {


extern const Class *TypeDate;
extern const Class *TypeStringList;
const Class *TypePath;
const Class *TypePathList;
const Class *TypeStreamP;
//const Class *TypeStreamSP;

const Class* TypeCallback;
const Class* TypeCallbackString;

extern const Class *TypeStringList;
extern const Class *TypeAny;
const Class *TypeOsConfiguration;


Any _os_config_get(Configuration &c, const string &key) {
	return c.get(key, Any());
}


static Stream *_kaba_stdin = nullptr;


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

class KabaFileStream : public os::fs::FileStream {
public:
	void _cdecl __delete__() {
		this->~KabaFileStream();
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
	int _get_pos() {
		return get_pos();
	}
	void _set_pos(int pos) {
		set_pos(pos);
	}
	void _seek(int delta) {
		seek(delta);
	}
	int _get_size32() {
		return get_size32();
	}
};

template<class F>
class KabaFormatter : public F {
public:
	/*void __init__(shared<Stream> stream) {
		msg_write("KabaFormatter init " + p2s(this));
		msg_write("  stream " + p2s(stream.get()));
		msg_write("  stream.count " + i2s(stream->_pointer_ref_counter));
		msg_write(stream->get_pos());
		new(this) F(stream);
	}*/
	void __init__(Stream* stream) {
		new(this) F(stream);
	}
	void __delete__() {
		this->F::~F();
	}
	void _cdecl _read_int(int &i) {
		KABA_EXCEPTION_WRAPPER(i = F::read_int());
	}
	void _cdecl _read_float(float &f) {
		 KABA_EXCEPTION_WRAPPER(f = F::read_float());
	}
	void _cdecl _read_bool(bool &b) {
		KABA_EXCEPTION_WRAPPER(b = F::read_bool());
	}
	void _cdecl _read_vector(vec3 &v) {
		KABA_EXCEPTION_WRAPPER(F::read_vector(&v));
	}
	void _cdecl _read_str(string &s) {
		KABA_EXCEPTION_WRAPPER(s = F::read_str());
	}
	void _cdecl _write_vector(const vec3 &v) {
		KABA_EXCEPTION_WRAPPER(F::write_vector(&v));
	}

	void _write_int(int i) { F::write_int(i); }
	void _write_float(float f) { F::write_float(f); }
	void _write_bool(bool b) { F::write_bool(b); }
	void _write_str(const string &s) { F::write_str(s); }

	static void declare(const Class *c) {
		using KF = KabaFormatter<F>;
		add_class(c);
			//class_add_element("stream", TypeStreamSP, &KF::stream);
		class_add_element("stream", TypeStreamP, &KF::stream);
			class_add_func(Identifier::Func::INIT, TypeVoid, &KF::__init__);
				//func_add_param("stream", TypeStreamSP);
				func_add_param("stream", TypeStreamP);
			class_add_func(Identifier::Func::DELETE, TypeVoid, &KF::__delete__);
			class_add_func("__lshift__", TypeVoid, &KF::_write_bool, Flags::RAISES_EXCEPTIONS);
				func_add_param("b", TypeBool);
			class_add_func("__lshift__", TypeVoid, &KF::_write_int, Flags::RAISES_EXCEPTIONS);
				func_add_param("i", TypeInt);
			class_add_func("__lshift__", TypeVoid, &KF::_write_float, Flags::RAISES_EXCEPTIONS);
				func_add_param("x", TypeFloat32);
			class_add_func("__lshift__", TypeVoid, &KF::_write_vector, Flags::RAISES_EXCEPTIONS);
				func_add_param("v", TypeVec3);
			class_add_func("__lshift__", TypeVoid, &KF::_write_str, Flags::RAISES_EXCEPTIONS);
				func_add_param("s", TypeString, Flags::OUT);
			class_add_func("__rshift__", TypeVoid, &KF::_read_bool, Flags::RAISES_EXCEPTIONS);
				func_add_param("b", TypeBool, Flags::OUT);
			class_add_func("__rshift__", TypeVoid, &KF::_read_int, Flags::RAISES_EXCEPTIONS);
				func_add_param("i", TypeInt, Flags::OUT);
			class_add_func("__rshift__", TypeVoid, &KF::_read_float, Flags::RAISES_EXCEPTIONS);
				func_add_param("x", TypeFloat32, Flags::OUT);
			class_add_func("__rshift__", TypeVoid, &KF::_read_vector, Flags::RAISES_EXCEPTIONS);
				func_add_param("v", TypeVec3, Flags::OUT);
			class_add_func("__rshift__", TypeVoid, &KF::_read_str, Flags::RAISES_EXCEPTIONS);
				func_add_param("s", TypeString, Flags::OUT);
	}
};

/*class KabaFileNotFoundError : public KabaFileError
{ public: KabaFileNotFoundError(const string &t) : KabaFileError(t){} };

class KabaFileNotWritableError : public KabaFileError
{ public: KabaFileNotWritableError(const string &t) : KabaFileError(t){} };*/



os::fs::FileStream* kaba_file_open(const Path &filename, const string &mode) {
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


string _cdecl kaba_shell_execute(const string &cmd) {
	try {
		return os::terminal::shell_execute(cmd);
	} catch(::Exception &e) {
		kaba_raise_exception(new KabaException(e.message()));
	}
	return "";
}


#pragma GCC pop_options

class KabaPath : public Path {
public:
	Path lshift_p(const Path &p) const {
		return *this | p;
	}
	Path lshift_s(const string &p) const {
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

void SIAddPackageOSPath(Context *c) {
	add_package(c, "os");

	TypePath = add_type("Path", config.super_array_size);

	add_class(TypePath);
		class_add_element_x("_s", TypeString, 0);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Path::__init__);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Path::__init_ext__, Flags::AUTO_CAST);
			func_add_param("p", TypeString);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &Path::__delete__);
		class_add_func("absolute", TypePath, &Path::absolute, Flags::CONST);
		class_add_func("dirname", TypeString, &Path::dirname, Flags::PURE);
		class_add_func("basename", TypeString, &Path::basename, Flags::PURE);
		class_add_func("basename_no_ext", TypeString, &Path::basename_no_ext, Flags::PURE);
		class_add_func("extension", TypeString, &Path::extension, Flags::PURE);
		class_add_func("canonical", TypePath, &Path::canonical, Flags::PURE);
		class_add_func(Identifier::Func::STR, TypeString, &Path::str, Flags::PURE);
		class_add_func("is_empty", TypeBool, &Path::is_empty, Flags::PURE);
		class_add_func("is_relative", TypeBool, &Path::is_relative, Flags::PURE);
		class_add_func("is_absolute", TypeBool, &Path::is_absolute, Flags::PURE);
		class_add_func("__bool__", TypeBool, &KabaPath::__bool__, Flags::PURE);
		class_add_func("parent", TypePath, &Path::parent, Flags::PURE);
		class_add_func("compare", TypeInt, &Path::compare, Flags::PURE);
			func_add_param("p", TypePath);
		class_add_func("relative_to", TypePath, &Path::relative_to, Flags::PURE);
			func_add_param("p", TypePath);
		class_add_const("EMPTY", TypePath, &Path::EMPTY);
		class_add_func("@from_str", TypePath, &KabaPath::from_str, Flags::STATIC | Flags::PURE);
			func_add_param("p", TypeString);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePath, TypePath, InlineID::NONE, &Path::operator =);
		add_operator(OperatorID::EQUAL, TypeBool, TypePath, TypePath, InlineID::NONE, &Path::operator ==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypePath, TypePath, InlineID::NONE, &Path::operator !=);
		add_operator(OperatorID::SMALLER, TypeBool, TypePath, TypePath, InlineID::NONE, &Path::operator <);
		add_operator(OperatorID::GREATER, TypeBool, TypePath, TypePath, InlineID::NONE, &Path::operator >);
		add_operator(OperatorID::SHIFT_LEFT, TypePath, TypePath, TypePath, InlineID::NONE, &KabaPath::lshift_p);
		add_operator(OperatorID::SHIFT_LEFT, TypePath, TypePath, TypeString, InlineID::NONE, &KabaPath::lshift_s);
		add_operator(OperatorID::BIT_OR, TypePath, TypePath, TypePath, InlineID::NONE, &KabaPath::lshift_p);
		add_operator(OperatorID::BIT_OR, TypePath, TypePath, TypeString, InlineID::NONE, &KabaPath::lshift_s);
		add_operator(OperatorID::IN, TypeBool, TypePath, TypePath, InlineID::NONE, &KabaPath::__contains__);


	// AFTER TypePath!
	TypePathList = add_type_l(TypePath);

	add_class(TypePath);
		class_add_func("all_parents", TypePathList, &Path::all_parents, Flags::PURE);

	add_class(TypePathList);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<Path>::__init__);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &Array<Path>::clear);
		class_add_func("clear", TypeVoid, &Array<Path>::clear);
		class_add_func("add", TypeVoid, &Array<Path>::add);
			func_add_param("p", TypePath);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePathList, TypePathList, InlineID::NONE, &PathList::assign);
		add_operator(OperatorID::IN, TypeBool, TypePathList, TypePath, InlineID::NONE, &PathList::__contains__);
		add_operator(OperatorID::ADD, TypePathList, TypePathList, TypePathList, InlineID::NONE, &PathList::__add__);
		add_operator(OperatorID::ADDS, TypeVoid, TypePathList, TypePathList, InlineID::NONE, &PathList::__adds__);
}


char _el_off_data[1024];
#define evil_member_offset(C, M)	((int_p)((char*)&(reinterpret_cast<C*>(&_el_off_data[0])->M) - &_el_off_data[0]))

template<class C>
class KabaSharedPointer : public shared<C> {
public:
	void __init__() {
		msg_write("new Shared Pointer");
		new(this) shared<C>;
	}
	void __delete__() {
		msg_write("del Shared Pointer");
		this->shared<C>::~shared<C>();
	}
	void assign(shared<C> o) {
		msg_write("Shared Pointer ass1");
		*(shared<C>*)this = o;
	}
	void assign_p(C *o) {
		msg_write("Shared Pointer ass2");
		*(shared<C>*)this = o;
	}
	shared<C> create(C *p) {
		msg_write("Shared Pointer create");
		msg_write("  p: " + p2s(p));
		msg_write("  p.count: " + i2s(p->_pointer_ref_counter));
		shared<C> sp;
		sp = p;
		msg_write(p2s(sp.get()));
		msg_write(p->_pointer_ref_counter);
		return sp;//shared<C>(p);
	}

	static void declare(const Class *c) {
		using SP = KabaSharedPointer<C>;
		add_class(c);
			class_add_func(Identifier::Func::INIT, TypeVoid, &SP::__init__);
			class_add_func(Identifier::Func::DELETE, TypeVoid, &SP::__delete__);
			class_add_func(Identifier::Func::SHARED_CLEAR, TypeVoid, &shared<C>::release);
			class_add_func(Identifier::Func::ASSIGN, TypeVoid, &SP::assign);
				func_add_param("other", c);
			class_add_func(Identifier::Func::ASSIGN, TypeVoid, &SP::assign_p);
				func_add_param("other", c->owner->get_pointer(c->param[0], -1));
			class_add_func(Identifier::Func::SHARED_CREATE, c, &SP::create, Flags::STATIC);
				func_add_param("other", c->owner->get_pointer(c->param[0], -1));
	}
};

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

	const Class *TypeStream = add_type("Stream", sizeof(Stream));
	TypeStreamP = add_type_p(TypeStream);
	//TypeStreamSP = add_type_p(TypeStream, Flags::SHARED);
	const Class *TypeFileStream = add_type("FileStream", sizeof(os::fs::FileStream));
	const Class *TypeFileStreamP = add_type_p(TypeFileStream);
	const Class *TypeBinaryFormatter = add_type("BinaryFormatter", sizeof(BinaryFormatter));
	const Class *TypeTextLinesFormatter = add_type("TextLinesFormatter", sizeof(TextLinesFormatter));
	const Class *TypeFilesystem = add_type("fs", 0);
	const Class *TypeFileError = add_type("FileError", sizeof(KabaFileError));
	//Class *TypeFileNotFoundError= add_type  ("FileError", sizeof(KabaFileNotFoundError));
	//Class *TypeFileNotWritableError= add_type  ("FileError", sizeof(KabaFileNotWritableError));
	auto TypeCommandLineParser = add_type("CommandLineParser", sizeof(CommandLineParser));
	TypeOsConfiguration = add_type("Configuration", sizeof(Configuration));
	const Class *TypeTerminal = add_type("terminal", 0);

	TypeCallback = add_type_f(TypeVoid, {});
	TypeCallbackString = add_type_f(TypeVoid, {TypeString});
	auto TypeCallbackStringList = add_type_f(TypeVoid, {TypeStringList});

	add_class(TypeStream);
		class_add_element(Identifier::SHARED_COUNT, TypeInt, evil_member_offset(os::fs::FileStream, _pointer_ref_counter));
		// FIXME &FileStream::_pointer_ref_counter does not work here
		// we get a base-class-pointer... \(O_O)/
		//class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &Stream::__delete__);
		//class_add_func_virtual("write", TypeVoid, Stream::write);
		//class_add_element(Identifier::SHARED_COUNT, TypeInt, &Stream::_pointer_ref_counter);
		//const_cast<Class*>(TypeStream)->elements.back().offset = offsetof(Stream, _pointer_ref_counter);


	//KabaSharedPointer<FileStream>::declare(TypeStreamSP);

	add_class(TypeFileStream);
		class_derive_from(TypeStream, false, false);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &KabaFileStream::__delete__);
		//class_add_func("getCDate", TypeDate, &File::GetDateCreation);
		class_add_func("mtime", TypeDate, &os::fs::FileStream::mtime);
		//class_add_func("getADate", TypeDate, &FileStream::GetDateAccess);
		class_add_func("get_size", TypeInt, &KabaFileStream::_get_size32);
		class_add_func("get_pos", TypeInt, &KabaFileStream::_get_pos);//os::fs::FileStream::get_pos);
		class_add_func("set_pos", TypeVoid, &KabaFileStream::_set_pos, Flags::RAISES_EXCEPTIONS);
			func_add_param("pos", TypeInt);
		class_add_func("seek", TypeVoid, &KabaFileStream::_seek, Flags::RAISES_EXCEPTIONS);
			func_add_param("delta", TypeInt);
		class_add_func("read", TypeString, &KabaFileStream::_read_size, Flags::RAISES_EXCEPTIONS);
			func_add_param("size", TypeInt);
		class_add_func("read", TypeInt, &KabaFileStream::_read_bytes, Flags::RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		class_add_func("write", TypeInt, &KabaFileStream::_write, Flags::RAISES_EXCEPTIONS);
		//class_add_func_virtual("write", TypeInt, &FileStream::write);
			func_add_param("s", TypeString);
		class_add_func("is_end", TypeBool, &KabaFileStream::is_end);
		//class_set_vtable(FileStream);


	KabaFormatter<BinaryFormatter>::declare(TypeBinaryFormatter);
	KabaFormatter<TextLinesFormatter>::declare(TypeTextLinesFormatter);


	add_class(TypeFileError);
		class_derive_from(TypeException, false, false);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaFileError::__init__, Flags::OVERRIDE);
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &KabaFileError::__delete__, Flags::OVERRIDE);
		class_set_vtable(KabaFileError);


	add_class(TypeCommandLineParser);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaCommandLineParser::__init__);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &KabaCommandLineParser::__delete__);
		class_add_func("info", TypeVoid, &CommandLineParser::info);
			func_add_param("cmd", TypeString);
			func_add_param("i", TypeString);
		class_add_func("show", TypeVoid, &CommandLineParser::show);
		class_add_func("parse", TypeVoid, &KabaCommandLineParser::parse1);
			func_add_param("arg", TypeStringList);
		class_add_func("option", TypeVoid, &KabaCommandLineParser::option1);
			func_add_param("name", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallback);
		class_add_func("option", TypeVoid, &KabaCommandLineParser::option2);
			func_add_param("name", TypeString);
			func_add_param("p", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallbackString);
		class_add_func("cmd", TypeVoid, &KabaCommandLineParser::cmd1);
			func_add_param("name", TypeString);
			func_add_param("p", TypeString);
			func_add_param("comment", TypeString);
			func_add_param("f", TypeCallbackStringList);


	add_class(TypeOsConfiguration);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Configuration::__init__);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &Configuration::__del__);
		class_add_func("load", TypeBool, &Configuration::load);
			func_add_param("path", TypePath);
		class_add_func("save", TypeVoid, &Configuration::save, Flags::CONST);
			func_add_param("path", TypePath);
		class_add_func(Identifier::Func::SET, TypeVoid, &Configuration::set_int);
			func_add_param("name", TypeString);
			func_add_param("value", TypeInt);
		class_add_func(Identifier::Func::SET, TypeVoid, &Configuration::set_float); // FIXME: operator preference...
			func_add_param("name", TypeString);
			func_add_param("value", TypeFloat32);
		class_add_func(Identifier::Func::SET, TypeVoid, &Configuration::set_bool);
			func_add_param("name", TypeString);
			func_add_param("value", TypeBool);
		class_add_func(Identifier::Func::SET, TypeVoid, &Configuration::set_str);
			func_add_param("name", TypeString);
			func_add_param("value", TypeString);
		class_add_func(Identifier::Func::SET, TypeVoid, &Configuration::set);
			func_add_param("name", TypeString);
			func_add_param("value", TypeAny);
		class_add_func("get_int", TypeInt, &Configuration::get_int, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeInt);
		class_add_func("get_float", TypeFloat32, &Configuration::get_float, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeFloat32);
		class_add_func("get_bool", TypeBool, &Configuration::get_bool, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeBool);
		class_add_func("get_str", TypeString, &Configuration::get_str, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeString);
		class_add_func(Identifier::Func::GET, TypeAny, &_os_config_get, Flags::CONST);
			func_add_param("name", TypeString);
		class_add_func("keys", TypeStringList, &Configuration::keys, Flags::CONST);


	// file access
	add_class(TypeFilesystem);
		class_add_func("open", TypeFileStreamP, &kaba_file_open, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("mode", TypeString);
		class_add_func("read", TypeString, &kaba_file_read, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_func("read_text", TypeString, &kaba_file_read_text, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_func("write", TypeVoid, &kaba_file_write, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeString);
		class_add_func("write_text", TypeVoid, &kaba_file_write_text, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("buffer", TypeString);
		class_add_func("exists", TypeBool, &os::fs::exists, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_func("size", TypeInt64, &os::fs::size, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_func("mtime", TypeDate, &os::fs::mtime, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_func("is_directory", TypeBool, &os::fs::is_directory, Flags::STATIC);
			func_add_param("filename", TypePath);
		class_add_func("hash", TypeString, &kaba_file_hash, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
			func_add_param("type", TypeString);
		class_add_func("move", TypeVoid, &kaba_file_move, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("rename", TypeVoid, &kaba_file_rename, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("copy", TypeVoid, &kaba_file_copy, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("source", TypePath);
			func_add_param("dest", TypePath);
		class_add_func("delete", TypeVoid, &kaba_file_delete, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("filename", TypePath);
		class_add_func("search", TypePathList, &os::fs::search, Flags::STATIC);
			func_add_param("dir", TypePath);
			func_add_param("filter", TypeString);
			func_add_param("options", TypeString);
		class_add_func("create_directory", TypeVoid, &kaba_dir_create, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("dir", TypePath);
		class_add_func("delete_directory", TypeVoid, &kaba_dir_delete, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("dir", TypePath);
		class_add_func("current_directory", TypePath, &os::fs::current_directory, Flags::STATIC);
		
		if (!_kaba_stdin)
			_kaba_stdin = new os::fs::FileStream(0);
		add_ext_var("stdin", TypeFileStreamP, &_kaba_stdin);
	
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
	add_func("shell_execute", TypeString, &kaba_shell_execute, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("cmd", TypeString);


	add_type_cast(50, TypeString, TypePath, "os.Path.@from_str");
}

};
