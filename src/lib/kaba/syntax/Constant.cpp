/*
 * Constant.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../dynamic/dynamic.h"
#include "../asm/asm.h"
#include <stdio.h>

namespace kaba {


void rec_assign(void *a, void *b, const Class *type);


Value::Value() {
}

Value::~Value() {
	clear();
}

bool Value::can_init(const Class *t) {
	if (!t->needs_constructor())
		return true;
	if (t->is_super_array())
		return true;
	return false;
}

int host_size(const Class *_type) {
	if (_type->is_super_array())
		return sizeof(DynamicArray);
	return _type->size;
}

void Value::init(const Class *_type) {
	clear();
	type = _type;

	if (type->is_super_array()) {
		value.resize(sizeof(DynamicArray));
		as_array().init(host_size(type->get_array_element()));
	} else {
		value.resize(max(type->size, (long long)16));
	}
}

void Value::clear() {
	if (type)
		if (type->is_super_array())
			as_array().simple_clear();

	value.clear();
	type = nullptr;
}

void Value::set(const Value &v) {
	init(v.type.get());
	rec_assign(p(), v.p(), type.get());
}

void* Value::p() const {
	return value.data;
}

int& Value::as_int() const {
	return *(int*)value.data;
}

long long& Value::as_int64() const {
	return *(long long*)value.data;
}

float& Value::as_float() const {
	return *(float*)value.data;
}

double& Value::as_float64() const {
	return *(double*)value.data;
}

complex& Value::as_complex() const {
	return *(complex*)value.data;
}

string& Value::as_string() const {
	return *(string*)value.data;
}

DynamicArray& Value::as_array() const {
	return *(DynamicArray*)p();
}

int map_size_complex(void *p, const Class *type) {
	if (type == TypeCString)
		return strlen((char*)p) + 1;
	if (type->is_super_array()) {
		int size = config.super_array_size;
		auto *ar = (DynamicArray*)p;
		for (int i=0; i<ar->num; i++)
			size += map_size_complex((char*)ar->data + i * ar->element_size, type->get_array_element());
		return size;
	}
	return type->size;
}

int Value::mapping_size() const {
	return map_size_complex(p(), type.get());
}

// map directly into <memory>
// additional data (array ...) into free parts after <locked>
// direct memory always pre-locked!
char *map_into_complex(char *memory, char *locked, long addr_off, char *p, const Class *type) {
	if (type->is_super_array()) {
		DynamicArray *ar = (DynamicArray*)p;

		auto *el = type->get_array_element();

		int direct_size = config.super_array_size;
		int indirect_size = el->size * ar->num; // target!
		char *ar_target = locked;
		locked += indirect_size;

		*(void**)&memory[0] = ar_target + addr_off; // .data
		*(int*)&memory[config.pointer_size    ] = ar->num;
		*(int*)&memory[config.pointer_size + 4] = 0; // .reserved
		*(int*)&memory[config.pointer_size + 8] = el->size; // target size!

		if (type->param[0]->is_super_array()) {
			for (int i=0; i<ar->num; i++) {
				int el_offset = i * el->size;
				int el_offset_host = i * ar->element_size;
				locked = map_into_complex(ar_target + el_offset, locked, addr_off, (char*)ar->data + el_offset_host, type->param[0]);
			}

		} else {
			memcpy(ar_target, ar->data, indirect_size);
		}
		return locked;
	} else if (type == TypeCString) {
		strcpy(memory, (char*)p);
		return memory + strlen((char*)p) + 1; // NO RECURSION!!!
	} else if (type->can_memcpy()) {
		memcpy(memory, p, type->size);
	} else {
		// TEST ME....
		for (auto &el: type->elements) {
			if (!el.hidden())
				locked = map_into_complex(memory + el.offset, locked, addr_off, p + el.offset, el.type);
		}
	}
	return locked;
}

void Value::map_into(char *memory, char *addr) const {
	map_into_complex(memory, memory + type->size, addr - memory, (char*)p(), type.get());
}

string Value::str() const {
	return var_repr(value.data, type.get());
}

Constant::Constant(const Class *_type, SyntaxTree *_owner) {
	Value::init(_type);
	name = "-none-";
	owner = _owner;
	used = false;
	address = p();
}

void Constant::init(const Class *_type) {
	Value::init(_type);
	address = p();
}

void Constant::set(const Value &v) {
	Value::set(v);
	address = p();
}

string Constant::str() const {
	return Value::str();
}

}

