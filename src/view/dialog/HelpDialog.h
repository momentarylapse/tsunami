/*
 * HelpDialog.h
 *
 *  Created on: 31.05.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_HELPDIALOG_H_
#define SRC_VIEW_DIALOG_HELPDIALOG_H_

#include "../../lib/hui/Window.h"

namespace tsunami {

class HelpDialog: public hui::Dialog {
public:
	HelpDialog(hui::Window *parent);
};

}

#endif /* SRC_VIEW_DIALOG_HELPDIALOG_H_ */
