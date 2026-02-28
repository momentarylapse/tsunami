/*
 * call.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include <type_traits>
#include "../syntax/Function.h"
#include "../syntax/Class.h"
#include "../CompilerConfiguration.h"



#if 0
#include "../../os/msg.h"
void db_out(const string &s) {
	msg_write(s);
}
#else
#define db_out(x)
#endif



namespace kaba {

#define CALL_DEBUG_X		0

// call-by-reference dummy
struct
CBR {
	int _dummy_[1024];
};

struct vec2 { float a; float b; };
struct vec3 { float a; float b; float c; };
struct vec4 { float a; float b; float c; float d; };

void call0_void(void *ff, void *ret, const Array<void*> &param) {
	((void(*)())ff)();
}

template<class R>
void call0(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,R>::value) {
		//msg_write("CBR return (1p)!!!");
		((void(*)(void*))ff)(ret);
	} else {
		*(R*)ret = ((R(*)())ff)();
	}
}

template<class A>
void call1_void_x(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,A>::value) {
		db_out("CBR -> void");
		((void(*)(void*))ff)(param[0]);
	} else {
		db_out("x -> void");
		((void(*)(A))ff)(*(A*)param[0]);
	}
}

template<class R, class A>
void call1(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,R>::value) {
		if constexpr (std::is_same<CBR,A>::value) {
			db_out("CBR -> CBR");
			((void(*)(void*, void*))ff)(ret, param[0]);
		} else {
			db_out("x -> CBR");
			((void(*)(void*, A))ff)(ret, *(A*)param[0]);
		}
	} else {
		if constexpr (std::is_same<CBR,A>::value) {
			db_out("CBR -> x");
			*(R*)ret = ((R(*)(void*))ff)(param[0]);
		} else {
			db_out("x -> x");
			*(R*)ret = ((R(*)(A))ff)(*(A*)param[0]);
		}
	}
}

template<class R, class A, class B>
void call2(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,R>::value) {
		if constexpr (std::is_same<CBR,A>::value and std::is_same<CBR,B>::value) {
			db_out("CBR CBR -> CBR");
			((void(*)(void*, void*, void*))ff)(ret, param[0], param[1]);
		} else if constexpr (std::is_same<CBR,A>::value) {
			db_out("CBR x -> CBR");
			((void(*)(void*, void*, B))ff)(ret, param[0], *(B*)param[1]);
		} else {
			db_out("x x -> CBR");
			((void(*)(void*, A, B))ff)(ret, *(A*)param[0], *(B*)param[1]);
		}

	} else {
		if constexpr (std::is_same<CBR,A>::value) {
			db_out("CBR x -> x");
			*(R*)ret = ((R(*)(void*, B))ff)(param[0], *(B*)param[1]);
		} else {
			db_out("x x -> x");
			*(R*)ret = ((R(*)(A, B))ff)(*(A*)param[0], *(B*)param[1]);
		}
	}
}

template<class R, class A, class B, class C>
void call3(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,R>::value) {
		((void(*)(void*, A, B, C))ff)(ret, *(A*)param[0], *(B*)param[1], *(C*)param[2]);
	} else {
		*(R*)ret = ((R(*)(A, B, C))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2]);
	}
}

template<class R, class A, class B, class C, class D>
void call4(void *ff, void *ret, const Array<void*> &param) {
	if constexpr (std::is_same<CBR,R>::value) {
		((void(*)(void*, A, B, C, D))ff)(ret, *(A*)param[0], *(B*)param[1], *(C*)param[2], *(D*)param[3]);
	} else {
		*(R*)ret = ((R(*)(A, B, C, D))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2], *(D*)param[3]);
	}
}

#ifdef CPU_ARM64
bool call_function_pointer_arm64(void *ff, void *ret, const Array<void*> &param, const Class *return_type, const Array<const Class*> &ptype) {
	const int N = 6; // #regs
	static int64 temp[N*4+2];
	memset(&temp, 0, sizeof(temp));
	// r0..5, d0..5, ret, f, r0:out, d0:out

	temp[N*2+1] = (int_p)ff;

	if (return_type->uses_return_by_memory()) {
		//msg_write("RET BY MEM");
		temp[N*2] = (int_p)ret;
	}

	int nrreg = 0;
	int nsreg = 0;
	for (int i=0; i<param.num; i++) {
		if (ptype[i] == common_types.i32 or ptype[i]->is_enum())
			temp[nrreg ++] = *(int*)param[i];
		else if (ptype[i] == common_types.i8 or ptype[i] == common_types.u8 or ptype[i] == common_types._bool)
			temp[nrreg ++] = *(int8*)param[i];
		else if (ptype[i] == common_types.i64 or ptype[i]->is_some_pointer())
			temp[nrreg ++] = *(int64*)param[i];
		else if (ptype[i] == common_types.f32)
			temp[N + nsreg ++] = *(int*)param[i]; // float
		else if (ptype[i] == common_types.f64)
			temp[N + nsreg ++] = *(int64*)param[i];
		else if (ptype[i]->uses_return_by_memory())
			temp[nrreg ++] = (int_p)param[i];
		else
			return false;
	}
	if (nrreg >= N or nsreg >= N)
		return false;

	//msg_write(format("call...  %d %d", nrreg, nsreg));

	int64* p = &temp[0];

	__asm__(
		"mov x20, %0\n"
		"ldr x0, [x20]\n" // -> r0
		"add x20, x20, 0x08\n"
		"ldr x1, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr x2, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr x3, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr x4, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr x5, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr d0, [x20]\n" // -> d0
		"add x20, x20, 0x08\n"
		"ldr d1, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr d2, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr d3, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr d4, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr d5, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr x8, [x20]\n" // -> r8
		"add x20, x20, 0x08\n"
		"ldr x7, [x20]\n" // -> fp
		"add x20, x20, 0x08\n"
		"blr x7\n"
		"str x0, [x20]\n" // -> r0:out
		"add x20, x20, 0x08\n"
		"str x1, [x20]\n"
		"add x20, x20, 0x08\n"
		"str x2, [x20]\n"
		"add x20, x20, 0x08\n"
		"str x3, [x20]\n"
		"add x20, x20, 0x08\n"
		"str x5, [x20]\n"
		"add x20, x20, 0x08\n"
		"str x6, [x20]\n"
		"add x20, x20, 0x08\n"
		"str d0, [x20]\n" // -> d0:out
		"add x20, x20, 0x08\n"
		"str d1, [x20]\n"
		"add x20, x20, 0x08\n"
		"str d2, [x20]\n"
		"add x20, x20, 0x08\n"
		"str d3, [x20]\n"
		"add x20, x20, 0x08\n"
		"str d4, [x20]\n"
		"add x20, x20, 0x08\n"
		"str d5, [x20]\n"
		 :
		 : "r"(p)
		 : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "d0", "d1", "d2", "d3", "d4", "d5");

	/*msg_write("ok");
	msg_write(temp[N*2+2]);
	msg_write(f2s(*(float*)&temp[N*2+3], 3));
	exit(1);*/
	//msg_write(bytes(&temp, sizeof(temp)).hex());

	if (return_type == common_types.i32 or return_type->is_enum()) {
		*(int*)ret = (int)temp[N*2+2];
	} else if (return_type == common_types.i64 or return_type->is_some_pointer()) {
		*(int64*)ret = temp[N*2+2];
	} else if (return_type == common_types.i8 or return_type == common_types.u8 or return_type == common_types._bool) {
		*(int8*)ret = (int8)temp[N*2+2];
	} else if (return_type == common_types.f64) {
		*(int64*)ret = temp[N*3+2];
	} else if (return_type->_return_in_float_registers() or return_type == common_types.f64) {
		//msg_error("ret in float");
		for (int i=0; i<return_type->size/4; i++)
			((int*)ret)[i] = (int)temp[N*3+2+i];
	}
	//msg_write(return_type->name + "  " + i2s(return_type->size));
	//msg_write("=>  " + bytes(ret, return_type->size).hex());
	return true;
}
#endif

