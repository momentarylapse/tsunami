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

extern const Class *TypeCallableBase;

#define CALL_DEBUG_X		0

// call-by-reference dummy
class CBR {
	int _dummy_[1024];
};

class vec2 { float a; float b; };
class vec3 { float a; float b; float c; };
class vec4 { float a; float b; float c; float d; };

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
	// r0..5, s0..5, ret, f, r0:out, s0:out

	temp[N*2+1] = (int_p)ff;

	if (return_type->uses_return_by_memory()) {
		//msg_write("RET BY MEM");
		temp[N*2] = (int_p)ret;
	}

	int nrreg = 0;
	int nsreg = 0;
	for (int i=0; i<param.num; i++) {
		if (ptype[i] == TypeInt32 or ptype[i]->is_enum())
			temp[nrreg ++] = *(int*)param[i];
		else if (ptype[i] == TypeInt8 or ptype[i] == TypeBool)
			temp[nrreg ++] = *(char*)param[i];
		else if (ptype[i] == TypeInt64 or ptype[i]->is_some_pointer())
			temp[nrreg ++] = *(int64*)param[i];
		else if (ptype[i] == TypeFloat32)
			temp[N + nsreg ++] = *(int*)param[i]; // float
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
		"ldr s0, [x20]\n" // -> s0
		"add x20, x20, 0x08\n"
		"ldr s1, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr s2, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr s3, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr s4, [x20]\n"
		"add x20, x20, 0x08\n"
		"ldr s5, [x20]\n"
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
		"str s0, [x20]\n" // -> s0:out
		"add x20, x20, 0x08\n"
		"str s1, [x20]\n"
		"add x20, x20, 0x08\n"
		"str s2, [x20]\n"
		"add x20, x20, 0x08\n"
		"str s3, [x20]\n"
		"add x20, x20, 0x08\n"
		"str s4, [x20]\n"
		"add x20, x20, 0x08\n"
		"str s5, [x20]\n"
		 :
		 : "r"(p)
		 : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "s0", "s1", "s2", "s3", "s4", "s5");

	/*msg_write("ok");
	msg_write(temp[N*2+2]);
	msg_write(f2s(*(float*)&temp[N*2+3], 3));
	exit(1);*/
	//msg_write(bytes(&temp, sizeof(temp)).hex());

	if (return_type == TypeInt32 or return_type->is_enum()) {
		*(int*)ret = (int)temp[N*2+2];
	} else if (return_type == TypeInt64 or return_type->is_some_pointer()) {
		*(int64*)ret = temp[N*2+2];
	} else if (return_type == TypeInt8 or return_type == TypeBool) {
		*(char*)ret = (char)temp[N*2+2];
	} else if (return_type->_return_in_float_registers()) {
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

	if (ptype.num == 0) {
		if (return_type == TypeVoid) {
			call0_void(ff, ret, param);
			return true;
		} else if (return_type == TypeInt32) {
			call0<int>(ff, ret, param);
			return true;
		} else if (return_type == TypeFloat32) {
			call0<float>(ff, ret, param);
			return true;
		} else if (return_type->uses_return_by_memory()) {
			call0<CBR>(ff, ret, param);
			return true;
		}
	} else if (ptype.num == 1) {
		if (return_type == TypeVoid) {
			if (ptype[0] == TypeInt32) {
				call1_void_x<int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
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
		} else if (return_type == TypeInt32) {
			if (ptype[0] == TypeInt32 or ptype[0]->is_enum()) {
				call1<int,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeInt8) {
				call1<int,char>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<int,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<int,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeInt64) {
			if (ptype[0] == TypeInt32) {
				call1<int64,int>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeBool or return_type == TypeInt8) {
			if (ptype[0] == TypeInt32) {
				call1<char,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<char,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<char,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeFloat32) {
			if (ptype[0] == TypeInt32) {
				call1<float,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<float,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<float,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeVec3) {
			if (ptype[0]->uses_call_by_reference()) {
				call1<vec3,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeQuaternion) {
			if (ptype[0]->uses_call_by_reference()) {
				call1<vec4,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type->uses_return_by_memory()) {
			if (ptype[0] == TypeInt32) {
				call1<CBR,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
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
		if (return_type == TypeInt32) {
			if ((ptype[0] == TypeInt32) and (ptype[1] == TypeInt32)) {
				call2<int,int,int>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeInt32)) {
				call2<int,CBR,int>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeFloat32)) {
				call2<int,CBR,float>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1]->uses_call_by_reference())) {
				call2<int,CBR,CBR>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeFloat32) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32)) {
				call2<float,float,float>(ff, ret, param);
				return true;
			}
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeFloat32)) {
				call2<float,CBR,float>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeInt64) {
			if ((ptype[0] == TypeInt64) and (ptype[1] == TypeInt64)) {
				call2<int64,int64,int64>(ff, ret, param);
				return true;
			} else if ((ptype[0] == TypeInt64) and (ptype[1] == TypeInt32)) {
				call2<int64,int64,int>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeComplex) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32)) {
				call2<vec2,float,float>(ff, ret, param);
				return true;
			}
		} else if (return_type == TypeQuaternion) {
			if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeFloat32)) {
				call2<vec4,CBR,float>(ff, ret, param);
				return true;
			}
		} else if (return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeInt32) and (ptype[1] == TypeInt32)) {
				call2<CBR,int,int>(ff, ret, param);
				return true;
			} else if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32)) {
				call2<CBR,float,float>(ff, ret, param);
				return true;
			} else if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeInt32)) {
				call2<CBR,CBR,int>(ff, ret, param);
				return true;
			} else if ((ptype[0]->uses_call_by_reference()) and (ptype[1] == TypeFloat32)) {
				call2<CBR,CBR,float>(ff, ret, param);
				return true;
			} else if ((ptype[0]->uses_call_by_reference()) and (ptype[1]->uses_call_by_reference())) {
				call2<CBR,CBR,CBR>(ff, ret, param);
				return true;
			}
		}
	} else if (ptype.num == 3) {
		if (return_type == TypeVec3) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32)) {
				call3<vec3,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32)) {
				((void(*)(void*, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2]);
				return true;
			}
		}*/
	} else if (ptype.num == 4) {
		if (return_type == TypeVoid) {
			if ((ptype[0] == TypeVec3) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32) and (ptype[3] == TypeFloat32)) {
				((void(*)(void*, float, float, float))ff)(param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}
		if (return_type->_return_in_float_registers() and (return_type->size == 16)) { // rect, color, plane, quaternion
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32) and (ptype[3] == TypeFloat32)) {
				call4<vec4,float,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32) and (ptype[3] == TypeFloat32)) {
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
	return object_get_virtual_func_pointer(c, TypeCallableBase->get_call());
}

bool call_callable(void *c, void *ret, const Array<void*> &_param, const Class *return_type, const Array<const Class*> &_ptype) {
	auto ptype = _ptype;
	ptype.insert(TypeCallableBase, 0);
	auto param = _param;
	param.insert(c, 0);
	return call_function_pointer(callable_get_func_pointer(c), ret, param, return_type, ptype);
}



}


