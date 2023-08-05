/*
 * BackendAmd64.h
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "BackendX86.h"

namespace kaba {


class BackendAmd64 : public BackendX86 {
public:
	BackendAmd64(Serializer *serializer);
	~BackendAmd64() override;

	void implement_return(const SerialNodeParam &p) override;
	void implement_mov_chunk(const SerialNodeParam &p1, const SerialNodeParam &p2, int size) override;

	int function_call_pre(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) override;
	void function_call_post(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, bool is_static) override;


	void add_function_outro(Function *f) override;
	void add_function_intro_params(Function *f) override;
	void add_function_intro_frame(int stack_alloc_size) override;

	void correct_return() {}


	Array<Asm::RegRoot> param_regs_root;
	int max_xmm_params;
};

}

