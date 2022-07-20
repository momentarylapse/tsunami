/*
 * ErrorHandler.cpp
 *
 *  Created on: 19.08.2020
 *      Author: michi
 */


#include "ErrorHandler.h"
#include "../lib/base/base.h"
#include "../lib/os/msg.h"
#include "../lib/kaba/kaba.h"
#include "../lib/hui/error.h"

#if HAS_LIB_UNWIND
	#define UNW_LOCAL_ONLY
	#include <libunwind.h>
#endif

namespace hui {
	void show_crash_window();
}


void ErrorHandler::init() {
	hui::SetErrorFunction(error_handler);
}

namespace kaba {
	//class StackFrameInfo;
	struct StackFrameInfo {
		void *rip;
		void *rsp;
		void *rbp;
		shared<Module> m;
		Function *f;
		int64 offset;
		string str() const;
	};
	StackFrameInfo get_func_from_rip(void *rip);

	string _cdecl var_repr(const void *p, const Class *type);
}

// _ZN13PluginManager15handle_draw_preEv
string unmangle(const string &name) {
	if (name.head(2) != "_Z")
		return name;
	string r;
	int i = 2;

	while (i < name.num) {

		if (name[i] == 'E')
			break;
		if (name[i] == 'R')
			break;

		bool is_namespace = false;
		if (name[i] == 'N') {
			is_namespace = true;
			i ++;
			if (name[i] == 'K') {
				i ++;
			}
		}

		int n = 0;
		if (name[i] >= '0' and name[i] <= '9') {
			n += n*10 + name[i] - '0';
			i ++;
		} else {
			r += ">" + name.sub(i);
			break;
		}
		if (name[i] >= '0' and name[i] <= '9') {
			n = n*10 + name[i] - '0';
			i ++;
		}

		if (r.num > 0)
			r += ".";
		r += name.sub(i, i + n);
		i += n;
		//if (is_namespace)
		//	r += "::";


	}

	if (r.num > 0)
		return r;
	return name;
}

void show_kaba_local_vars(const kaba::Function *f, void *bp) {
	msg_write(f->long_name());
	for (auto &v: f->var) {
		auto pp = (char*)bp + v->_offset;
		//msg_write(format("   %-30s                  @ %s  %d", (v->type->cname(f->owner()->base_class) + " " + v->name), p2s(pp), v->_offset));
		string s = format("  %20s %-14s  = ", v->type->cname(f->owner()->base_class), v->name);
		if (v->type->is_pointer()) {
			s += p2s(*(void**)pp);
//			if (*(void**)pp)
//				s += "   ->  " + kaba::var_repr(pp, v->type);
		} else if (v->type->can_memcpy()) {
			s += kaba::var_repr(pp, v->type);
		} else if (v->type->can_memcpy()) {
			s += "...";
		}
		msg_write(s);
	}

}



#if HAS_LIB_UNWIND
void ErrorHandler::show_backtrace() {

	unw_cursor_t cursor;
	unw_context_t uc;
	unw_word_t ip, sp, bp, offp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	kaba::Function *first_kaba = nullptr;

	Array<string> lines;

	void* first_kaba_bp = nullptr;
	while (unw_step(&cursor) > 0) {
		/*if (ip < 0x1000) {
			msg_write(" -> CLOSE-TO-NULL");
			continue;
		}*/
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		unw_get_reg(&cursor, UNW_X86_64_RBP, &bp);
	//	printf("frame      ----   ip = %lx, sp = %lx, bp = %lx\n", (long) ip, (long) sp, (long) bp);
		char _name[256];
		int n = unw_get_proc_name(&cursor, _name, sizeof(_name), &offp);
		if (n == 0) {
			string name = unmangle(_name);
			if (name.match("*signal_handler*") or name == "killpg")
				continue;
			lines.add(">>  " + name + "()");
			if (name == "main")
				break;
		} else {
			auto r = kaba::get_func_from_rip((void*)(int_p)ip);
			lines.add(r.str());
			if (!first_kaba) {
				first_kaba_bp = (void*)(int_p)bp;
				first_kaba = r.f;
			}
		}
	}

	foreachb(auto &l, lines)
		msg_write(l);

	if (first_kaba) {
		msg_write("\n\n------  local variables  ------");
		show_kaba_local_vars(first_kaba, first_kaba_bp);
	}
}
#else
void ErrorHandler::show_backtrace() {}
#endif

void ErrorHandler::error_handler() {

	msg_reset_shift();
	msg_write("");
	msg_write("================================================================================");
	msg_write("program has crashed, error handler has been called... maybe SegFault... m(-_-)m");
	msg_write("      trace:");


	show_backtrace();

	hui::show_crash_window();
	exit(1);
}

