#include "../../os/date.h"
#include "../../os/time.h"
#include "../kaba.h"
#include "lib.h"


namespace kaba {

void SIAddPackageTime(Context *c) {
	add_internal_package(c, "time", "1");

	common_types.date = add_type("Date", sizeof(Date));
	common_types.timer = add_type("Timer", sizeof(os::Timer));


	add_class(common_types.date);
		class_add_element("time", common_types.i64, &Date::time);
		class_add_func("format", common_types.string, &Date::format, Flags::Pure);
			func_add_param("f", common_types.string);
		class_add_func(Identifier::func::Str, common_types.string, &Date::str, Flags::Pure);
		class_add_func(Identifier::func::Assign, common_types._void, &generic_assign<Date>, Flags::Mutable);
			func_add_param("o", common_types.date);


	add_class(common_types.timer);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init<os::Timer>, Flags::Mutable);
		class_add_func("get", common_types.f32, &os::Timer::get, Flags::Mutable);
		class_add_func("reset", common_types._void, &os::Timer::reset, Flags::Mutable);
		class_add_func("peek", common_types.f32, &os::Timer::peek);


	add_func("sleep", common_types._void, &os::sleep, Flags::Static);
		func_add_param("duration", common_types.f32);
	add_func("now", common_types.date, &Date::now, Flags::Static);
}

};