// void*,int,int64,float,float64,char,bool,string,vector,complex

// BEFORE call-by-ref transformation!!!
bool call_function_pointer(void *ff, void *ret, const Array<void*> &param, const Class *return_type, const Array<const Class*> &ptype) {

	// TODO handle return in member functions on windows...
//	if ((config.abi == Abi::AMD64_WINDOWS) and !f->is_static() and f->name_space->uses_call_by_reference() and f->literal_return_type->uses_return_by_memory())
//		return false;

#ifdef CPU_ARM64
	return call_function_pointer_arm64(ff, ret, param, return_type, ptype);
#else

	auto is_gpr = [] (const Class* t) -> bool {
		if (t == common_types.i64 or t->is_pointer_raw())
			return true;
		if (t == common_types.i32 or t->is_enum())
			return true;
		if (t == common_types.i16 or t == common_types.u16)
			return true;
		if (t == common_types.i8 or t == common_types.u8 or t == common_types._bool)
			return true;
		return false;
	};

	if (ptype.num == 0) {
		if (return_type == common_types._void) {
			call0_void(ff, ret, param);
			return true;
		} else if (return_type == common_types.i32) {
			call0<int>(ff, ret, param);
			return true;
		} else if (return_type == common_types.f32) {
			call0<float>(ff, ret, param);
			return true;
		} else if (return_type->is_some_pointer()) {
			call0<void*>(ff, ret, param);
			return true;
		} else if (return_type->uses_return_by_memory()) {
			call0<CBR>(ff, ret, param);
			return true;
		}
	} else if (ptype.num == 1) {
		if (return_type == common_types._void) {
			if (is_gpr(ptype[0])) {
				call1_void_x<int64>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f32) {
				call1_void_x<float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
#if CALL_DEBUG_X
				void *ppp, *qqq;
				asm volatile ("movq %%rsp, %%rax;"
				              "movq %%rax, %0;"
							"movq %%rbp, %%rax;"
		  "movq %%rax, %1;"
				                  :  "=r" (ppp), "=r"(qqq) : : );
				printf("stack before  sp=%p  bp=%p\n", ppp, qqq);
#endif
				call1_void_x<CBR>(ff, ret, param);
#if CALL_DEBUG_X
				asm volatile ("movq %%rsp, %%rax;"
				              "movq %%rax, %0;"
							"movq %%rbp, %%rax;"
		  "movq %%rax, %1;"
				                  :  "=r" (ppp), "=r"(qqq) : : );
				printf("stack after  sp=%p  bp=%p\n", ppp, qqq);
#endif
				return true;
			}
		} else if (return_type == common_types.i32) {
			if (is_gpr(ptype[0])) {
				call1<int,int64>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f32) {
				call1<int,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<int,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.i64) {
			if (is_gpr(ptype[0])) {
				call1<int64,int64>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types._bool or return_type == common_types.i8) {
			if (is_gpr(ptype[0])) {
				call1<char,int64>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f32) {
				call1<char,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<char,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.f32) {
			if (is_gpr(ptype[0])) {
				call1<float,int64>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f32) {
				call1<float,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<float,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.f64) {
			if (ptype[0] == common_types.f32) {
				call1<double,float>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f64) {
				call1<double,double>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.vec3) {
			if (ptype[0]->uses_call_by_reference()) {
				call1<vec3,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.quaternion) {
			if (ptype[0]->uses_call_by_reference()) {
				call1<vec4,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type->uses_return_by_memory()) {
			if (is_gpr(ptype[0])) {
				call1<CBR,int64>(ff, ret, param);
				return true;
			} else if (ptype[0] == common_types.f32) {
				call1<CBR,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->is_pointer_raw()) {
				call1<CBR,void*>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<CBR,CBR>(ff, ret, param);
				return true;
			}
		}
	} else if (ptype.num == 2) {
		if (return_type == common_types.i32) {
			if (is_gpr(ptype[0]) and is_gpr(ptype[1])) {
				call2<int,int64,int64>(ff, ret, param);
				return true;
			}
			if (ptype[0]->uses_call_by_reference() and is_gpr(ptype[1])) {
				call2<int,CBR,int64>(ff, ret, param);
				return true;
			}
			if (ptype[0]->uses_call_by_reference() and (ptype[1] == common_types.f32)) {
				call2<int,CBR,float>(ff, ret, param);
				return true;
			}
			if (ptype[0]->uses_call_by_reference() and (ptype[1]->uses_call_by_reference())) {
				call2<int,CBR,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types._bool) {
			if (is_gpr(ptype[0]) and is_gpr(ptype[1])) {
				call2<bool,int64,int64>(ff, ret, param);
				return true;
			}
			if (ptype[0]->is_some_pointer() and ptype[1]->is_some_pointer()) {
				call2<bool,void*,void*>(ff, ret, param);
				return true;
			}
			/*if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == common_types.i32)) {
				call2<bool,CBR,int>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == common_types.f32)) {
				call2<bool,CBR,float>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1]->uses_call_by_reference())) {
				call2<bool,CBR,CBR>(ff, ret, param);
				return true;
			}*/
		} else if (return_type == common_types.f32) {
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32)) {
				call2<float,float,float>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == common_types.f32)) {
				call2<float,CBR,float>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.i64) {
			if (is_gpr(ptype[0]) and is_gpr(ptype[1])) {
				call2<int64,int64,int64>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.complex) {
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32)) {
				call2<vec2,float,float>(ff, ret, param);
				return true;
			}
		} else if (return_type == common_types.quaternion) {
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == common_types.f32)) {
				call2<vec4,CBR,float>(ff, ret, param);
				return true;
			}
		} else if (return_type->uses_return_by_memory()) {
			if (is_gpr(ptype[0]) and is_gpr(ptype[1])) {
				call2<CBR,int64,int64>(ff, ret, param);
				return true;
			} else if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32)) {
				call2<CBR,float,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference() and is_gpr(ptype[1])) {
				call2<CBR,CBR,int64>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference() and (ptype[1] == common_types.f32)) {
				call2<CBR,CBR,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference() and (ptype[1]->uses_call_by_reference())) {
				call2<CBR,CBR,CBR>(ff, ret, param);
				return true;
			}
		}
	} else if (ptype.num == 3) {
		if (return_type == common_types.vec3) {
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32) and (ptype[2] == common_types.f32)) {
				call3<vec3,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32) and (ptype[2] == common_types.f32)) {
				((void(*)(void*, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2]);
				return true;
			}
		}*/
	} else if (ptype.num == 4) {
		if (return_type == common_types._void) {
			if ((ptype[0] == common_types.vec3) and (ptype[1] == common_types.f32) and (ptype[2] == common_types.f32) and (ptype[3] == common_types.f32)) {
				((void(*)(void*, float, float, float))ff)(param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}
		if (return_type->_return_in_float_registers() and (return_type->size == 16)) { // rect, color, plane, quaternion
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32) and (ptype[2] == common_types.f32) and (ptype[3] == common_types.f32)) {
				call4<vec4,float,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == common_types.f32) and (ptype[1] == common_types.f32) and (ptype[2] == common_types.f32) and (ptype[3] == common_types.f32)) {
				((void(*)(void*, float, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}*/
	}
	db_out(".... NOPE");
	return false;
#endif
}

bool call_function(Function *f, void *ret, const Array<void*> &param) {

	// BEFORE call-by-ref transformation!!!
	//bool call_function(Function *f, void *ff, void *ret, const Array<void*> &param) {
	db_out("eval: " + f->signature());
	auto ptype = f->literal_param_type;

	// TODO handle return in member functions on windows...
	if ((config.target.abi == Abi::AMD64_WINDOWS) and !f->is_static() and f->name_space->uses_call_by_reference() and f->literal_return_type->uses_return_by_memory())
		return false;

	auto fp = f->address_preprocess;
	if (!fp)
		fp = (void*)(int_p)f->address;
	return call_function_pointer(fp, ret, param, f->literal_return_type, ptype);
}

void *object_get_virtual_func_pointer(void *ob, Function *f) {
	int virt_index = f->virtual_index;
	return (*(void***)ob)[virt_index];
}

void *object_get_member_func_pointer(void *ob, Function *f, bool allow_virtual) {
	if (allow_virtual and f->virtual_index >= 0)
		return object_get_virtual_func_pointer(ob, f);
	if (auto fp = f->address_preprocess)
		return fp;
	return (void*)(int_p)f->address;
}

bool call_member_function(Function *f, void *instance, void *ret, const Array<void*> &param, bool allow_virtual) {
	// BEFORE call-by-ref transformation!!!
	//bool call_function(Function *f, void *ff, void *ret, const Array<void*> &param) {
	db_out("eval: " + f->signature());
	auto ptype = f->literal_param_type;

	// TODO handle return in member functions on windows...
	if ((config.target.abi == Abi::AMD64_WINDOWS) and f->name_space->uses_call_by_reference() and f->literal_return_type->uses_return_by_memory())
		return false;

	auto fp = object_get_member_func_pointer(instance, f, allow_virtual);
	Array<void*> param_with_instance = param;
	param_with_instance.insert(instance, 0);
	return call_function_pointer(fp, ret, param_with_instance, f->literal_return_type, ptype);
}

void *callable_get_func_pointer(void *c) {
	return object_get_virtual_func_pointer(c, common_types.callable_base->get_call());
}

bool call_callable(void *c, void *ret, const Array<void*> &_param, const Class *return_type, const Array<const Class*> &_ptype) {
	auto ptype = _ptype;
	ptype.insert(common_types.callable_base, 0);
	auto param = _param;
	param.insert(c, 0);
	return call_function_pointer(callable_get_func_pointer(c), ret, param, return_type, ptype);
}



}


