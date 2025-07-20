/*
 * exception.cpp
 *
 *  Created on: Jan 27, 2018
 *      Author: michi
 */

#include "exception.h"
#include "../kaba.h"
#include "../../os/msg.h"
#include "../../base/iter.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// RtlAddFunctionTable, RtlInstallFunctionTableCallback on Windows...?


namespace kaba {



bool _verbose_exception_ = false;

static void dbo(const string &s) {
	if (_verbose_exception_)
		msg_write(s);
}


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


KabaNoValueError::KabaNoValueError() : KabaException("no value") {
}

void KabaNoValueError::__init__() {
	new(this) KabaNoValueError();
}

KabaNullPointerError::KabaNullPointerError() : KabaException("null pointer") {
}



struct StackFrameInfo {
	void *rip = nullptr;
	void *rsp = nullptr;
	void *rbp = nullptr;
	shared<Module> s;
	Function *f = nullptr;
	int64 offset = 0;
	string str() const {
		if (!s or !f)
			return ">>  NOT IN KABA";
		return format(">>  %s()  +0x%x  (%s)", f->long_name(), offset, s->filename.str());
	}
};


inline void func_from_rip_test_module(StackFrameInfo &r, shared<Module> m, void *rip, bool from_package) {
	for (auto&& [i,f]: enumerate(m->tree->functions)) {
		if (from_package and !f->throws_exceptions())
			continue;
		int64 frip = f->address;
		if (frip >= (int64)rip)
			continue;
		int64 offset = (int64)rip - frip;
		if (offset >= r.offset)
			continue;
		if (from_package and offset >= 500)
			continue;
		r.s = m;
		r.f = f;
		r.offset = offset;
	}
}

StackFrameInfo get_func_from_rip(void *rip) {
	StackFrameInfo r;
	r.rip = rip;
	r.f = nullptr;
	r.offset = 1000000;

	// compiled functions
	for (auto s: default_context->public_modules) {
		if ((rip < s->opcode) or (rip > &s->opcode[s->opcode_size]))
			continue;
		func_from_rip_test_module(r, s, rip, false);
	}
	if (r.f)
		return r;

	// externally linked...
	for (auto p: default_context->internal_packages) {
		func_from_rip_test_module(r, p, rip, true);
	}
	return r;
}

struct ExceptionBlockData {
	// needs_killing might be a reference to blocks... std::move()...
	// better keep blocks alive for a while
	Array<Block*> blocks;
	Array<Block*> needs_killing;
	Block *except_block;
	Node *except;
};

inline bool ex_type_match(const Class *ex_type, const Class *catch_type) {
	if (ex_type == TypeUnknown)
		return true;
	if (catch_type == TypeVoid)
		return true;
	return ex_type->is_derived_from(catch_type);
}

ExceptionBlockData get_blocks(shared<Module> s, Function *f, void* rip, const Class *ex_type) {
	ExceptionBlockData ebd;
	ebd.except_block = nullptr;
	ebd.except = nullptr;

	if (f) {
		// which blocks are we in?
		auto blocks = f->all_blocks();
		//printf("%d blocks\n", blocks.num);
		foreachb (Block *b, blocks) {
			//printf("-block  %p    %p-%p\n", b, b->_start, b->_end);
			if ((b->_start <= rip) and (b->_end >= rip)) {
//				printf("   inside\n");
				ebd.blocks.add(b);
			}
		}
	}


	// walk through the blocks from inside to outside
	Array<int> node_index;
	for (auto&& [bi,b]: enumerate(ebd.blocks)) {
		ebd.needs_killing.add(b);

		if (!b->parent)
			continue;

		// are we in a try block?
		for (auto n: weak(b->parent->params)) {
			if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::Try)) {
				if (n->params[0]->as_block() == b) {
					dbo("found try block");
					for (int i=1; i<n->params.num; i+=2) {
						auto ee = n->params[i].get();
						dbo(ee->type->name);
						if (!ex_type_match(ex_type, ee->type))
							continue;
						dbo("match");
						ebd.except = ee;
						ebd.except_block = n->params[i+1]->as_block();
						return ebd;
					}
				}
			}
		}
	}
	return ebd;
}


const Class* _get_type(void *p, void *vtable, const Class *ns) {
	for (auto *c: weak(ns->classes)) {
		if (c->_vtable_location_compiler_)
			if ((c->_vtable_location_target_ == vtable) or (c->_vtable_location_external_ == vtable))
				return c;
		auto *r = _get_type(p, vtable, c);
		if (r)
			return r;

	}
	return nullptr;
}

