#include "../kaba.h"
#include "future.h"

namespace kaba {

void SIAddPackageAsync(Context *c) {
	add_internal_package(c, "async", "1");

	common_types.future_core_t = add_class_template("@FutureCore", {"T"}, new TemplateClassInstantiatorFutureCore);
	common_types.future_t = add_class_template("future", {"T"}, new TemplateClassInstantiatorFuture);
	common_types.promise_t = add_class_template("promise", {"T"}, new TemplateClassInstantiatorPromise);


	common_types.void_future = add_type("future[void]", sizeof(base::future<void>));
	common_types.void_promise = add_type("promise[void]", sizeof(base::promise<void>));
	common_types.string_future = add_type("future[string]", sizeof(base::future<string>));
	common_types.string_promise = add_type("promise[string]", sizeof(base::promise<string>));

	common_types.path_future = add_type("future[Path]", sizeof(base::future<Path>));
	common_types.bool_future = add_type("future[bool]", sizeof(base::future<bool>));

	common_types.callback = add_type_func(common_types._void, {});
	auto TypeCallbackPath = add_type_func(common_types._void, {common_types.path});
	common_types.callback_string = add_type_func(common_types._void, {common_types.string});
	auto TypeCallbackBool = add_type_func(common_types._void, {common_types._bool});



	lib_create_future<string>(common_types.string_future, common_types.string, common_types.callback_string);
	lib_create_future<Path>(common_types.path_future, common_types.path, TypeCallbackPath);
	lib_create_future<bool>(common_types.bool_future, common_types._bool, TypeCallbackBool);
	lib_create_future<void>(common_types.void_future, common_types._void, common_types.callback);

	lib_create_promise<void>(common_types.void_promise, common_types._void, common_types.void_future);
	lib_create_promise<string>(common_types.string_promise, common_types.string, common_types.string_future);
}

}
