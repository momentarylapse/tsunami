/*
 * Clipboard.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CLIPBOARD_H_
#define SRC_LIB_HUI_CLIPBOARD_H_

#include "../base/base.h"
#include "../base/future.h"

namespace hui {

namespace clipboard {

void copy(const string &buffer);
base::future<string> paste();

}

}


#endif /* SRC_LIB_HUI_CLIPBOARD_H_ */
