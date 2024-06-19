/*
 * SelectStringDialog.h
 *
 *  Created on: 3 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_SELECTSTRINGDIALOG_H_
#define SRC_VIEW_DIALOG_SELECTSTRINGDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../lib/base/optional.h"

namespace tsunami {

class SelectStringDialog : public hui::Dialog {
public:
	SelectStringDialog(hui::Window *parent, const Array<int> &strings);

	void on_string();
	void on_cancel();

	base::optional<int> result;
};

}

#endif /* SRC_VIEW_DIALOG_SELECTSTRINGDIALOG_H_ */
