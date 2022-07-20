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
#include "../../os/msg.h"

namespace kaba {

int get_virtual_index(void *func, const string &tname, const string &name);



struct ExternalLinkData {
	string name;
	void *pointer;
};
Array<ExternalLinkData> ExternalLinks;

struct ClassOffsetData {
	string class_name, element;
	int offset;
	bool is_virtual;
};
Array<ClassOffsetData> ClassOffsets;

struct ClassSizeData {
	string class_name;
	int size;
};
Array<ClassSizeData> ClassSizes;




void reset_external_data() {
	ExternalLinks.clear();
	ClassOffsets.clear();
	ClassSizes.clear();
}

extern const Class *TypeStringAutoCast;


string decode_symbol_name(const string &name) {
	return name.replace("lib__", "").replace("@list", "[]").replace("@@", ".").replace(TypeStringAutoCast->name, "string");//.replace("@", "");
}

string function_link_name(Function *f) {
	string name = decode_symbol_name(f->cname(f->owner()->base_class));
	if (!f->is_static())
		name += ":" + decode_symbol_name(f->name_space->name);
	for (auto &p: f->literal_param_type)
		name += ":" + decode_symbol_name(p->name);
	return name;
}

// program variables - specific to the surrounding program, can't always be there...
void link_external(const string &name, void *pointer) {
	ExternalLinkData l;
	l.name = name;
	l.pointer = pointer;
	if (abs((int_p)pointer) < 1000)
		msg_error("probably a virtual function: " + name);
	ExternalLinks.add(l);


	auto names = name.explode(":");
	string sname = decode_symbol_name(names[0]);
	for (auto p: packages)
		foreachi(Function *f, p->syntax->functions, i)
			if (f->cname(p->base_class()) == sname) {
				if (names.num > 1)
					if (name != function_link_name(f))
						continue;
				f->address = (int_p)pointer;
			}
}

void *get_external_link(const string &name) {
	for (ExternalLinkData &l: ExternalLinks)
		if (l.name == name)
			return l.pointer;
	return nullptr;
}

void declare_class_size(const string &class_name, int size) {
	ClassSizeData d;
	d.class_name = class_name;
	d.size = size;
	ClassSizes.add(d);
}

void split_namespace(const string &name, string &class_name, string &element) {
	int p = name.rfind(".");
	class_name = name.sub(0, p);
	element = name.tail(name.num - p - 1);
}

void _declare_class_element(const string &name, int offset) {
	ClassOffsetData d;
	split_namespace(name, d.class_name, d.element);
	d.offset = offset;
	d.is_virtual = false;
	ClassOffsets.add(d);
}

void _link_external_virtual(const string &name, void *p, void *instance) {
	VirtualTable *v = *(VirtualTable**)instance;


	ClassOffsetData d;
	split_namespace(name, d.class_name, d.element);
	d.offset = get_virtual_index(p, d.class_name, d.element);
	d.is_virtual = true;
	ClassOffsets.add(d);

	link_external(name, v[d.offset]);
}

int process_class_offset(const string &class_name, const string &element, int offset) {
	for (auto &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.element == element))
			return d.offset;
	return offset;
}

int process_class_size(const string &class_name, int size) {
	for (auto &d: ClassSizes)
		if (d.class_name == class_name)
			return d.size;
	return size;
}

int process_class_num_virtuals(const string &class_name, int num_virtual) {
	for (auto &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.is_virtual))
			num_virtual = max(num_virtual, d.offset + 1);
	return num_virtual;
}

}


