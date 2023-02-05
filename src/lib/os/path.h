/*
 * path.h
 *
 *  Created on: Jul 30, 2020
 *      Author: michi
 */

#ifndef SRC_LIB_OS_PATH_H_
#define SRC_LIB_OS_PATH_H_

#include "../base/base.h"


class Path {
	string s;
public:
	Path();
	Path(const string &s);
	Path(const char *s);

	void __init__();
	void __init_ext__(const string &s);
	void __delete__();

	void operator=(const Path &p);
	void operator|=(const Path &p);
	Path operator|(const Path &p) const;
	bool operator==(const Path &p) const;
	bool operator!=(const Path &p) const;
	bool operator<(const Path &p) const;
	bool operator>(const Path &p) const;
	bool operator<=(const Path &p) const;
	bool operator>=(const Path &p) const;
	explicit operator bool() const;
	int compare(const Path &p) const;

	string str() const;
	const char *c_str() const;
	bool is_relative() const;
	bool is_absolute() const;
	bool is_in(const Path &p) const;
	bool is_empty() const;
	bool has_dir_ending() const;
	string basename() const;
	string basename_no_ext() const;
	Path no_ext() const;
	string extension() const;
	Path with(const string &s) const;
	string dirname() const;
	Path parent() const;
	Path canonical() const;
	Path as_dir() const;
	Array<Path> all_parents() const;
	Path root() const;
	Path relative_to(const Path &p) const;

	static const Path EMPTY;

	Path absolute() const;

private:
	Path _canonical_remove(int n_remove, bool keep_going, bool as_dir) const;
};


#endif /* SRC_LIB_OS_PATH_H_ */
