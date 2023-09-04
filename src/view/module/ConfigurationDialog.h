/*
 * ConfigurationDialog.h
 *
 *  Created on: 6 Aug 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_
#define SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_

#include "../../lib/base/pointer.h"
#include "../../lib/base/future.h"

namespace hui {
	class Window;
}
class Module;

base::future<void> configure_module(hui::Window *win, shared<Module> m);



#endif /* SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_ */
