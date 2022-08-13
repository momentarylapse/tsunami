/*
 * ConfigurationDialog.h
 *
 *  Created on: 6 Aug 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_
#define SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_

#include "../../lib/hui/Callback.h"
#include "../../lib/base/pointer.h"

namespace hui {
	class Window;
}
class Module;

void configure_module(hui::Window *win, shared<Module> m, hui::Callback cb = nullptr, hui::Callback cb_cancel = nullptr);



#endif /* SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_ */