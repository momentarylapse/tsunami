/*
 * Constant.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Kaba{



Value::Value()
{
	type = TypeVoid;
}

Value::~Value()
{
	clear();
}

void Value::init(const Class *_type)
{
	clear();
	type = _type;

	if (type->is_super_array()){
		value.resize(sizeof(DynamicArray));
		as_array().init(type->parent->size);
	}else{
		value.resize(max(type->size, (long long)16));
	}
}

void Value::clear()
{
	if (type->is_super_array())
		as_array().simple_clear();

	value.clear();
	type = TypeVoid;
}

void Value::set(const Value &v)
{
	init(v.type);
	if (type->is_super_array()){
		as_array().simple_resize(v.as_array().num);
		memcpy(as_array().data, v.as_array().data, as_array().num * type->parent->size);

	}else{
		// plain old data
		memcpy(p(), v.p(), type->size);
	}
}

void* Value::p() const
{
	return value.data;
}

int& Value::as_int() const
{
	return *(int*)value.data;
}

long long& Value::as_int64() const
{
	return *(long long*)value.data;
}

float& Value::as_float() const
{
	return *(float*)value.data;
}

double& Value::as_float64() const
{
	return *(double*)value.data;
}

complex& Value::as_complex() const
{
	return *(complex*)value.data;
}

string& Value::as_string() const
{
	return *(string*)value.data;
}

DynamicArray& Value::as_array() const
{
	return *(DynamicArray*)p();
}

int Value::mapping_size() const
{
	if (type->is_super_array()){
		if (type->parent->is_super_array())
			throw Exception("mapping const[][]... TODO", "", 0, 0, nullptr);
		return config.super_array_size + (as_array().num * type->parent->size);
	}
	if (type == TypeCString)
		return strlen((char*)p()) + 1;

	// plain old data
	return type->size;
}

void Value::map_into(char *memory, char *addr) const
{
	if (type->is_super_array()){
		if (type->parent->is_super_array())
			throw Exception("mapping const[][]... TODO", "", 0, 0, nullptr);
		// const string -> variable length
		int size = as_array().element_size * as_array().num;
		int data_offset = config.super_array_size;

		*(void**)&memory[0] = addr + data_offset; // .data
		*(int*)&memory[config.pointer_size    ] = as_array().num;
		*(int*)&memory[config.pointer_size + 4] = 0; // .reserved
		*(int*)&memory[config.pointer_size + 8] = as_array().element_size;
		memcpy(&memory[data_offset], as_array().data, size);
	}else if (type == TypeCString){
		strcpy(memory, (char*)p());
	}else{
		memcpy(memory, p(), type->size);
	}
}

string Value::str() const
{
	return type->var2str(value.data);
}

Constant::Constant(const Class *_type, SyntaxTree *_owner)
{
	init(_type);
	name = "-none-";
	owner = _owner;
	used = false;
	address = nullptr;
}

string Constant::str() const
{
	return Value::str();
}

}

