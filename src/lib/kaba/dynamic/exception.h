/*
 * exception.h
 *
 *  Created on: Jan 27, 2018
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_EXCEPTION_H_
#define SRC_LIB_KABA_LIB_EXCEPTION_H_

#include "../../base/base.h"

namespace base {
	struct Error;
}

namespace kaba {

class Module;


class KabaException {
public:
	string text;
	KabaException(){}
	explicit KabaException(const string &message);
	virtual ~KabaException() = default;
	void _cdecl __init__(const string &message);
	virtual _cdecl void __delete__();
	virtual _cdecl string message();
};

class KabaNoValueError : public KabaException {
public:
	KabaNoValueError();
	void _cdecl __init__();
};

class KabaNullPointerError : public KabaException {
public:
	KabaNullPointerError();
};

enum class ErrorID {
	NONE,
	OPTIONAL_NO_VALUE,
	NULL_POINTER
};

void kaba_raise_exception(KabaException* kaba_exception);
KabaException* create_kaba_exception(const string& message);
void kaba_die_exception(KabaException* e);
void kaba_die_error(base::Error* e);
void kaba_die_msg(const string& msg);
void kaba_die_id(ErrorID id);
void kaba_assert(bool b);



#define KABA_EXCEPTION_WRAPPER2(CODE,EXCLASS) \
try{ \
	CODE; \
}catch(::Exception &e){ \
	kaba::kaba_raise_exception(new EXCLASS(e.message())); \
}

#define KABA_EXCEPTION_WRAPPER_RESULT_VOID(CODE) \
try{ \
	CODE; \
	return base::result_success(); \
}catch(::Exception &e){ \
	return base::Error(e.message()); \
}


}


#endif /* SRC_LIB_KABA_LIB_EXCEPTION_H_ */
