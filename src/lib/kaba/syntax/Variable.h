/*
 * Variable.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"
#include "../../base/pointer.h"

namespace kaba {

class Class;
enum class Flags;


class Variable : public Sharable<Empty> {
public:
	Variable(const string &name, const Class *type);
	~Variable();
	const Class *type; // for creating instances
	string name;
	string long_name(const Class *ns) const;
	string cname(const Class *ns, const Class *ns_obs) const;
	int64 _offset; // for compilation
	void *memory;
	bool memory_owner;
	Flags flags;
	bool is_extern() const;
	bool is_const() const;
	bool explicitly_constructed;
	int _label;
};



}
