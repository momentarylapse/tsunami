/*
 * extern.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "extern.h"
#include "lib.h"
#include "../kaba.h"
#include "../syntax/Function.h"
#include "../syntax/SyntaxTree.h"
#include "../../base/base.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

namespace kaba {

int get_virtual_index(void *func, const string &tname, const string &name);


ExternalLinkData::ExternalLinkData(Context *c) {
	context = c;
}


void ExternalLinkData::reset() {
	external_links.clear();
	class_offsets.clear();
	class_sizes.clear();
}

extern const Class *TypeStringAutoCast;

static const string LIB_LINK_PREFIX = "/";

string decode_symbol_name(const string &_name) {
	string name = _name.replace("lib__", "");
	return name.replace("@list", "[]").replace("@optional", "?").replace("@@", ".").replace(TypeStringAutoCast->name, "string");//.replace("@", "");
}

string class_link_name(const Class *c) {
	if (c == TypeStringAutoCast)
		return "string";
	if (c->owner->module->is_system_module())
		return c->long_name();
	return decode_symbol_name(c->cname(c->owner->base_class));
}

string function_link_name(Function *f) {
	string name = f->cname(f->owner()->base_class);

	// internal lib?
	if (f->owner()->module->is_system_module()) {
		if (f->owner()->module->filename != "base")
			name = str(f->owner()->module->filename) + "." + name;
		name = LIB_LINK_PREFIX + name;
	} else {
		if (name.head(5) == "lib__")
			name = LIB_LINK_PREFIX + name;
		name = decode_symbol_name(name);
	}

	// parameters
	for (auto &p: f->literal_param_type)
		name += ":" + class_link_name(p);
	return name;
}

Module *link_most_likely_internal_lib_module(Context *context, const string &name) {
	auto names = name.sub_ref(1).replace(":", ".").explode(".");

	for (auto p: weak(context->packages))
		if (str(p->filename) == names[0])
			return p;
	return context->packages[0].get(); // base
}

// program variables - specific to the surrounding program, can't always be there...
void ExternalLinkData::link(const string &name, void *pointer) {
	ExternalLink l;
	l.name = name;
	l.pointer = pointer;
	if (abs((int_p)pointer) < 1000)
		msg_error("probably a virtual function: " + name);
	external_links.add(l);

	// lib linking override?
	if (name.head(LIB_LINK_PREFIX.num) != LIB_LINK_PREFIX)
		return;

	auto names = name.sub_ref(1).explode(":");
	if (auto p = link_most_likely_internal_lib_module(context, name))
		for (auto f: p->tree->functions) {
			if (name != function_link_name(f))
					continue;
			//	msg_write("LINK  " + name + "   >>   " + f->long_name());
				f->address = (int_p)pointer;
				return;
			}
	//msg_error("can not override internal link: " + name);
}

void *ExternalLinkData::get_link(const string &name) {
	for (auto &l: external_links)
		if (l.name == name)
			return l.pointer;
	return nullptr;
}

void ExternalLinkData::declare_class_size(const string &class_name, int size) {
	ClassSize d;
	d.class_name = class_name;
	d.size = size;
	class_sizes.add(d);
}

void split_namespace(const string &name, string &class_name, string &element) {
	int p = name.rfind(".");
	class_name = name.sub(0, p);
	element = name.tail(name.num - p - 1);
}

void ExternalLinkData::_declare_class_element(const string &name, int offset) {
	ClassOffset d;
	split_namespace(name, d.class_name, d.element);
	d.offset = offset;
	d.is_virtual = false;
	class_offsets.add(d);
}

void ExternalLinkData::_link_virtual(const string &name, void *p, void *instance) {
	VirtualTable *v = *(VirtualTable**)instance;


	ClassOffset d;
	split_namespace(name, d.class_name, d.element);
	d.offset = get_virtual_index(p, d.class_name, d.element);
	d.is_virtual = true;
	class_offsets.add(d);

	link(name, v[d.offset]);
}

int ExternalLinkData::process_class_offset(const string &class_name, const string &element, int offset) {
	for (auto &d: class_offsets)
		if ((d.class_name == class_name) and (d.element == element))
			return d.offset;
	return offset;
}

int ExternalLinkData::process_class_size(const string &class_name, int size) {
	for (auto &d: class_sizes)
		if (d.class_name == class_name)
			return d.size;
	return size;
}

int ExternalLinkData::process_class_num_virtuals(const string &class_name, int num_virtual) {
	for (auto &d: class_offsets)
		if ((d.class_name == class_name) and (d.is_virtual))
			num_virtual = max(num_virtual, d.offset + 1);
	return num_virtual;
}

}


