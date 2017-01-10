
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
	virtual void add_function_call(Script *script, int func_no, const SerialCommandParam &instance, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual void add_virtual_function_call(int virtual_index, const SerialCommandParam &instance, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual int fc_begin(const SerialCommandParam &instance, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual void fc_end(int push_size, const SerialCommandParam &ret);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual SerialCommandParam SerializeParameter(Command *link, Block *block, int index);
	virtual void SerializeStatement(Command *com, const Array<SerialCommandParam> &params, const SerialCommandParam &ret, Block *block, int index, int marker_before_params);
	virtual void SerializeInlineFunction(Command *com, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual void SerializeOperator(Command *com, const Array<SerialCommandParam> &param, const SerialCommandParam &ret);

	virtual void DoMapping();
	void ProcessReferences();
	virtual void CorrectUnallowedParamCombis();
	virtual void CorrectUnallowedParamCombis2(SerialCommand &c);
};

};

#endif /* SERIALIZER_X86_H_ */
