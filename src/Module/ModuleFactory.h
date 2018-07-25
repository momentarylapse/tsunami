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
enum class ModuleType;

class ModuleFactory
{
public:
	static Module *create(Session *session, ModuleType type, const string &sub_type);
	static Module *_create_dummy(ModuleType type);
	static Module *_create_special(Session *session, ModuleType type, const string &sub_type);
	static string base_class(ModuleType);
};

#endif /* SRC_MODULE_MODULEFACTORY_H_ */
