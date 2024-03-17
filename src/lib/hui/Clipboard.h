/*
 * Clipboard.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CLIPBOARD_H_
#define SRC_LIB_HUI_CLIPBOARD_H_

namespace hui {

namespace clipboard {

void copy(const string &buffer);
base::future<string> paste();

}

}


#endif /* SRC_LIB_HUI_CLIPBOARD_H_ */
