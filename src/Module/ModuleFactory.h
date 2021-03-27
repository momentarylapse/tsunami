/*
 * ModuleFactory.h
 *
 *  Created on: 08.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_MODULEFACTORY_H_
#define SRC_MODULE_MODULEFACTORY_H_

#include "../lib/base/base.h"

class Module;
class Session;
enum class ModuleCategory;

class ModuleFactory
{
public:
	static Module *create(Session *session, ModuleCategory type, const string &sub_type);
	static Module *_create_dummy(ModuleCategory type);
	static Module *_create_special(Session *session, ModuleCategory type, const string &sub_type);
	static string base_class(ModuleCategory);
};

#endif /* SRC_MODULE_MODULEFACTORY_H_ */
