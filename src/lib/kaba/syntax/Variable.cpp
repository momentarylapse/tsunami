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

Variable::Variable(const string& _name, const Class* _type, const Class* _ns, int _token_id) {
	name = _name;
	type = _type;
	ns = _ns;
	token_id = _token_id;
	_offset = 0;
	flags = Flags::Mutable;
	explicitly_constructed = false;
	memory = nullptr;
	memory_owner = false;
	_label = -1;
}

Variable::~Variable() {
	if (memory_owner)
		free(memory);
}

string Variable::long_name(const Class *_ns) const {
	return namespacify_rel(name, _ns, nullptr);
}

string Variable::cname(const Class *_ns, const Class *ns_obs) const {
	return namespacify_rel(name, _ns, ns_obs);
}

bool Variable::is_mutable() const {
	return flags_has(flags, Flags::Mutable);
}

bool Variable::is_extern() const {
	return flags_has(flags, Flags::Extern);
}


}
