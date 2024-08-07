/*
 * ModuleConfiguration.h
 *
 *  Created on: 06.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MODULECONFIGURATION_H_
#define SRC_MODULE_MODULECONFIGURATION_H_


#include "../lib/base/base.h"

namespace kaba {
	class Script;
	class Class;
};
class Any;

namespace tsunami {

class Session;
class Module;

class ModuleConfiguration : public VirtualBase {
public:
	ModuleConfiguration() { _module = nullptr; kaba_class = nullptr; }
	virtual ~ModuleConfiguration() {}
	void _cdecl __init__();
	void _cdecl __delete__() override;
	virtual void _cdecl reset(){}
	string to_string() const;
	void from_string(const string &s, Session *session);
	void from_string_legacy(const string &s, Session *session);
	Any to_any() const;
	void from_any(const Any &a, Session *session);
	virtual string auto_conf(const string &name) const;
	void changed();

	Module *_module;
	const kaba::Class *kaba_class;

	string safe_module_name() const;
};

string var_to_string(const kaba::Class *c, char *v);
void var_from_string(const kaba::Class *type, char *v, const string &s, int &pos, Session *session);

}

#endif /* SRC_MODULE_MODULECONFIGURATION_H_ */
