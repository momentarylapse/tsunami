#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif


namespace Kaba {


extern const Class *TypeDate;
const Class *TypeTimer;

void SIAddPackageTime() {
	add_package("time", false);

	TypeDate = add_type("Date", sizeof(Date));
	TypeTimer = add_type("Timer", sizeof(hui::Timer));


	add_class(TypeDate);
		class_add_elementx("time", TypeInt, &Date::time);
		class_add_funcx("format", TypeString, &Date::format);
			func_add_param("f", TypeString);
		class_add_funcx("str", TypeString, &Date::str);
		class_add_funcx("now", TypeDate, &get_current_date, FLAG_STATIC);


	add_class(TypeTimer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Timer::reset));
		class_add_func("get", TypeFloat32, mf(&hui::Timer::get));
		class_add_func("reset", TypeVoid, mf(&hui::Timer::reset));
		class_add_func("peek", TypeFloat32, mf(&hui::Timer::peek));



	add_func("sleep", TypeVoid, (void*)&hui::Sleep, FLAG_STATIC);
		func_add_param("duration", TypeFloat32);
}

};
