/*
 * ModuleFactory.h
 *
 *  Created on: 08.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_MODULEFACTORY_H_
#define SRC_MODULE_MODULEFACTORY_H_

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"

class Module;
class Session;
enum class ModuleCategory;

namespace kaba {
	class Class;
}

class ModuleFactory {
public:
	static xfer<Module> create(Session *session, ModuleCategory type, const string &sub_type);
	static xfer<Module> create_by_class(Session *session, const kaba::Class* type);
	static xfer<Module> _create_dummy(ModuleCategory type);
	static xfer<Module> _create_special(Session *session, ModuleCategory type, const string &sub_type);
	static string base_class(ModuleCategory);
};

#endif /* SRC_MODULE_MODULEFACTORY_H_ */
