/*
 * ConfigurationDialog.h
 *
 *  Created on: 6 Aug 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_
#define SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_

#include "../../lib/hui/Callback.h"

namespace hui {
	class Window;
}
class Module;

void configure_module(hui::Window *win, Module *m, hui::Callback cb, hui::Callback cb_cancel = nullptr);
void configure_module_autodel(hui::Window *win, Module *m, hui::Callback cb = nullptr, hui::Callback cb_cancel = nullptr);



#endif /* SRC_VIEW_MODULE_CONFIGURATIONDIALOG_H_ */
