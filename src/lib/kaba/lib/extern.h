/*
 * extern.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"

namespace kaba {

class Context;


template<typename T>
void* mf(T tmf) {
	union {
		T f;
		struct {
			int_p a;
			int_p b;
		};
	} pp;
	pp.a = 0;
	pp.b = 0;
	pp.f = tmf;

	// on ARM the "virtual bit" is in <b>, on x86 it is in <a>
	return (void*)(pp.a | (pp.b & 1));
}

class ExternalLinkData {
public:

	Context* context;
	struct ExternalLink {
		string name;
		void* pointer;
	};
	Array<ExternalLink> external_links;

	// also enum values
	struct ClassOffset {
		string class_name, element;
		int offset;
		bool is_virtual;
	};
	Array<ClassOffset> class_offsets;

	struct ClassSize {
		string class_name;
		int size;
	};
	Array<ClassSize> class_sizes;

	explicit ExternalLinkData(Context* c);

	void reset();
	void link(const string& name, void* pointer);
	template<typename T>
	void link_class_func(const string& name, T pointer) {
		link(name, mf(pointer));
	}
	void declare_class_size(const string& class_name, int offset);
	void _declare_class_element(const string& name, int offset);
	template<class T>
	void declare_class_element(const string& name, T pointer) {
		_declare_class_element(name, *(int*)(void*)&pointer);
	}
	void _link_virtual(const string& name, void* p, void* instance);
	template<class T>
	void link_virtual(const string& name, T pointer, void* instance) {
		_link_virtual(name, mf(pointer), instance);
	}
	template<class T>
	void declare_enum(const string& name, T value) {
		_declare_class_element(name, (int)value);
	}

	void *get_link(const string& name);
	int process_class_offset(const string& class_name, const string& element, int offset);
	int process_class_size(const string& class_name, int size);
	int process_class_num_virtuals(const string& class_name, int num_virtual);

};



}
