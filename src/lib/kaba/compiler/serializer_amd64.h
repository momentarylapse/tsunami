
#ifndef SERIALIZER_AMD64_H_
#define SERIALIZER_AMD64_H_

#include "serializer_x86.h"

namespace Kaba
{


class SerializerAMD64 : public SerializerX86 {
public:
	SerializerAMD64(Script *script, Asm::InstructionWithParamsList *list) : SerializerX86(script, list){};
	virtual ~SerializerAMD64(){}
	void add_function_call(Function *f, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) override;
	void add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	int fc_begin(Function *f, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) override;
	void fc_end(int push_size, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) /*override*/;
	void add_function_intro_params(Function *f) override;
	void add_function_outro(Function *f) override;
	void correct_unallowed_param_combis2(SerialNode &c) override;
};

};

#endif /* SERIALIZER_AMD64_H_ */
