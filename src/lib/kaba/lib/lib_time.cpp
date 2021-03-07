#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#elif defined(_X_USE_HUI_MINIMAL_)
	#include "../../hui_minimal/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif


namespace kaba {


extern const Class *TypeDate;
const Class *TypeTimer;

void SIAddPackageTime() {
	add_package("time");

	TypeDate = add_type("Date", sizeof(Date));
	TypeTimer = add_type("Timer", sizeof(hui::Timer));


	add_class(TypeDate);
		class_add_elementx("time", TypeInt64, &Date::time);
		class_add_funcx("format", TypeString, &Date::format, Flags::PURE);
			func_add_param("f", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_STR, TypeString, &Date::str, Flags::PURE);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &Date::__assign__);
			func_add_param("o", TypeDate);
		class_add_funcx("now", TypeDate, &Date::now, Flags::STATIC);


	add_class(TypeTimer);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &hui::Timer::reset);
		class_add_funcx("get", TypeFloat32, &hui::Timer::get);
		class_add_funcx("reset", TypeVoid, &hui::Timer::reset);
		class_add_funcx("peek", TypeFloat32, &hui::Timer::peek);



	add_funcx("sleep", TypeVoid, &hui::Sleep, Flags::STATIC);
		func_add_param("duration", TypeFloat32);
}

};
