
#ifndef SERIALIZER_ARM_H_
#define SERIALIZER_ARM_H_

#include "serializer.h"

namespace Kaba
{


class SerializerARM : public Serializer
{
public:
	SerializerARM(Script *script, Asm::InstructionWithParamsList *list) : Serializer(script, list){};
	virtual ~SerializerARM(){}
	virtual void add_function_call(Script *script, int func_no, const SerialCommandParam &instance, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual void add_virtual_function_call(int virtual_index, const SerialCommandParam &instance, const Array<SerialCommandParam> &params, const SerialCommandParam &ret);
	virtual int fc_begin(const SerialCommandParam &instance, const Array<SerialCommandParam> &param, const SerialCommandParam &ret);
	virtual void fc_end(int push_size, const SerialCommandParam &ret);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual SerialCommandParam SerializeParameter(Command *link, Block *block, int index);
	virtual void SerializeStatement(Command *com, const Array<SerialCommandParam> &param, const SerialCommandParam &ret, Block *block, int index, int marker_before_params);
	virtual void SerializeInlineFunction(Command *com, const Array<SerialCommandParam> &param, const SerialCommandParam &ret);
	virtual void SerializeOperator(Command *com, const Array<SerialCommandParam> &param, const SerialCommandParam &ret);

	virtual void DoMapping();
	void ConvertGlobalLookups();
	void CorrectUnallowedParamCombis();
	void ConvertMemMovsToLdrStr(SerialCommand &c);
	void ConvertGlobalRefs();
	void ProcessReferences();
	void ProcessDereferences();
	virtual void CorrectReturn();

	void transfer_by_reg_in(SerialCommand &c, int &i, int pno);
	void transfer_by_reg_out(SerialCommand &c, int &i, int pno);
	void gr_transfer_by_reg_in(SerialCommand &c, int &i, int pno);
	void gr_transfer_by_reg_out(SerialCommand &c, int &i, int pno);
};

};

#endif /* SERIALIZER_ARM_H_ */
