/*
 * SerializerX.h
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_COMPILER_SERIALIZERX_H_
#define SRC_LIB_KABA_COMPILER_SERIALIZERX_H_

#include "../kaba.h"
#include "serializer.h"

namespace Asm {
	class InstructionWithParamsList;
}

namespace kaba {

class Script;
class Function;
class Block;
class Node;
class SerialNodeParam;

class SerializerX : public Serializer {
public:
	SerializerX(Script *s, Asm::InstructionWithParamsList *list);
	virtual ~SerializerX();

	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	int fc_begin(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void fc_end(int push_size, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_function_intro_params(Function *f) override;
	void add_function_intro_frame(int stack_alloc_size) override;
	void add_function_outro(Function *f) override;
	SerialNodeParam serialize_parameter(Node *link, Block *block, int index) override;
	void serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index) override;
	void serialize_inline_function(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;


	void do_mapping() override;
	void fix_return_by_ref();
};

}

#endif /* SRC_LIB_KABA_COMPILER_SERIALIZERX_H_ */
