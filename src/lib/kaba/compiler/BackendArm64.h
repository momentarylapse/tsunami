//
// Created by Michael Ankele on 2024-06-23.
//

#ifndef BACKENDARM64_H
#define BACKENDARM64_H

#include "BackendARM.h"

namespace kaba {

class BackendArm64 : public BackendARM {
public:
	explicit BackendArm64(Serializer* serializer);

	void process(Function *f, int index) override;
	void correct() override;
	void correct_implement_commands();
	void add_function_intro_frame(int stack_alloc_size);
	void add_function_intro_params(Function *f);
	void implement_return(const SerialNodeParam &p);

	SerialNodeParam param_vreg_auto(const Class *type, int vreg);
	void _immediate_to_register(int64 val, int size, int vreg);
	void _local_to_register(int offset, int size, int vreg);
	int _to_register(const SerialNodeParam &p, int force_vreg = -1);
	void _from_register(int reg, const SerialNodeParam &p);

	void _immediate_to_register_64(int64 val, int r);
	void _local_to_register_64(int offset, int r);
	void _global_to_register_64(int64 addr, int r);
	void _register_to_local_64(int r, int offset);
	void _register_to_global_64(int r, int64 addr);
	int _reference_to_register_64(const SerialNodeParam &p, int force_vreg = -1, const Class *type = nullptr);
	void _from_register_64(int reg, const SerialNodeParam &p);

	void _immediate_to_register_32(int val, int r);
	void _local_to_register_32(int offset, int r);
	void _global_to_register_32(int64 addr, int r);
	void _register_to_local_32(int r, int offset);
	void _register_to_global_32(int r, int64 addr);
	void _from_register_32(int reg, const SerialNodeParam &p);

	void _immediate_to_register_8(int val, int r);
	void _register_to_local_8(int r, int offset);
	void _register_to_global_8(int r, int64 addr);
	void _local_to_register_8(int offset, int r);
	void _global_to_register_8(int64 addr, int r);
	void _from_register_8(int reg, const SerialNodeParam &p);

	int _to_register_float(const SerialNodeParam &p, int force_vreg);
	void _from_register_float(int sreg, const SerialNodeParam &p);

	struct CallData {
		int64 push_size;
		Array<int> allocated_vregs;
	};

	CallData fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static);
	void fc_end(const CallData& d, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, bool is_static);

	void assemble() override;

	CommandList pre_cmd;
};

} // kaba

#endif //BACKENDARM64_H
