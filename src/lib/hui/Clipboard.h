/*
 * Clipboard.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CLIPBOARD_H_
#define SRC_LIB_HUI_CLIPBOARD_H_

namespace hui
{

namespace Clipboard
{

void _cdecl Copy(const string &buffer);
string _cdecl Paste();

}

}


#endif /* SRC_LIB_HUI_CLIPBOARD_H_ */
