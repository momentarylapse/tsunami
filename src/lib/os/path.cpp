/*
 * path.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: michi
 */

#include "path.h"
#include "filesystem.h"

#include "msg.h"

//const string SEPARATOR = "/";
//const string SEPARATOR_OTHER = "\\";
// const version creates badly compiled code...
#define SEPARATOR "/"
#define SEPARATOR_OTHER "\\"

// internal format:
// * always /
// * no double //
// * ending / allowed

const Path Path::EMPTY;

Path::Path() {
}

Path::Path(const string &_s) {
	s = _s.replace(SEPARATOR_OTHER, SEPARATOR).replace("//", "/");
}

Path::Path(const char *_s) : Path(string(_s)) {}

void Path::__init__() {
	new(this) Path();
}

void Path::__init_ext__(const string &s) {
	new(this) Path(s);
}

void Path::__delete__() {
	this->~Path();
}

void Path::operator =(const Path &p) {
	s = p.s;
}

void Path::operator |=(const Path &p) {
	if (is_empty())
		s = p.s;
	else if (has_dir_ending())
		s += p.s;
	else
		s += SEPARATOR + p.s;
}

Path Path::operator |(const Path &p) const {
	Path temp = *this;
	temp |= p;
	return temp;
}

int icomparex(const string &a, const string &b) {
	int i = a.icompare(b);
	if (i == 0)
		return a.compare(b);
	return i;
}

// * compare literally!
// * ignore / at the end
// * NOPE: ignore recursion
int Path::compare(const Path &p) const {
	//return icomparex(canonical().as_dir().s, p.canonical().as_dir().s);
	if (has_dir_ending() and !p.has_dir_ending())
		return icomparex(s.sub_ref(0, -1), p.s);
	if (!has_dir_ending() and p.has_dir_ending())
		return icomparex(s, p.s.sub_ref(0, -1));
	return icomparex(s, p.s);
}

bool Path::operator ==(const Path &p) const {
	return compare(p) == 0;
}

bool Path::operator !=(const Path &p) const {
	return !(*this == p);
}

bool Path::operator <(const Path &p) const {
	return compare(p) < 0;
}

bool Path::operator >(const Path &p) const {
	return compare(p) > 0;
}

bool Path::operator <=(const Path &p) const {
	return compare(p) <= 0;
}

bool Path::operator >=(const Path &p) const {
	return compare(p) >= 0;
}

Path::operator bool() const {
	return !is_empty();
}


// transposes path-strings to the current operating system
// accepts windows and linux paths ("/" and "\\")
string Path::str() const {
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	return s.replace("/", "\\");
#else
	return s.replace("\\", "/");
#endif
}

string Path::repr() const {
	return str().repr();
}

const char *Path::c_str() const {
	return str().c_str();
}

bool Path::is_relative() const {
	if (is_empty())
		return true;
	return (s.head(1) != SEPARATOR) and (s.sub(1,3) != ":/");
}

bool Path::is_absolute() const {
	if (is_empty())
		return false;
	return (s.head(1) == SEPARATOR) or (s.sub(1,3) == ":/");
}

bool Path::is_in(const Path &p) const {
	if (p.is_empty())
		return !is_empty();
	string dir = p.canonical().as_dir().s;
	return s.head(dir.num) == dir;
}

bool Path::is_empty() const {
	return s.num == 0;
}

bool Path::has_dir_ending() const {
	return s.tail(1) == SEPARATOR;
}

string Path::basename() const {
	int i = s.sub(0, -1).rfind(SEPARATOR);
	if (i >= 0)
		return s.tail(s.num - i - 1).replace(SEPARATOR, "");
	return s;
}

string Path::basename_no_ext() const {
	string b = basename();
	int pos = b.rfind(".");
	if (pos >= 0)
		return b.head(pos);
	return "";
}

Path Path::no_ext() const {
	int pos = s.rfind(".");
	if (pos >= 0)
		return s.head(pos);
	return *this;
}

string Path::extension() const {
	string b = basename();
	int pos = b.rfind(".");
	if (pos >= 0)
		return b.tail(b.num - pos - 1).lower();
	return "";
}

Path Path::with(const string &_s) const {
	return s + _s;
}

// ends with '/' or '\'
string Path::dirname() const {
	return parent().str();
}

Path Path::absolute() const {
	if (is_relative())
		return os::fs::current_directory() | *this;
	return *this;
}

// either '/' or 'c:/'
Path Path::root() const {
	if (is_relative())
		return EMPTY;
	return Path(s.explode(SEPARATOR)[0]).as_dir();
}

Path Path::relative_to(const Path &p) const {
	if (p.is_empty())
		return *this;
	string dir = p.canonical().as_dir().s;
	string me = canonical().s;
	if (me.head(dir.num) != dir)
		return EMPTY;
	return Path(me.sub(dir.num));
}

// cancel '/x/../'
// deal with '/./'
// leave './x'
Path Path::canonical() const {
	return _canonical_remove(0, true, has_dir_ending());
}

// make sure the name ends with a slash
Path Path::as_dir() const {
	if (has_dir_ending())
		return *this;
	return Path(s + SEPARATOR);
}


Path Path::_canonical_remove(int n_remove, bool keep_going, bool make_dir) const {
	auto xx = s.explode(SEPARATOR);
	for (int i=xx.num-1; i>=0 and ((n_remove > 0) or keep_going); i--) {
		if (i == 0 and n_remove > 0) {
			if (is_absolute() or xx[i] == ".")
				return EMPTY; // ERROR
		}

		if (xx[i] == "") {
			if (i > 0 or is_relative())
				n_remove ++;
		} else if (xx[i] == ".") {
			if (i > 0)
				n_remove ++;
		} else if (xx[i] == "..") {
			n_remove += 2;
		}
		if (n_remove > 0) {
			xx.erase(i);
			n_remove --;
		}
	}
	if (n_remove > 0 and is_absolute())
		return EMPTY; // ERROR
	if (xx.num == 0 and is_relative())
		return EMPTY; // ERROR
	auto pp = Path(string("../").repeat(n_remove) + implode(xx, SEPARATOR));
	if (make_dir)
		return pp.as_dir();
	return pp;
}

Path Path::parent() const {
	return _canonical_remove(1, false, true);//.as_dir();
}

// starts from root
// not including self
Array<Path> Path::all_parents() const {
	Array<Path> parents;
	auto p = parent();
	while (!p.is_empty()) {
		parents.add(p);
		p = p.parent();
	}
	return parents;
}



template<> string _xf_str_(const string &f, const Path &value) {
	return value.str();
	/*auto ff = xf_parse(f);
	if (ff.type == 's') {
	} else {
		throw Exception("format evil (Path): " + f);
	}
	return ff.apply_justify(value.str());*/
}

template<> string _xf_str_(const string &f, Path value) { return _xf_str_<const Path&>(f, value); }

template<> string repr(const Path& p) {
	return p.repr();
}
