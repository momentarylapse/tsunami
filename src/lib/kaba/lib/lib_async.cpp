#include "../kaba.h"
#include "future.h"

namespace kaba {


extern const Class* TypePath;

const Class* TypeCallback;
const Class* TypeCallbackString;

const Class* TypeFutureT;
const Class* TypePromiseT;
const Class* TypeFutureCoreT;

const Class* TypeVoidFuture;
const Class* TypeVoidPromise;
const Class* TypeStringFuture;
const Class* TypeStringPromise;
const Class* TypePathFuture;
const Class* TypeBoolFuture;

void SIAddPackageAsync(Context *c) {
	add_internal_package(c, "async");

	TypeFutureCoreT = add_class_template("@FutureCore", {"T"}, new TemplateClassInstantiatorFutureCore);
	TypeFutureT = add_class_template("future", {"T"}, new TemplateClassInstantiatorFuture);
	TypePromiseT = add_class_template("promise", {"T"}, new TemplateClassInstantiatorPromise);


	TypeVoidFuture = add_type("future[void]", sizeof(base::future<void>));
	TypeVoidPromise = add_type("promise[void]", sizeof(base::promise<void>));
	TypeStringFuture = add_type("future[string]", sizeof(base::future<string>));
	TypeStringPromise = add_type("promise[string]", sizeof(base::promise<string>));

	TypePathFuture = add_type("future[Path]", sizeof(base::future<Path>));
	TypeBoolFuture = add_type("future[bool]", sizeof(base::future<bool>));

	TypeCallback = add_type_func(TypeVoid, {});
	auto TypeCallbackPath = add_type_func(TypeVoid, {TypePath});
	TypeCallbackString = add_type_func(TypeVoid, {TypeString});
	auto TypeCallbackBool = add_type_func(TypeVoid, {TypeBool});



	lib_create_future<string>(TypeStringFuture, TypeString, TypeCallbackString);
	lib_create_future<Path>(TypePathFuture, TypePath, TypeCallbackPath);
	lib_create_future<bool>(TypeBoolFuture, TypeBool, TypeCallbackBool);
	lib_create_future<void>(TypeVoidFuture, TypeVoid, TypeCallback);

	lib_create_promise<void>(TypeVoidPromise, TypeVoid, TypeVoidFuture);
	lib_create_promise<string>(TypeStringPromise, TypeString, TypeStringFuture);
}

}
