
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
	void add_function_call(Function *f, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_virtual_function_call(int virtual_index, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	int fc_begin(const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void fc_end(int push_size, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void AddFunctionIntro(Function *f) override;
	void AddFunctionOutro(Function *f) override;
	SerialNodeParam SerializeParameter(Node *link, Block *block, int index) override;
	void SerializeStatement(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, Block *block, int index) override;
	void SerializeInlineFunction(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;

	void DoMapping() override;
	void ProcessReferences();
	virtual void CorrectUnallowedParamCombis();
	virtual void CorrectUnallowedParamCombis2(SerialNode &c);
};

};

#endif /* SERIALIZER_X86_H_ */
