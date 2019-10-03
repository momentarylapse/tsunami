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
#include <stdio.h>
#include <assert.h>


namespace Kaba{


bool _verbose_exception_ = false;


KabaException::KabaException(const string &message) {
	text = message;
}

void KabaException::__init__(const string &message) {
	new(this) KabaException(message);
}

void KabaException::__delete__() {
	this->~KabaException();
}

string KabaException::message() {
	return text;
}





struct StackFrameInfo {
	void *rip;
	void *rsp;
	void *rbp;
	Script *s;
	Function *f;
	int64 offset;
};


extern Array<Script*> _public_scripts_;

inline void func_from_rip_test_script(StackFrameInfo &r, Script *s, void *rip, bool from_package)
{
	foreachi (Function *f, s->syntax->functions, i){
		if (from_package and !f->throws_exceptions)
			continue;
		void *frip = f->address;
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
	r.f = nullptr;
	r.offset = 1000000;

	// compiled functions
	for (Script* s: _public_scripts_){
		if ((rip < s->opcode) or (rip > &s->opcode[s->opcode_size]))
			continue;
		func_from_rip_test_script(r, s, rip, false);
	}
	if (r.f)
		return r;

	// externally linked...
	for (auto p: Packages){
		func_from_rip_test_script(r, p, rip, true);
	}
	return r;
}

struct ExceptionBlockData
{
	// needs_killing might be a reference to blocks... std::move()...
	// better keep blocks alive for a while
	Array<Block*> blocks;
	Array<Block*> needs_killing;
	Block *except_block;
	Node *except;
};

inline bool ex_type_match(const Class *ex_type, const Class *catch_type)
{
	if (ex_type == TypeUnknown)
		return true;
	if (catch_type == TypeVoid)
		return true;
	return ex_type->is_derived_from(catch_type);
}

ExceptionBlockData get_blocks(Script *s, Function *f, void* rip, const Class *ex_type)
{
	ExceptionBlockData ebd;
	ebd.except_block = nullptr;
	ebd.except = nullptr;

	if (f){
		// which blocks are we in?
		auto blocks = f->all_blocks();
		//printf("%d blocks\n", blocks.num);
		foreachb (Block *b, blocks){
			//printf("-block  %p    %p-%p\n", b, b->_start, b->_end);
			if ((b->_start <= rip) and (b->_end >= rip)){
//				printf("   inside\n");
				ebd.blocks.add(b);
			}
		}
	}


	// walk through the blocks from inside to outside
	Array<int> node_index;
	foreachi (Block *b, ebd.blocks, bi){
		ebd.needs_killing.add(b);

		if (!b->parent)
			continue;

		// are we in a try block?
		for (Node *n: b->parent->uparams){
			if ((n->kind == NodeKind::STATEMENT) and (n->as_statement()->id == StatementID::TRY)){
				if (n->uparams[0]->as_block() == b){
					if (_verbose_exception_)
						msg_write("found try block");
					auto ee = n->uparams[1];
					if (_verbose_exception_)
						msg_write(ee->type->name);
					if (!ex_type_match(ex_type, ee->type))
						continue;
					if (_verbose_exception_)
						msg_write("match");
					ebd.except = ee;
					ebd.except_block = n->uparams[2]->as_block();
					return ebd;
				}
			}
		}
	}
	return ebd;
}


void* rbp2 = nullptr;

#ifdef CPU_AMD64

void relink_return(void *rip, void *rbp, void *rsp)
{
#ifdef OS_LINUX
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
		: );

//	printf("rbp=%p\n", rbp2);
#endif

	exit(0);
}
#endif

const Class* _get_type(void *p, void *vtable, const Class *ns) {
	for (auto *c: ns->classes) {
		if (c->_vtable_location_compiler_)
			if ((c->_vtable_location_target_ == vtable) or (c->_vtable_location_external_ == vtable))
				return c;
		auto *r = _get_type(p, vtable, c);
		if (r)
			return r;

	}
	return nullptr;
}

const Class* get_type(void *p)
{
	if (!p)
		return TypeUnknown;
	void *vtable = *(void**)p;
	auto scripts = _public_scripts_;
	for (auto p: Packages)
		scripts.add(p);
	for (Script* s: scripts) {
		auto *r = _get_type(p, vtable, s->syntax->base_class);
		if (r)
			return r;
	}
	return TypeUnknown;
}

#ifdef CPU_AMD64

Array<StackFrameInfo> get_stack_trace(void **rbp)
{
	Array<StackFrameInfo> trace;

	void **rsp = nullptr;
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
				msg_write(">>  " + r.s->filename + " : " + r.f->long_name() + format("()  +%d", r.offset));

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
#ifdef OS_LINUX
	// get stack frame base pointer rbp
	void **rbp = nullptr;
	void **rsp = nullptr;
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

	const Class *ex_type = get_type(kaba_exception);

	for (auto r: trace){

		if (_verbose_exception_)
			msg_write(">>  " + r.s->filename + " : " + r.f->long_name() + format("()  +%d", r.offset));
		auto ebd = get_blocks(r.s, r.f, r.rip, ex_type);

		for (Block *b: ebd.needs_killing){
			if (_verbose_exception_)
				msg_write("  block " + p2s(b));
			for (Variable *v: b->vars){
				char *p = (char*)r.rbp + v->_offset;
				if (_verbose_exception_)
					msg_write("   " + v->type->name + " " + v->name + "  " + p2s(p));
				auto cf = v->type->get_destructor();
				if (cf){
					typedef void con_func(void *);
					con_func * f = (con_func*)cf->address;
					if (f){
						f(p);
					}
				}
			}
		}
		if (ebd.except_block){
			if (_verbose_exception_)
				msg_write("except_block block: " + p2s(ebd.except_block));

			if (ebd.except->uparams.num > 0){
				auto v = ebd.except_block->vars[0];
				void **p = (void**)((int_p)r.rbp + v->_offset);
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
		msg_write(">>  " + r.s->filename + " : " + r.f->long_name() + format("()  + 0x%x", r.offset));
#endif
	exit(1);
}
#pragma GCC pop_options

#else

void _cdecl kaba_raise_exception(KabaException *kaba_exception) {
	msg_error("exception handling not working on this architecture...");
	msg_write(kaba_exception->message());
	exit(1);
}

#endif

}

