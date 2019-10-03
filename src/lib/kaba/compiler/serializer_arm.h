
#ifndef SERIALIZER_ARM_H_
#define SERIALIZER_ARM_H_

#include "serializer.h"

namespace Kaba
{


class SerializerARM : public Serializer {
public:
	SerializerARM(Script *script, Asm::InstructionWithParamsList *list) : Serializer(script, list){};
	virtual ~SerializerARM(){}
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) override;
	int fc_begin(Function *f, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) override;
	void fc_end(int push_size, const SerialNodeParam &ret) override;
	void add_function_intro_params(Function *f) override;
	void add_function_intro_frame(int stack_alloc_size) override;
	void add_function_outro(Function *f) override;
	SerialNodeParam serialize_parameter(Node *link, Block *block, int index) override;
	void serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index) override;
	void serialize_inline_function(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) override;

	void do_mapping() override;
	void convert_global_lookups();
	void correct_unallowed_param_combis();
	void convert_mem_movs_to_ldr_str(SerialNode &c);
	void convert_global_refs();
	void process_references();
	void process_dereferences();
	void correct_return() override;

	void transfer_by_reg_in(SerialNode &c, int &i, int pno);
	void transfer_by_reg_out(SerialNode &c, int &i, int pno);
	void gr_transfer_by_reg_in(SerialNode &c, int &i, int pno);
	void gr_transfer_by_reg_out(SerialNode &c, int &i, int pno);
	void split_mov_reg_immediate(SerialNode &c, int &i);
};

};

#endif /* SERIALIZER_ARM_H_ */
