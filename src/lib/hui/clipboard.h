/*
 * clipboard.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CLIPBOARD_H_
#define SRC_LIB_HUI_CLIPBOARD_H_

namespace hui
{

// clipboard
void _cdecl CopyToClipBoard(const string &buffer);
string _cdecl PasteFromClipBoard();

}


#endif /* SRC_LIB_HUI_CLIPBOARD_H_ */
