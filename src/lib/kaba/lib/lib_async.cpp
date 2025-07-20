#include "../kaba.h"
#include "future.h"

namespace kaba {

const Class *TypeFutureT;
const Class *TypePromiseT;
const Class *TypeFutureCoreT;

const Class* TypeVoidFuture;
const Class* TypeVoidPromise;
const Class* TypeStringFuture;
const Class* TypeStringPromise;

void SIAddPackageAsync(Context *c) {
	add_internal_package(c, "async");

	TypeFutureCoreT = add_class_template("@FutureCore", {"T"}, new TemplateClassInstantiatorFutureCore);
	TypeFutureT = add_class_template("future", {"T"}, new TemplateClassInstantiatorFuture);
	TypePromiseT = add_class_template("promise", {"T"}, new TemplateClassInstantiatorPromise);


	// some pre-defined futures  (more in hui)
	TypeVoidFuture = add_type("future[void]", sizeof(base::future<void>));
	TypeVoidPromise = add_type("promise[void]", sizeof(base::promise<void>));
	TypeStringFuture = add_type("future[string]", sizeof(base::future<string>));
	TypeStringPromise = add_type("promise[string]", sizeof(base::promise<string>));

}

}
