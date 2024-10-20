/*
 * date.h
 *
 *  Created on: 19 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_OS_DATE_H_
#define SRC_LIB_OS_DATE_H_

#include "../base/base.h"

class Date {
public:
	int64 time;
	int milli_second;
	int dummy[7];
	string _cdecl format(const string &f) const;
	string _cdecl str() const;
	void __assign__(const Date &d);

	static Date _cdecl now();
	static Date _cdecl from_unix(int64 t);
};


#endif /* SRC_LIB_OS_DATE_H_ */