const Class* get_type(void *p) {
	if (!p)
		return TypeUnknown;
	void *vtable = *(void**)p;
	auto modules = default_context->public_modules;
	for (auto p: default_context->internal_packages)
		modules.add(p);
	for (auto s: modules) {
		auto *r = _get_type(p, vtable, s->tree->base_class);
		if (r)
			return r;
	}
	return TypeUnknown;
}

void just_die(KabaException *kaba_exception, const Array<StackFrameInfo> &trace) {
	// uncaught...
	const Class *ex_type = get_type(kaba_exception);
	if (!kaba_exception)
		msg_error("uncaught exception  (nil)");
	else if (ex_type == TypeUnknown)
		msg_error("uncaught exception:  " + kaba_exception->message());
	else
		msg_error("uncaught " + get_type(kaba_exception)->name + ":  " + kaba_exception->message());

	// stack trace
	msg_write("stack trace:");
	for (const auto& r: trace)
		if (!r.f or r.f->name != "@die")
			msg_write(r.str());

	exit(1);
}

void clean_up_block(Block *b, const StackFrameInfo& r) {
	dbo("  block " + p2s(b));
	for (Variable *v: b->vars) {
		// for now, ignore temporary variables...
		if (v->name.head(1) == "-")
			continue;

		char *p = (char*)r.rbp + v->_offset;
		dbo("   " + v->type->name + " " + v->name + "  " + p2s(p));
		if (auto cf = v->type->get_destructor()) {
			dbo("call destr: " + cf->long_name());
			typedef void destructor_func(void *);
			if (auto* f = (destructor_func*)cf->address)
				f(p);
		}
	}
}

#ifdef CPU_AMD64

/*--------------------------------------------------------------*\
      AMD64
\*--------------------------------------------------------------*/

