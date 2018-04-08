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

class ModuleFactory
{
public:
	static Module *create(Session *session, int type, const string &sub_type);
	static Module *_create_dummy(int type);
	static Module *_create_special(Session *session, int type, const string &sub_type);
	static string base_class(int type);
};

#endif /* SRC_MODULE_MODULEFACTORY_H_ */
