/*
 * Constant.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_CONSTANT_H_
#define SRC_LIB_KABA_SYNTAX_CONSTANT_H_


#include "../../base/base.h"

class complex;

namespace kaba {

class Class;
class Block;
class SyntaxTree;


class Value {
public:
	string value;
	shared<const Class> type;

	Value();
	~Value();

	static bool can_init(const Class *type);
	void init(const Class *type);
	void clear();
	void set(const Value &v);

	void* p() const;
	int& as_int() const;
	int64& as_int64() const;
	float& as_float() const;
	double& as_float64() const;
	complex& as_complex() const;
	string& as_string() const;
	DynamicArray& as_array() const;

	int mapping_size() const;
	void map_into(char *mem, char *addr) const;
	string str() const;
};

// for any type of constant used in the module
class Constant : public Sharable<Value> {
public:
	Constant(const Class *type, SyntaxTree *owner);
	string name;
	string str() const;
	void *address_compiler;
	void *address_runtime;
	bool used;
	SyntaxTree *owner;
	
	void init(const Class *type);
	void set(const Value &v);
};


}



#endif /* SRC_LIB_KABA_SYNTAX_CONSTANT_H_ */
