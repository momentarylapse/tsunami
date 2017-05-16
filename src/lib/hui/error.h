/*
 * hui_error.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUI_ERROR_H_
#define HUI_ERROR_H_

#include "Callback.h"

namespace hui
{

class Window;

// error handling
void SetErrorFunction(const Callback &function);
void SetDefaultErrorHandler(const Callback &error_cleanup_function);
void RaiseError(const string &message);
void SendBugReport(Window *parent);

};

#endif /* HUI_ERROR_H_ */
