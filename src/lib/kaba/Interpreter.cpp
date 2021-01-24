/*
 * Interpreter.cpp
 *
 *  Created on: Jan 22, 2021
 *      Author: michi
 */

#include "Interpreter.h"
#include "kaba.h"
#include "compiler/SerializerX.h"
#include "../file/msg.h"

namespace kaba {


bool call_function(Function *f, void *ff, void *ret, const Array<void*> &param);

Interpreter::Interpreter(Script *s) {
	script = s;
}

Interpreter::~Interpreter() {
}

void Interpreter::do_error(const string &s) {
	script->do_error_internal("interpreter: " + s);
}

void Interpreter::add_function(Function *f, SerializerX *ser) {
	//msg_write("INT: add func " + f->signature(TypeVoid));
	IFunction ff;
	ff.f = f;
	//ff.cmd = new CommandList;
	//*ff.cmd = ser->cmd;
	ff.ser = ser;
	functions.add(ff);
}

void Interpreter::run(const string &name) {
	for (auto &f: functions) {
		if (f.f->name == name) {
			run_function(f.f, f.ser);
		}
	}
}

void Interpreter::run_function(Function *f, SerializerX *ser) {
	msg_write("RUN..." + f->signature(TypeVoid));
	Frame frame;
	frame.stack.resize(f->_var_size + 64);
	frame.offset = f->_var_size;
	//msg_write(frame.offset);


	for (auto &c: ser->cmd.cmd)
		for (int k=0; k<3; k++)
			if (c.p[k].kind == NodeKind::VAR_TEMP) {
				int n = c.p[k].p;
				if (n >= frame.temps.num)
					frame.temps.resize(n + 1);
				if (frame.temps[n].num < c.p[k].type->size)
					frame.temps[n].resize(c.p[k].type->size);
			}

	for (int i=0; i<ser->cmd.cmd.num; i++) {
		auto &c = ser->cmd.cmd[i];
		i = run_command(i, c, ser, frame);
	}
}

int Interpreter::run_command(int index, SerialNode &n,SerializerX *ser, Frame &frame) {
	auto get_param = [&] (int i) {
		if (n.p[i].kind == NodeKind::NONE) {
			return (void*)nullptr;
		} else if (n.p[i].kind == NodeKind::IMMEDIATE) {
			return (void*)&n.p[i].p;
		} else if (n.p[i].kind == NodeKind::CONSTANT_BY_ADDRESS) {
			return (void*)(int_p)n.p[i].p;
		} else if (n.p[i].kind == NodeKind::CONSTANT) {
			return ((Constant*)n.p[i].p)->p();
		} else if (n.p[i].kind == NodeKind::VAR_LOCAL) {
			return (void*)&frame.stack[frame.offset + ((Variable*)(int_p)n.p[i].p)->_offset];
		} else if (n.p[i].kind == NodeKind::LOCAL_MEMORY) {
			return (void*)&frame.stack[frame.offset + n.p[i].p];
		} else if (n.p[i].kind == NodeKind::VAR_TEMP) {
			int nn = n.p[i].p;
			return (void*)&frame.temps[nn][0];
		} else {
			do_error("unhandled param " + n.p[i].str(ser));
		}
		return (void*)nullptr;
	};


	msg_write("    " + n.str(ser));
	if (n.inst == INST_MARKER) {
	} else if (n.inst == Asm::INST_PUSH) {
		call_params.add(get_param(0));
	} else if (n.inst == Asm::INST_RET) {
		return 10000000;
	} else if (n.inst == Asm::INST_LEA) {
		*(void**)get_param(0) = get_param(1);
	} else if (n.inst == Asm::INST_MOV) {
		memcpy(get_param(0), get_param(1), n.p[0].type->size);
		//msg_write(frame.stack.head(16).hex());
	} else if (n.inst == Asm::INST_MOVZX) {
		if (n.p[0].type->size < n.p[1].type->size)
			memcpy(get_param(0), get_param(1), n.p[0].type->size);
		else
			do_error("movzx...");
	} else if (n.inst == Asm::INST_ADD) {
		if (n.p[0].type == TypeInt)
			*(int*)get_param(0) = *(int*)get_param(1) + *(int*)get_param(2);
		else if (n.p[0].type == TypeInt64)
			*(int64*)get_param(0) = *(int64*)get_param(1) + *(int64*)get_param(2);
		else if (n.p[0].type == TypeChar)
			*(char*)get_param(0) = *(char*)get_param(1) + *(char*)get_param(2);
	} else if (n.inst == Asm::INST_SUB) {
		if (n.p[0].type == TypeInt)
			*(int*)get_param(0) = *(int*)get_param(1) - *(int*)get_param(2);
		else if (n.p[0].type == TypeInt64)
			*(int64*)get_param(0) = *(int64*)get_param(1) - *(int64*)get_param(2);
		else if (n.p[0].type == TypeChar)
			*(char*)get_param(0) = *(char*)get_param(1) - *(char*)get_param(2);
	} else if (n.inst == Asm::INST_IMUL) {
		if (n.p[0].type == TypeInt)
			*(int*)get_param(0) = *(int*)get_param(1) * *(int*)get_param(2);
		else if (n.p[0].type == TypeInt64)
			*(int64*)get_param(0) = *(int64*)get_param(1) * *(int64*)get_param(2);
		else if (n.p[0].type == TypeChar)
			*(char*)get_param(0) = *(char*)get_param(1) * *(char*)get_param(2);
	} else if (n.inst == Asm::INST_IDIV) {
		if (n.p[0].type == TypeInt)
			*(int*)get_param(0) = *(int*)get_param(1) / *(int*)get_param(2);
		else if (n.p[0].type == TypeInt64)
			*(int64*)get_param(0) = *(int64*)get_param(1) / *(int64*)get_param(2);
		else if (n.p[0].type == TypeChar)
			*(char*)get_param(0) = *(char*)get_param(1) / *(char*)get_param(2);
	} else if (n.inst == Asm::INST_MODULO) {
		if (n.p[0].type == TypeInt)
			*(int*)get_param(0) = *(int*)get_param(1) % *(int*)get_param(2);
		else if (n.p[0].type == TypeInt64)
			*(int64*)get_param(0) = *(int64*)get_param(1) % *(int64*)get_param(2);
		else if (n.p[0].type == TypeChar)
			*(char*)get_param(0) = *(char*)get_param(1) % *(char*)get_param(2);
	} else if (n.inst == Asm::INST_CALL) {
		auto *f = ((Function*)n.p[1].p);
		//msg_write("CALL " + f->signature(TypeVoid));
		if (f->address) {
			//msg_write("addr...");
			//char rrr[64];

			auto pt = f->literal_param_type;
			if (!f->is_static())
				pt.insert(f->name_space, 0);

			// argh, de-convert from call-by-reference
			for (int i=0; i<pt.num; i++)
				if (pt[i]->uses_call_by_reference())
					call_params[i] = *(void**)call_params[i];

			call_function(f, f->address, get_param(0), call_params);
		} else {
			do_error("call non-addr");
		}
		//auto ret = n.p[0];
		call_params.clear();
	} else {
		do_error("UNHANDLED COMMAND: " + n.str(ser));
	}
	return index;
}


}
