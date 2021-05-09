/*
 * Variable.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */


#include "Variable.h"
#include "Flags.h"

namespace kaba {

string namespacify_rel(const string &name, const Class *name_space, const Class *observer_ns);

Variable::Variable(const string &_name, const Class *_type) {
	name = _name;
	type = _type;
	_offset = 0;
	flags = Flags::NONE;
	explicitly_constructed = false;
	memory = nullptr;
	memory_owner = false;
	_label = -1;
}

Variable::~Variable() {
	if (memory_owner)
		free(memory);
}

string Variable::long_name(const Class *ns) const {
	return namespacify_rel(name, ns, nullptr);
}

string Variable::cname(const Class *ns, const Class *ns_obs) const {
	return namespacify_rel(name, ns, ns_obs);
}

bool Variable::is_const() const {
	return flags_has(flags, Flags::CONST);
}

bool Variable::is_extern() const {
	return flags_has(flags, Flags::EXTERN);
}


}