void relink_return(void *rip, void *rbp, void *rsp) {
	static void* rbp2 = nullptr;
#if defined(OS_LINUX) || defined(OS_MAC)
	dbo(format("relink to rip=%s, rbp=%s  rsp=%s\n", p2s(rip), p2s(rbp), p2s(rsp)));
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

Array<StackFrameInfo> get_stack_trace(const StackFrameInfo& frame) {
	Array<StackFrameInfo> trace;

	void **rbp = (void**)frame.rbp;

	void **rsp = nullptr;
//	msg_write("stack trace");
//	printf("rbp=%p     ...%p\n", rbp, &rsp);

	while (true) {
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
		if (r.f) {
			r.rsp = rsp;
			r.rbp = rbp;
			trace.add(r);
			dbo(r.str());

		} else {
			//dbo("unknown function...: " + p2s(rip));
			break;
		}
	}
	return trace;
}


// stack unwinding does not work if gcc does not use a stack frame...
KABA_LINK_GROUP_BEGIN

StackFrameInfo get_current_stack_frame() {
	void *rbp = nullptr;
	void *rsp = nullptr;

#if defined(OS_LINUX) || defined(OS_MAC)
	// get stack frame base pointer rbp
	asm volatile("movq %%rbp, %0\n\t"
				 "movq %%rsp, %1\n\t"
			: "=r" (rbp), "=r" (rsp)
			:
			: );

//	printf("rbp=%p   rsp=%p    local=%p\n", rbp, rsp, &rsp);

	// check sanity
	void **local = (void**)&rsp;
	// rbp  >  local > rsp
	assert((rbp > rsp) and (rbp > local) and (local > rsp));
	assert((int_p)rbp - (int_p)rsp < 10000);

#else

	// TODO

#endif

	return {nullptr, rsp, rbp};
}

void _cdecl kaba_raise_exception(KabaException *kaba_exception) {
#if defined(OS_LINUX) || defined(OS_MAC)
	auto frame = get_current_stack_frame();

	void *return_rsp = nullptr;
	void *return_rip = nullptr;


	{
		auto trace = get_stack_trace(frame);

		const Class *ex_type = get_type(kaba_exception);

		for (auto r: trace) {

			dbo(r.str());
			auto ebd = get_blocks(r.s, r.f, r.rip, ex_type);

			for (Block *b: ebd.needs_killing)
				clean_up_block(b, r);

			if (ebd.except_block) {
				dbo("except_block block: " + p2s(ebd.except_block));

				if (ebd.except->params.num > 0) {
					auto v = ebd.except_block->vars[0];
					void **p = (void**)((int_p)r.rbp + v->_offset);
					*p = kaba_exception;
				}

				// TODO special return
				return_rsp = (void*)((int_p)r.rsp - 16);
				return_rip = ebd.except_block->_start;
				break;
			}
		}

		// uncaught?
		if (!return_rsp)
			just_die(kaba_exception, trace);
	}
	relink_return(return_rip, frame.rbp, return_rsp);

#else
	just_die(kaba_exception, {});
#endif
}

KABA_LINK_GROUP_END


#elif defined(CPU_ARM64)

/*--------------------------------------------------------------*\
	  ARM64
\*--------------------------------------------------------------*/

StackFrameInfo get_current_stack_frame() {
	StackFrameInfo frame;
	int local;
	void *local_address = (void*)&local; // an address on the current stack

#if defined(OS_LINUX) || defined(OS_MAC)
	asm volatile("mov %0, x29\n" // base/frame pointer
			"mov %1, sp\n" // stack pointer
			: "=r" (frame.rbp), "=r" (frame.rsp)
			: : );
	//printf("rsp=%p    rbp=%p   local=%p\n", frame.rsp, frame.rbp, local_address);

	// check sanity
	// rbp  >  local  >  rsp
	assert((frame.rbp > frame.rsp) and (frame.rbp >= local_address) and (local_address >= frame.rsp));
	assert((int_p)frame.rbp - (int_p)frame.rsp < 10000);

#else

	msg_error("exception - reading stack frame not implemented for this architecture");
	exit(1);

#endif

	return frame;
}

Array<StackFrameInfo> get_stack_trace(const StackFrameInfo& frame) {
	Array<StackFrameInfo> trace;

	StackFrameInfo cur = frame;

	//msg_write("stack trace");
	//printf("rbp=%p     sp=%p\n", cur.rbp, &cur.rsp);

	while (true) {
		//printf("unroll   rbp=%p   ...%p\n", cur.rbp, &cur.rsp);
		// next (previous/caller) stack frame
		StackFrameInfo next;
		next.rsp = (void*)((int_p)cur.rbp + 16);
		next.rbp = ((void**)cur.rbp)[0]; //(void*)((int_p)cur.rbp - 16);
		next.rip = ((void**)cur.rbp)[1];

		cur = next;

		//printf("unwind  =>   rip=%p   rsp=%p   rbp=%p\n", cur.rip, cur.rsp, cur.rbp);
		auto r = get_func_from_rip(cur.rip);
		if (r.f) {
			r.rsp = cur.rsp;
			r.rbp = cur.rbp;
			trace.add(r);
			dbo(r.str());

		} else {
			//dbo("unknown function...: " + p2s(rip));
			break;
		}
	}
	return trace;
}

void relink_return(const StackFrameInfo& frame) {
#if defined(OS_LINUX) || defined(OS_MAC)
	dbo(format("relink to rip=%s  rsp=%s   rbp=%s\n", p2s(frame.rip), p2s(frame.rsp), p2s(frame.rbp)));
	asm volatile(
			"mov x29,  %0\n" // base/frame pointer
			"mov sp,  %1\n" // stack pointer
			"mov x30,  %2\n" // rip
			"ret"
		: : "r" (frame.rbp), "r" (frame.rsp), "r" (frame.rip) : );
#endif
	msg_error("catching exceptions not implemented on this architecture");
	exit(1);
}

void kaba_raise_exception(KabaException *kaba_exception) {
#if defined(OS_LINUX) || defined(OS_MAC)
	auto current_frame = get_current_stack_frame();

	StackFrameInfo return_frame;

	auto trace = get_stack_trace(current_frame);

	const Class *ex_type = get_type(kaba_exception);

	for (const auto& f: trace) {

		dbo(f.str());
		auto ebd = get_blocks(f.s, f.f, f.rip, ex_type);

	// FIXME
	//	for (Block *b: ebd.needs_killing)
	//		clean_up_block(b, r);

		if (ebd.except_block) {
			dbo("except_block block: " + p2s(ebd.except_block));

			if (ebd.except->params.num > 0) {
				auto v = ebd.except_block->vars[0];
				void **p = (void**)((int_p)f.rsp + v->_offset);
				*p = kaba_exception;
			}

			// TODO special return
			return_frame = f;
			return_frame.rip = ebd.except_block->_start;
			break;
		}
	}

	// caught?
	if (return_frame.rip)
		relink_return(return_frame);

	just_die(kaba_exception, trace);

#else
	just_die(kaba_exception, {});
#endif
}

#else

/*--------------------------------------------------------------*\
      OTHER CPUS
\*--------------------------------------------------------------*/

StackFrameInfo get_current_stack_frame() {
	return {};
}

Array<StackFrameInfo> get_stack_trace(const StackFrameInfo& frame) {
	return {};
}

void _cdecl kaba_raise_exception(KabaException *kaba_exception) {
	msg_error("exception handling not working on this architecture...");
	msg_write(kaba_exception->message());
	exit(1);
}

#endif




void kaba_die(KabaException* e) {
	auto frame = get_current_stack_frame();
	auto trace = get_stack_trace(frame);
	just_die(e, trace);
}

void kaba_assert(bool b) {
	if (!b) {
		auto e = new KabaException("assert failed");
		kaba_die(e);
	}
}

}

