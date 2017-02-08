
#ifndef SERIALIZER_X86_H_
#define SERIALIZER_X86_H_

#include "serializer.h"

namespace Kaba
{

class SerializerX86 : public Serializer
{
public:
	SerializerX86(Script *script, Asm::InstructionWithParamsList *list) : Serializer(script, list){};
	virtual ~SerializerX86(){}
	virtual void add_function_call(Script *script, int func_no, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_virtual_function_call(int virtual_index, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual int fc_begin(const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void fc_end(int push_size, const SerialNodeParam &ret);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual SerialNodeParam SerializeParameter(Node *link, Block *block, int index);
	virtual void SerializeStatement(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, Block *block, int index, int marker_before_params);
	virtual void SerializeInlineFunction(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);

	virtual void DoMapping();
	void ProcessReferences();
	virtual void CorrectUnallowedParamCombis();
	virtual void CorrectUnallowedParamCombis2(SerialNode &c);
};

};

#endif /* SERIALIZER_X86_H_ */
