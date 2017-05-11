/*
 * hui_error.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUI_ERROR_H_
#define HUI_ERROR_H_



// error handling
void HuiSetErrorFunction(const HuiCallback &function);
void HuiSetDefaultErrorHandler(const HuiCallback &error_cleanup_function);
void HuiRaiseError(const string &message);
void HuiSendBugReport(HuiWindow *parent);


#endif /* HUI_ERROR_H_ */
