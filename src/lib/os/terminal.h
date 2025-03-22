/*
 * terminal.h
 *
 *  Created on: 28.09.2022
 *      Author: michi
 */

#ifndef SRC_LIB_OS_TERMINAL_H_
#define SRC_LIB_OS_TERMINAL_H_

#include "../base/base.h"

namespace os::terminal {

extern const string RED;
extern const string GREEN;
extern const string YELLOW;
extern const string BLUE;
extern const string MAGENTA;
extern const string CYAN;
extern const string GRAY;
extern const string DARK_GRAY;
extern const string ORANGE;
extern const string BOLD;
extern const string END;

void print(const string &s);

extern string _print_postfix_;

string shell_execute(const string &cmd, bool verbose = false);

}

#endif /* HUITIMER_H_ */
