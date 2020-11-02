
#ifndef SERIALIZER_X86_H_
#define SERIALIZER_X86_H_

#include "serializer.h"

namespace kaba
{

class SerializerX86 : public Serializer {
public:
	SerializerX86(Script *script, Asm::InstructionWithParamsList *list) : Serializer(script, list){};
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
	void process_references();
	virtual void correct_unallowed_param_combis();
	virtual void correct_unallowed_param_combis2(SerialNode &c);
};

};

#endif /* SERIALIZER_X86_H_ */
