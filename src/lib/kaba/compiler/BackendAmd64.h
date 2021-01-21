/*
 * BackendAmd64.h
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "serializer.h"
#include "BackendX86.h"

namespace kaba {


class BackendAmd64 : public BackendX86 {
public:
	BackendAmd64(Serializer *serializer);
	~BackendAmd64() override;

	void process_references() override;

	void implement_return(kaba::SerialNode &c, int i) override;
	void implement_mov_chunk(kaba::SerialNode &c, int i, int size) override;

	int fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) override;
	void fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;
	void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) override;


	void add_function_outro(Function *f) override;
	void add_function_intro_params(Function *f) override;
	void add_function_intro_frame(int stack_alloc_size) override;

	void correct_return() {}


	Array<int> param_regs_root;
	int max_xmm_params;
};

}

