
#ifndef SERIALIZER_AMD64_H_
#define SERIALIZER_AMD64_H_

#include "serializer_x86.h"

namespace Script
{


class SerializerAMD64 : public SerializerX86
{
public:
	SerializerAMD64(Script *script, Asm::InstructionWithParamsList *list) : SerializerX86(script, list){};
	virtual ~SerializerAMD64(){}
	virtual void add_function_call(Script *script, int func_no);
	virtual void add_virtual_function_call(int virtual_index);
	virtual int fc_begin();
	virtual void fc_end(int push_size);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual void CorrectUnallowedParamCombis2(SerialCommand &c);
};

};

#endif /* SERIALIZER_AMD64_H_ */
