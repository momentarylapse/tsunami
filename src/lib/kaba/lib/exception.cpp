/*
 * exception.cpp
 *
 *  Created on: Jan 27, 2018
 *      Author: michi
 */

#include "exception.h"
#include "../kaba.h"
#include "../../file/msg.h"
#include <stdlib.h>
#include <assert.h>


namespace Kaba{


bool _verbose_exception_ = false;


KabaException::KabaException(const string &message)
{
	text = message;
}

void KabaException::__init__(const string &message)
{
	new(this) KabaException(message);
}

void KabaException::__delete__()
{
	this->~KabaException();
}

string KabaException::message()
{
	return text;
}





struct StackFrameInfo
{
	void *rip;
	void *rsp;
	void *rbp;
	Script *s;
	Function *f;
	int64 offset;
};


extern Array<Script*> PublicScript;

inline void func_from_rip_test_script(StackFrameInfo &r, Script *s, void *rip, bool from_package)
{
	foreachi (Function *f, s->syntax->functions, i){
		if (from_package and !f->throws_exceptions)
			continue;
		void *frip = (void*)s->func[i];
		if (frip >= rip)
			continue;
		int_p offset = (int_p)rip - (int_p)frip;
		if (offset >= r.offset)
			continue;
		if (from_package and offset >= 500)
			continue;
		r.s = s;
		r.f = f;
		r.offset = offset;
	}
}

StackFrameInfo get_func_from_rip(void *rip)
{
	StackFrameInfo r;
	r.rip = rip;
	r.f = NULL;
	r.offset = 1000000;

	// compiled functions
	for (Script* s: PublicScript){
		if ((rip < s->opcode) or (rip > &s->opcode[s->opcode_size]))
			continue;
		func_from_rip_test_script(r, s, rip, false);
	}
	if (r.f)
		return r;

	// externally linked...
	for (auto p: Packages){
		func_from_rip_test_script(r, p.script, rip, true);
	}
	return r;
}

struct ExceptionBlockData
{
	Array<Block*> needs_killing;
	Block *except_block;
	Node *except;
};

inline bool ex_type_match(Class *ex_type, Class *catch_type)
{
	if (ex_type == TypeUnknown)
		return true;
	if (catch_type == TypeVoid)
		return true;
	return ex_type->is_derived_from(catch_type);
}

ExceptionBlockData get_blocks(Script *s, Function *f, void* rip, Class *ex_type)
{
	ExceptionBlockData ebd;
	ebd.except_block = NULL;
	ebd.except = NULL;

	Array<Block*> blocks;
	foreachb (Block *b, s->syntax->blocks)
		if ((b->_start <= rip) and (b->_end >= rip))
			blocks.add(b);
	ebd.needs_killing = blocks;

	Array<int> node_index;
	foreachi (Block *b, blocks, bi){
		if (bi == 0)
			continue;
		int index = -1;
		foreachi (Node *n, b->nodes, ni){
			if (n->kind == KIND_BLOCK and n->link_no == blocks[bi-1]->index){
				node_index.add(ni);
				index = ni;
			}
		}
		if (index < 0){
			msg_error("block link error...");
			return ebd;
		}
		if (index > 0)
			if ((b->nodes[index - 1]->kind == KIND_STATEMENT) and (b->nodes[index - 1]->link_no == STATEMENT_TRY)){
				auto ee = b->nodes[index + 1];
				if (!ex_type_match(ex_type, ee->type))
					continue;
				//msg_write("try...");
				ebd.needs_killing = blocks.sub(0, bi);
				//msg_write(b->nodes[index + 2]->link_no);
				ebd.except = ee;
				ebd.except_block = s->syntax->blocks[b->nodes[index + 2]->link_no];
			}
	}
	//msg_write(ia2s(node_index));
	return ebd;
}


void* rbp2 = NULL;

void relink_return(void *rip, void *rbp, void *rsp)
{
	if (_verbose_exception_)
		printf("relink to rip=%p, rbp=%p  rsp=%p\n", rip, rbp, rsp);
	// ARGH....
	asm volatile(	"mov %1, %%rsp\n\t"
			"pop %%rbp\n\t" // pop rbp
			"pop %%rax\n\t" // pop rip
			"push %2\n\t" // push rip
			"ret"
		: "=r" (rbp2)
		: "r" (rsp), "r" (rip)
		: "%rsp");

//	printf("rbp=%p\n", rbp2);

	exit(0);
}

Class* get_type(void *p)
{
	if (!p)
		return TypeUnknown;
	void *vtable = *(void**)p;
	Array<Script*> scripts = PublicScript;
	for (auto p: Packages)
		scripts.add(p.script);
	for (Script* s: scripts)
		for (Class *c: s->syntax->classes)
			if (c->_vtable_location_compiler_)
				if ((c->_vtable_location_target_ == vtable) or (c->_vtable_location_external_ == vtable))
					return c;
	return TypeUnknown;
}


Array<StackFrameInfo> get_stack_trace(void **rbp)
{
	Array<StackFrameInfo> trace;

	void **rsp = NULL;
//	msg_write("stack trace");
//	printf("rbp=%p     ...%p\n", rbp, &rsp);

	while (true){
		rsp = rbp;
		rbp = (void**)*rsp;
		rsp ++;
		//printf("-- rsp: %p\n", rsp);
		//printf("-- rbp: %p\n", rbp);
		void *rip = *rsp;
		//printf("-- rip: %p\n", rip);
		rsp ++;
//		printf("unwind  =>   rip=%p   rsp=%p   rbp=%p\n", rip, rsp, rbp);
		auto r = get_func_from_rip(rip);
		if (r.f){
			r.rsp = rsp;
			r.rbp = rbp;
			trace.add(r);
			if (_verbose_exception_)
				msg_write(">>  " + r.s->filename + " : " + r.f->name + format("()  +%d", r.offset));

		}else{
			//if (_verbose_exception_)
			//	msg_write("unknown function...: " + p2s(rip));
			break;
		}
	}
	return trace;
}


// stack unwinding does not work if gcc does not use a stack frame...
#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

void _cdecl kaba_raise_exception(KabaException *kaba_exception)
{
	// get stack frame base pointer rbp
	void **rbp = NULL;
	void **rsp = NULL;
	asm volatile("movq %%rbp, %0\n\t"
			"movq %%rsp, %1\n\t"
		: "=r" (rbp), "=r" (rsp)
		:
		: );

	if (_verbose_exception_)
		msg_error("raise...");

//	printf("rbp=%p   rsp=%p    local=%p\n", rbp, rsp, &rsp);

	// check sanity
	void **local = (void**)&rsp;
	// rbp  >  local > rsp
	assert((rbp > rsp) and (rbp > local) and (local > rsp));
	assert((int_p)rbp - (int_p)rsp < 10000);


	auto trace = get_stack_trace(rbp);

	Class *ex_type = get_type(kaba_exception);

	for (auto r: trace){

		if (_verbose_exception_)
			msg_write(">>  " + r.s->filename + " : " + r.f->name + format("()  +%d", r.offset));
		auto ebd = get_blocks(r.s, r.f, r.rip, ex_type);

		for (Block *b: ebd.needs_killing){
			if (_verbose_exception_)
				msg_write("  block " + i2s(b->index));
			for (int i: b->vars){
				auto v = r.f->var[i];
				char *p = (char*)r.rbp + v._offset;
				if (_verbose_exception_)
					msg_write("   " + v.type->name + " " + v.name + "  " + p2s(p));
				auto cf = v.type->get_destructor();
				if (cf){
					typedef void con_func(void *);
					con_func * f = (con_func*)cf->script->func[cf->nr];
					if (f){
						f(p);
					}
				}
			}
		}
		if (ebd.except_block){
			if (_verbose_exception_)
				msg_write("except_block block: " + i2s(ebd.except_block->index));

			if (ebd.except->params.num > 0){
				auto v = r.f->var[ebd.except_block->vars[0]];
				void **p = (void**)((int_p)r.rbp + v._offset);
				*p = kaba_exception;
			}

			// TODO special return
			relink_return(ebd.except_block->_start, rbp, (void*)((int_p)r.rsp - 16));
			return;
		}
	}


	// uncaught...
	if (!kaba_exception)
		msg_error("uncaught exception  (nil)");
	else if (ex_type == TypeUnknown)
		msg_error("uncaught exception:  " + kaba_exception->message());
	else
		msg_error("uncaught " + get_type(kaba_exception)->name + ":  " + kaba_exception->message());
	for (auto r: trace)
		msg_write(">>  " + r.s->filename + " : " + r.f->name + format("()  + 0x%x", r.offset));
	exit(1);
}
#pragma GCC pop_options

}

