/*
 * ErrorHandler.h
 *
 *  Created on: 19.08.2020
 *      Author: michi
 */

#pragma once

namespace tsunami {

class ErrorHandler {
public:
	static void init();
	static void show_backtrace();
	static void error_handler();
};

}
