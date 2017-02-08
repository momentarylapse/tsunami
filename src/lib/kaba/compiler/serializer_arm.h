
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
	virtual void add_function_call(Script *script, int func_no, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_virtual_function_call(int virtual_index, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual int fc_begin(const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret);
	virtual void fc_end(int push_size, const SerialNodeParam &ret);
	virtual void AddFunctionIntro(Function *f);
	virtual void AddFunctionOutro(Function *f);
	virtual SerialNodeParam SerializeParameter(Node *link, Block *block, int index);
	virtual void SerializeStatement(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret, Block *block, int index, int marker_before_params);
	virtual void SerializeInlineFunction(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret);

	virtual void DoMapping();
	void ConvertGlobalLookups();
	void CorrectUnallowedParamCombis();
	void ConvertMemMovsToLdrStr(SerialNode &c);
	void ConvertGlobalRefs();
	void ProcessReferences();
	void ProcessDereferences();
	virtual void CorrectReturn();

	void transfer_by_reg_in(SerialNode &c, int &i, int pno);
	void transfer_by_reg_out(SerialNode &c, int &i, int pno);
	void gr_transfer_by_reg_in(SerialNode &c, int &i, int pno);
	void gr_transfer_by_reg_out(SerialNode &c, int &i, int pno);
};

};

#endif /* SERIALIZER_ARM_H_ */
