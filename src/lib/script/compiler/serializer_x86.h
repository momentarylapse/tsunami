
#ifndef SERIALIZER_X86_H_
#define SERIALIZER_X86_H_

#include "serializer.h"

namespace Script
{

class SerializerX86 : public Serializer
{
public:
	SerializerX86(Script *script, Asm::InstructionWithParamsList *list) : Serializer(script, list){};
	virtual ~SerializerX86(){}
	virtual void add_function_call(Script *script, int func_no);
	virtual void add_virtual_function_call(int virtual_index);
	virtual int fc_begin();
	virtual void fc_end(int push_size);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual SerialCommandParam SerializeParameter(Command *link, Block *block, int index);
	virtual void SerializeCompilerFunction(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret, Block *block, int index, int marker_before_params);
	virtual void SerializeOperator(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret);

	virtual void DoMapping();
	void ProcessReferences();
	virtual void CorrectUnallowedParamCombis();
	virtual void CorrectUnallowedParamCombis2(SerialCommand &c);
};

};

#endif /* SERIALIZER_X86_H_ */
